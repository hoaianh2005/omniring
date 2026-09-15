#include "imuCtrlTasks.h"

imu_pwr_state imu_pwr_stt=
{
    .on=false
};

void imu_pwr(bool state)
{
    imu_pwr_stt.on = state;
    if (gpio1) {
        int ret = gpio_pin_set(gpio1, IMU_PWR_PIN, state ? 0 : 1);
        printk("\nIMU power: request=%d pin=P1.%d level=%d ret=%d", state, IMU_PWR_PIN, state ? 0 : 1, ret);
    } else {
        printk("\nIMU power: gpio1 is NULL");
    }
    if (!state) {
        imu_sleep(&bmi270_dev);
    }
}

imu_state_t imu_fsm[3]=
{
    { .ns={IMU_SLEEP, IMU_NO_MOVE, IMU_MOVING} },
    { .ns={IMU_SLEEP, IMU_NO_MOVE, IMU_MOVING} },
    { .ns={IMU_SLEEP, IMU_NO_MOVE, IMU_MOVING} }
};

imu_state imu_cs=IMU_SLEEP;
uint8_t* imu_stt2str[3]={"SLEEP", "NO MOVE", "MOVING"};

void imu_finite_sm(imu_state_input input)
{
    imu_cs=imu_fsm[imu_cs].ns[input];
    printk("\n----> IMU: %s (input=%d)", imu_stt2str[imu_cs], input);
    switch(imu_cs)
    {
        case IMU_SLEEP:
        {
            if(imu_pwr_stt.on) 
            {
                printk("\n    - imu power state now: %d!(1:ON, 0:OFF)", imu_pwr_stt.on);
                imu_pwr(OFF);
            }
            printk("\n    - imu power state now: %d!(1:ON, 0:OFF)", imu_pwr_stt.on);
            printk("\n    - imu_pwroff entered!");
            break;
        }
        case IMU_NO_MOVE:
        {
            if(!imu_pwr_stt.on)
            {
                printk("\n    - imu power state now: %d!(1:ON, 0:OFF)", imu_pwr_stt.on);
                printk("\nIMU FSM: start imu_pwr(ON)");
                imu_pwr(ON);
                printk("\nIMU FSM: calling bmi270 init");
                if(!imu_init(&bmi270_dev))
                {
                    printk("\nBMI270 INIT OK\n");
                    printk("\nIMU FSM: calling bmi270 config");
                    if(!imu_config(&bmi270_dev))
                    {
                        printk("\nBMI270 CONFIG OK\n");
                        printk("\nIMU FSM: give imu_rx_start_sem");
                        k_sem_give(&imu_rx_start_sem);
                    }
                    else
                    {
                        printk("\nIMU FSM: bmi270 config FAILED");
                    }
                }
                else
                {
                    printk("\nIMU FSM: bmi270 init FAILED");
                }
            }
            else
            {
                printk("\nIMU FSM: already powered/configured, no re-init");
            }
            printk("\n    - imu power state now: %d!(1:ON, 0:OFF)", imu_pwr_stt.on);
            printk("\n    - imu_no_move entered!");
            break;
        }
        case IMU_MOVING:
        {
            printk("\n    - imu_moving entered!");
            break;
        }    
        default: break;
    }
}

imu_data_t acc_pingpong[2][ACC_PINGPONG_SIZE];
imu_data_t* acc_pingpong_wr_ptr=acc_pingpong[0];
imu_data_t* acc_pingpong_rd_ptr=acc_pingpong[1];
uint32_t acc_pingpong_wr_idx=0;

K_SEM_DEFINE(imu_rx_start_sem,0,1);
K_SEM_DEFINE(imu_rx_stop_sem,0,1);
K_EVENT_DEFINE(data_is_ready_evt);
K_EVENT_DEFINE(sensors_rx_stopped_evt);

