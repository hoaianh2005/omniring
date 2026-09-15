#include "gaugeCtrlTasks.h"
/*
*
*
*
*
*/
gauge_pwr_state gauge_pwr_stt=
{
    .on=false
};
void gauge_pwr(bool state)
{
    gauge_pwr_stt.on=!state;
}
/*******************************************************************************************************/
/*                                   GAUGE FSM IMPLEMENTATION START                                    */
/*******************************************************************************************************/
gauge_state_t gauge_fsm[3]=
{
    /* state 0: LOW_BAT */
    {
		.ns={GAUGE_LOWBAT, GAUGE_NORMALBAT, GAUGE_CHARGINGBAT}
	},
	/* state 1: NORMAL_BAT */
	{
		.ns={GAUGE_LOWBAT, GAUGE_NORMALBAT, GAUGE_CHARGINGBAT}
	},
	/* state 2: CHARGING */
	{
		.ns={GAUGE_LOWBAT, GAUGE_NORMALBAT, GAUGE_CHARGINGBAT}
	}
};
gauge_state gauge_cs=GAUGE_NORMALBAT;
uint8_t* gauge_stt2str[3]={"LOW BAT", "NORMAL BAT", "CHARGING"};
static uint8_t bat_stt_cycle_id=0;
static uint8_t charge_cycle_id=0;
static uint8_t charge_stt_val=1;
static uint8_t discharge_stt_val=0;

void pin_vol_to_perc(uint16_t* vol_ret)
{
    if((*vol_ret)<4200 && (*vol_ret)>=4100) (*vol_ret)=100;
    else if((*vol_ret)<4100 && (*vol_ret)>=4000) (*vol_ret)=95;
    else if((*vol_ret)<4000 && (*vol_ret)>=3900) (*vol_ret)=86;
    else if((*vol_ret)<3900 && (*vol_ret)>=3800) (*vol_ret)=78;
    else if((*vol_ret)<3800 && (*vol_ret)>=3700) (*vol_ret)=67;
    else if((*vol_ret)<3700 && (*vol_ret)>=3600) (*vol_ret)=55;
    else if((*vol_ret)<3600 && (*vol_ret)>=3500) (*vol_ret)=30;
    else if((*vol_ret)<3500 && (*vol_ret)>=3400) (*vol_ret)=15;
    else if((*vol_ret)<3400 && (*vol_ret)>=3300) (*vol_ret)=9;
    else if((*vol_ret)<3300 && (*vol_ret)>=3200) (*vol_ret)=8;
    else if((*vol_ret)<3200 && (*vol_ret)>=3100) (*vol_ret)=7;
    else if((*vol_ret)<3100 && (*vol_ret)>=3000) (*vol_ret)=5;
    else if((*vol_ret)<3000 && (*vol_ret)>=2900) (*vol_ret)=4;
    else if((*vol_ret)<2900 && (*vol_ret)>=2800) (*vol_ret)=3;
    else if((*vol_ret)<2800 && (*vol_ret)>=2700) (*vol_ret)=1;
    else (*vol_ret)=0;
}

