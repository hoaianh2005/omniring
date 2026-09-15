#include "ppgCtrlTasks.h"
/*
*
*
*
*
*/
ppg_pwr_state ppg_pwr_stt=
{
    .on=false
};
void ppg_pwr(bool state)
{
    gpio_pin_set(gpio0, PPG_PWR_PIN, state);
    ppg_pwr_stt.on=!state;
    if(!state) k_msleep(10);
}
/*******************************************************************************************************/
/*                                     PPG FSM IMPLEMENTATION START                                    */
/*******************************************************************************************************/
ppg_state_t ppg_fsm[2]=
{
    /* state 0: PWROFF */
    {
		.ns={PPG_PWROFF, PPG_MEASURE}
	},
	/* state 1: MEASURE */
	{
		.ns={PPG_PWROFF, PPG_MEASURE}
	},
};
ppg_state ppg_cs=PPG_PWROFF;
uint8_t* ppg_stt2str[2]={"POWER OFF", "MEASURE"};
void ppg_finite_sm(ppg_state_input input)
{
	ppg_cs=ppg_fsm[ppg_cs].ns[input];
    printk("\n----> PPG: %s", ppg_stt2str[ppg_cs]);
    switch(ppg_cs)
    {
        case PPG_PWROFF:
        {
            if(ppg_pwr_stt.on) 
            {
                printk("\n    - ppg power state now: %d!(1:ON, 0:OFF)", ppg_pwr_stt.on);
                ppg_pwr(OFF);
            }
            printk("\n    - ppg power state now: %d!(1:ON, 0:OFF)", ppg_pwr_stt.on);
            printk("\n    - ppg_pwroff entered!");
            break;
        }
        case PPG_MEASURE:
        {
            if(!ppg_pwr_stt.on) 
            {
                printk("\n    - ppg power state now: %d!(1:ON, 0:OFF)", ppg_pwr_stt.on);
                ppg_pwr(ON);
                // if(ppg_init(&as7057_dev))
                // {
                //     if(ppg_config(&as7057_dev)) k_sem_give(&ppg_rx_start_sem);
                // }
                ppg_init(&as7057_dev);
                ppg_config(&as7057_dev);
                k_sem_give(&ppg_rx_start_sem);
            }
            printk("\n    - ppg power state now: %d!(1:ON, 0:OFF)", ppg_pwr_stt.on);
            printk("\n    - ppg_measuring entered!");
            break;
        }
        default: break;
    }
}
/*******************************************************************************************************/
/*                                     PPG FSM IMPLEMENTATION END                                      */
/*******************************************************************************************************/
/*
*/
/*******************************************************************************************************/
/*                                      PPG READ & PROC DATA START                                     */
/*******************************************************************************************************/
int32_t ppg_pingpong[2][PPG_PINGPONG_SIZE];
int32_t* ppg_pingpong_wr_ptr=ppg_pingpong[0];
int32_t* ppg_pingpong_rd_ptr=ppg_pingpong[1];
uint32_t ppg_pingpong_wr_idx=0;
K_SEM_DEFINE(ppg_rx_start_sem,0,1);
K_SEM_DEFINE(ppg_rx_stop_sem,0,1);
void ppg_ctrl_thread()
{
    uint32_t ppg_raw_len=0;
    static uint8_t ppg_raw_buf[PPG_RAW_BUF_SIZE]; 
    uint8_t asat_stt_ret;

    uint8_t ppg_result_len=0;
    static int32_t ppg_data_to_save[PPG_RESULT_BUF_SIZE];

    while(1)
    {
        k_sem_take(&ppg_rx_start_sem, K_FOREVER);
        k_msleep(500);
        if(ppg_fifo_burst_read(&as7057_dev, ppg_raw_buf, &ppg_raw_len))
        {
            if(ppg_reg_read(&as7057_dev, STATUS_ASAT, &asat_stt_ret)) printk("\nPPG_REV_DATA_THREAD: asat reg read = %d\n", asat_stt_ret);
            printk("\nPPG_REV_DATA_THREAD: ready process data!");

            ppg_data_proc(&as7057_dev, ppg_raw_buf, ppg_raw_len, ppg_data_to_save, &ppg_result_len);
            printk("\nPPG_REV_DATA_THREAD: data is processed!");
            
            printk("\nPPG_REV_DATA_THREAD: ppg results log start-> ");
            for(int i=0; i<ppg_result_len; i++)
            {
                printk("%d ", ppg_data_to_save[i]);
                ppg_pingpong_wr_ptr[ppg_pingpong_wr_idx++]=ppg_data_to_save[i];
            }
            printk(" <-ppg results log end");

            printk("\nPPG_REV_DATA_THREAD: ppg_data_wr_idx now = %d", ppg_pingpong_wr_idx);
            if(ppg_pingpong_wr_idx>=1500) 
            {
                printk("\nPPG_REV_DATA_THREAD: ppg buffer ready(1500 samples)!");
                ppg_pingpong_wr_idx=0;
                if(ppg_pingpong_wr_ptr==ppg_pingpong[0]) 
                {
                    printk("\nPPG_REV_DATA_THREAD: switch to pingpong[1] write pointer!");
                    ppg_pingpong_wr_ptr=ppg_pingpong[1];
                    ppg_pingpong_rd_ptr=ppg_pingpong[0];
                }
                else if(ppg_pingpong_wr_ptr==ppg_pingpong[1]) 
                {
                    printk("\nPPG_REV_DATA_THREAD: switch to pingpong[0] write pointer!");
                    ppg_pingpong_wr_ptr=ppg_pingpong[0];
                    ppg_pingpong_rd_ptr=ppg_pingpong[1];
                }
                if(ble_connected)
                {
                    printk("\nPPG_REV_DATA_THREAD: ble connected, post event to ready send data!");
                    k_event_post(&data_is_ready_evt, 0x2);
                }
                else printk("\nPPG_REV_DATA_THREAD: ble disconnected, no post event!");
            }
        }
        else printk("\nPPG_REV_DATA_THREAD: fifo read failed!");
        int thread_stop_ret=k_sem_take(&ppg_rx_stop_sem, K_NO_WAIT);
        if(!thread_stop_ret)
        {
            printk("\nPPG_REV_DATA_THREAD: ready stop thread!");
            k_event_post(&sensors_rx_stopped_evt, 0x2);
        }
        else k_sem_give(&ppg_rx_start_sem);
    }
}
K_THREAD_DEFINE(ppg_ctrl_thread_tid, 1024, ppg_ctrl_thread, NULL, NULL, NULL, 6, 0, 0);
/*******************************************************************************************************/
/*                                      PPG READ & PROC DATA END                                       */
/*******************************************************************************************************/
/*
*/
const struct ppg_api_t as7057_api=
{
    .init=as7057_init,
    .config=as7057_config,
    .reg_read=as7057_reg_read,
    .fifo_burst_read=as7057_fifo_burst_read,
    .data_proc=as7057_data_proc
};
const struct device_t as7057_dev=
{
    .name="ams_osram_as7057",
    .data=&as7057,
    .api=&as7057_api
};