void imu_ctrl_thread()
{ 
    uint32_t acc_raw_len=0;
    static uint8_t acc_raw_buf[ACC_RAW_BUF_SIZE];
    uint8_t int_stt_ret=0;

    uint32_t acc_result_len=0;
    static imu_data_t acc_data_to_save[ACC_RESULT_BUF_SIZE];

    evt_q_t evt_q_put;
    while(1)
    {
        k_sem_take(&imu_rx_start_sem, K_FOREVER);
        printk("\nIMU thread: sem taken, start FIFO read");
        k_msleep(1000);
        if(!imu_fifo_burst_read(&bmi270_dev, acc_raw_buf, &acc_raw_len))
        {
            printk("\nIMU FIFO read OK, raw_len=%d", acc_raw_len);
            if(imu_reg_read(&bmi270_dev, BMI270_REG_INTERNAL_STATUS, &int_stt_ret)) printk("\nIMU_REV_DATA_THREAD: int status reg = %d", int_stt_ret);
            if(int_stt_ret)
            {
                evt_q_put.evt_src=WOM_EVT;
                k_msgq_put(&sys_evt_msgq, &evt_q_put, K_NO_WAIT);
            }
            else
            {
                evt_q_put.evt_src=NO_WOM_EVT;
                k_msgq_put(&sys_evt_msgq, &evt_q_put, K_NO_WAIT);
            }

            printk("\nIMU_REV_DATA_THREAD: ready process data!");
            imu_data_proc(&bmi270_dev, acc_raw_buf, acc_raw_len, acc_data_to_save, &acc_result_len);
            printk("\nIMU_REV_DATA_THREAD: data is processed! count=%d", acc_result_len);

            if(acc_result_len > 0)
            {
                printk("\nIMU sample[0]: x=%d y=%d z=%d", acc_data_to_save[0].x, acc_data_to_save[0].y, acc_data_to_save[0].z);
                ble_send_imu_sample(&acc_data_to_save[0]);
            }

            int debug_count = MIN(acc_result_len, 5U);
            for(int i=0; i<debug_count; i++)
            {
                printk("\nIMU sample[%d]: x=%d y=%d z=%d", i, acc_data_to_save[i].x, acc_data_to_save[i].y, acc_data_to_save[i].z);
            }

            printk("\nIMU_REV_DATA_THREAD: acc results log start-> ");
            for(int i=0; i<acc_result_len; i++)
            {
                acc_pingpong_wr_ptr[acc_pingpong_wr_idx++]=acc_data_to_save[i];
            }
            printk(" <-acc results log end");

            printk("\nIMU_REV_DATA_THREAD: acc_data_wr_idx now = %d", acc_pingpong_wr_idx);
            if(acc_pingpong_wr_idx>=1500) 
            {
                printk("\nIMU_REV_DATA_THREAD: acc buffer ready(1500 samples)!");
                acc_pingpong_wr_idx=0;
                if(acc_pingpong_wr_ptr==acc_pingpong[0]) 
                {
                    printk("\nIMU_REV_DATA_THREAD: switch to pingpong[1] write pointer!");
                    acc_pingpong_wr_ptr=acc_pingpong[1];
                    acc_pingpong_rd_ptr=acc_pingpong[0];
                }
                else if(acc_pingpong_wr_ptr==acc_pingpong[1]) 
                {
                    printk("\nIMU_REV_DATA_THREAD: switch to pingpong[0] write pointer!");
                    acc_pingpong_wr_ptr=acc_pingpong[0];
                    acc_pingpong_rd_ptr=acc_pingpong[1];
                }
                if(ble_connected)
                {
                    printk("\nIMU_REV_DATA_THREAD: ble connected, post event to ready send data!");
                    k_event_post(&data_is_ready_evt, 0x1);
                }
                else printk("\nIMU_REV_DATA_THREAD: ble disconnected, no post event!");
            }
        }
        else printk("\nIMU_REV_DATA_THREAD: fifo read failed!");
        int thread_stop_ret=k_sem_take(&imu_rx_stop_sem, K_NO_WAIT);
        if(!thread_stop_ret)
        {
            printk("\nIMU_REV_DATA_THREAD: ready stop thread!");
            k_event_post(&sensors_rx_stopped_evt, 0x1);
        }
        else k_sem_give(&imu_rx_start_sem);
    }
}
K_THREAD_DEFINE(imu_ctrl_thread_tid, 1024, imu_ctrl_thread, NULL, NULL, NULL, 6, 0, 0);

/* Định nghĩa Struct API & Device cho BMI270 */
const struct imu_api_t bmi270_api =
{
    .init = bmi270_init,
    .config = bmi270_config,
    .reg_read = bmi270_reg_read,
    .sleep = bmi270_sleep,
    .fifo_burst_read = bmi270_fifo_burst_read,
    .data_proc = bmi270_data_proc
};

const struct device_t bmi270_dev =
{
    .name = "bosch_bmi270",
    .data = &bmi270, /* Tạm dùng chung i2c_dt_spec hardware nếu đấu nối chung bus */
    .api = &bmi270_api
};