void gauge_finite_sm(gauge_state_input input)
{
	gauge_cs=gauge_fsm[gauge_cs].ns[input];
    printk("\n----> GAUGE: %s", gauge_stt2str[gauge_cs]);
    switch(gauge_cs)
    {
        case GAUGE_LOWBAT:
        {
            uint8_t soc_ret[2];
            uint8_t temp_ret[2];
            uint16_t bat_stt[2];
            if(gauge_soc_read(&bq27220_dev, soc_ret) && gauge_temp_read(&bq27220_dev, temp_ret)) 
            {
                bat_stt[0]=soc_ret[0]|(soc_ret[1]<<8); pin_vol_to_perc(&bat_stt[0]);
                bat_stt[1]=temp_ret[0]|(temp_ret[1]<<8); bat_stt[1]=bat_stt[1]/10;
                ble_data_framing(BAT_STATE_EVT, bat_stt_cycle_id, 0, bat_stt); 
                bat_stt_cycle_id=(bat_stt_cycle_id+1)%128;
                printk("\n    -- pin's perc now = %d & pin's temp now(K) = %d", bat_stt[0], bat_stt[1]);
            }
            printk("\n    - gauge_lowbat entered!");
            break;
        }
        case GAUGE_NORMALBAT:
        {
            if(!gauge_pwr_stt.on)
            {
                printk("\n    - gauge power state now: %d!(1:ON, 0:OFF)", gauge_pwr_stt.on);
                gauge_pwr(ON);
                uint8_t ret[2];
                if(gauge_soc_read(&bq27220_dev, ret)) gauge_irq_config(&bq27220_dev);
            }
            printk("\n    - gauge power state now: %d!(1:ON, 0:OFF)", gauge_pwr_stt.on);
            printk("\n    - gauge_normalbat entered!");
            break;
        }
        case GAUGE_CHARGINGBAT:
        {     
            ble_data_framing(CHARGE_EVT, charge_cycle_id, 0, &charge_stt_val); 
            charge_cycle_id=(charge_cycle_id+1)%128;
            printk("\n    - gauge_chargingbat entered!"); 
            break;
        }	
        default: break;
    }
}
/*
*/
/*******************************************************************************************************/
/*                                     GAUGE READ DATA/SIGS START                                      */
/*******************************************************************************************************/
K_SEM_DEFINE(pin_status_update_sem,0,1);
void gauge_low_bat_cb_handler(const struct device *dev, struct gpio_callback *cb, uint32_t pins)
{
    printk("\nGAUGE_LOW_BAT_ISR: low bat detected!");
    evt_q_t evt_q_put;
    evt_q_put.evt_src=LOW_BAT_EVT;
    k_msgq_put(&sys_evt_msgq, &evt_q_put, K_NO_WAIT);
}
static void delayed_work_cb_handler(struct k_work *work)
{
    int ret=gpio_pin_get(gpio1, CHARGE_INDICATOR_LED);
    if(ret) // discharging -> need to re-check low bat status
    {
        printk("\nGAUGE_CHARGING_ISR: discharging...logic level now = %d", ret);
        k_sem_give(&pin_status_update_sem);
    }
    else   // charging
    {
        printk("\nGAUGE_CHARGING_ISR: charging...logic level now = %d", ret);
        evt_q_t evt_q_put;
        evt_q_put.evt_src=CHARGING_EVT;
        k_msgq_put(&sys_evt_msgq, &evt_q_put, K_NO_WAIT);
    }
}
K_WORK_DELAYABLE_DEFINE(charge_dbc_work,delayed_work_cb_handler);
void gauge_charging_cb_handler(const struct device *dev, struct gpio_callback *cb, uint32_t pins)
{
    printk("\nGAUGE_CHARGING_ISR: reschedule delayed work!");
    k_work_reschedule(&charge_dbc_work, K_MSEC(1000));
}
void gauge_stt_check_thread()
{
    bq27220_status_ret stt_ret;
    evt_q_t evt_q_put;
    uint8_t discharge_cycle_id=0;
    while(1)
    {
        k_sem_take(&pin_status_update_sem, K_FOREVER);
        ble_data_framing(CHARGE_EVT, discharge_cycle_id, 0,  &discharge_stt_val);
        discharge_cycle_id=(discharge_cycle_id+1)%128;
        int ret=gauge_status_check(&bq27220_dev, &stt_ret);
        if(ret)
        {
            if(stt_ret==BQ_NORMAL_BAT)
            {
                printk("\nGAUGE_STT_CHECK_THREAD: after charge, pin status now is NORMAL_BAT");
                evt_q_put.evt_src=NORMAL_BAT_EVT;
                k_msgq_put(&sys_evt_msgq, &evt_q_put, K_NO_WAIT);
            }
            else if(stt_ret==BQ_LOW_BAT)
            {
                printk("\nGAUGE_STT_CHECK_THREAD: after charge, pin status now is LOW_BAT");
                evt_q_put.evt_src=LOW_BAT_EVT;
                k_msgq_put(&sys_evt_msgq, &evt_q_put, K_NO_WAIT);
            }
        }
    }
}
K_THREAD_DEFINE(gauge_stt_check_thread_tid, 1024, gauge_stt_check_thread, NULL, NULL, NULL, 7, 0, 0);

K_SEM_DEFINE(pin_soc_read_sem,0,1);
void gauge_soc_read_thread()
{
    uint8_t vol_ret[2];
    uint8_t temp_ret[2];
    uint16_t bat_stt[2];
    while(1)
    {
        k_sem_take(&pin_soc_read_sem, K_FOREVER);
        if(gauge_soc_read(&bq27220_dev, vol_ret) && gauge_temp_read(&bq27220_dev, temp_ret)) 
        {
            bat_stt[0]=vol_ret[0]|(vol_ret[1]<<8);
            pin_vol_to_perc(&bat_stt[0]);
            bat_stt[1]=temp_ret[0]|(temp_ret[1]<<8); bat_stt[1]=bat_stt[1]/10;
            ble_data_framing(BAT_STATE_EVT, bat_stt_cycle_id, 0, bat_stt);
            printk("\nGAUGE_SOC_READ_THREAD: pin's perc now = %d & pin's temp now(K) = %d", bat_stt[0], bat_stt[1]);
        }
    }
}
K_THREAD_DEFINE(gauge_soc_read_thread_tid, 1024, gauge_soc_read_thread, NULL, NULL, NULL, 7, 0, 0);
/*******************************************************************************************************/
/*                                     GAUGE READ DATA/SIGS END                                        */
/*******************************************************************************************************/
const struct gauge_api bq27220_api=
{
    .soc_read=bq_soc_read,
    .temp_read=bq_temp_read,
    .status_check=bq_status_check,
    .irq_config=bq_irq_config
};
const struct device_t bq27220_dev=
{
    .name="ti_bq27220",
    .data=&bq27220,
    .api=&bq27220_api
};