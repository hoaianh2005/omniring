#include "lteCtrlTasks.h"
/*
*
*
*
*
*/
lte_pwr_state lte_pwr_stt=
{
    .on=true
};
void lte_pwr(bool state)
{
    if(!lte_pwr_stt.on) 
    {
        lte_turn_on(&lexi_dev);
        printk("\n    -- lte turned on by pull low pwrkey_on pin!");
    }
    lte_pwr_stt.on=!state;
    if(!state)
    {
        uart_rx_enable(uart1, lexi_rsp_buf[0], LTE_RSP_BUF_SIZE, 2000);
        printk("\n    -- uart1 rx buffer enabled!");
    }
    else 
    {
        lte_turn_off(&lexi_dev);
        printk("\n    -- lte turned off by cpwroff command!");
    }
}
/*******************************************************************************************************/
/*                                     LTE FSM IMPLEMENTATION START                                    */
/*******************************************************************************************************/
lte_state_t lte_fsm[3]=
{
    /* state 0: PWROFF */
    {
		.ns={LTE_PWROFF, LTE_SLEEP, LTE_ACTIVE}
	},
	/* state 1: SLEEP */
	{
		.ns={LTE_PWROFF, LTE_SLEEP, LTE_ACTIVE}
	},
	/* state 2: ACTIVE */
	{
		.ns={LTE_PWROFF, LTE_SLEEP, LTE_ACTIVE}
	},
};
lte_state lte_cs=LTE_PWROFF;
uint8_t* lte_stt2str[3]={"POWER OFF", "SLEEP", "ACTIVE"};
void lte_finite_sm(lte_state_input input)
{
	lte_cs=lte_fsm[lte_cs].ns[input];
    printk("\n----> LTE: %s", lte_stt2str[lte_cs]);
    switch(lte_cs)
    {
        case LTE_PWROFF:
        {
            if(lte_pwr_stt.on)
            {
                printk("\n    - lte power state now: %d!(1:ON, 0:OFF)", lte_pwr_stt.on);
                lte_mqtt_server_disc(&lexi_dev);
                lte_pwr(OFF);
                uart1_suspend();    
            }
            printk("\n    - lte power state now: %d!(1:ON, 0:OFF)", lte_pwr_stt.on);
            printk("\n    - lte_pwroff entered!");
            break;
        }
        case LTE_SLEEP:
        {
            if(!lte_pwr_stt.on)
            {
                printk("\n    - lte power state now: %d!(1:ON, 0:OFF)", lte_pwr_stt.on);
                uart1_resume();
                lte_pwr(ON);
                lte_mqtt_server_conn(&lexi_dev);
                lte_sleep(&lexi_dev);
            }
            printk("\n    - lte power state now: %d!(1:ON, 0:OFF)", lte_pwr_stt.on);
            printk("\n    - lte_sleep_mode entered!");
            break;
        }
        case LTE_ACTIVE:
        {
            printk("\n    - lte_active_mode entered!");
            break;
        }
        default: break;
    }
}
/*******************************************************************************************************/
/*                                     LTE FSM IMPLEMENTATION END                                      */
/*******************************************************************************************************/
/*
*/
/*******************************************************************************************************/
/*                                          LTE SEND DATA START                                        */
/*******************************************************************************************************/
K_SEM_DEFINE(lte_tx_start_sem,0,1);
uint32_t current_pos=0;
static uint8_t cycle_id=0;
void uint16_to_hex_fixed(uint16_t num, char* out) 
{
    const char hex_chars[]="0123456789ABCDEF";  
    for(int i = 3; i >= 0; i--) 
    {
        out[i]=hex_chars[num&0xF];
        num>>=4;
    }
    out[4]='\0';
}
void init_lat_lon_str() 
{
    const char* mac_addr="CC7CAEE3DC54"; // lg76g-pb
    // const char* mac_addr="C0B1FA3EEB42";    // sim65m-w
    for(int i=0; i<12; i++) 
    {
        pub_data[i]=mac_addr[i];
    }
    uint16_t header_info=(0x3<<12)|((cycle_id&0x7F)<<5)|(0x00);
    char hex_header[5];
    uint16_to_hex_fixed(header_info, hex_header);
    for(int i=0; i<4; i++) 
    {
        pub_data[12+i]=hex_header[i];
    }
    current_pos=16; 
    pub_data[current_pos]='\0';
    cycle_id=(cycle_id+1)%128;
}
void int32_to_hex_fixed(uint32_t num, char* out) 
{
    const char hex_chars[]="0123456789ABCDEF";  
    for(int i=7; i>=0; i--) 
    {
        out[i]=hex_chars[num&0xF];
        num>>=4;
    }
    out[8]='\0';
}
void append_hex_pair(int32_t x1, int32_t x2) 
{
    char temp[9];
    int32_to_hex_fixed((uint32_t)x1, temp);
    for(int i=0; i<8; i++) 
    {
        if(current_pos<256-1) pub_data[current_pos++]=temp[i];
    }
    int32_to_hex_fixed((uint32_t)x2, temp);
    for(int i=0; i<8; i++) 
    {
        if(current_pos<256-1) pub_data[current_pos++]=temp[i];
    }
    pub_data[current_pos]='\0';
}

void timeout_cb_handler(struct k_timer *timer_id) 
{
    evt_q_t evt_q_put;
    evt_q_put.evt_src=LATLON_BUF_TIMEOUT_EVT;
    k_msgq_put(&sys_evt_msgq, &evt_q_put, K_NO_WAIT);
}
K_TIMER_DEFINE(lte_start_tx_timer, timeout_cb_handler, NULL);
K_EVENT_DEFINE(lte_ctrl_thread_evt);
void lte_ctrl_thread()
{
    static location_data_t location_data_to_send[LOCATION_DATA_PINGPONG_SIZE];
    location_data_t lat_lon;
    uint32_t len_to_read;
    evt_q_t evt_q_put;
    while(1)
    {
        int ret=k_event_wait(&lte_ctrl_thread_evt, 0x3, false, K_FOREVER);
        if(location_data_pingpong_wr_idx>0)
        {
            printk("\nLTE_SEND_DATA_THREAD: ready send lat_lon buffer!");
            printk("\nLTE_SEND_DATA_THREAD: location_data_pingpong_idx now = %d", location_data_pingpong_wr_idx);
            len_to_read=location_data_pingpong_wr_idx;
            location_data_pingpong_wr_idx=0;
            if(location_data_pingpong_wr_ptr==location_data_pingpong[0])
            {
                location_data_pingpong_wr_ptr=location_data_pingpong[1];
                memcpy(location_data_to_send, location_data_pingpong[0], len_to_read*sizeof(location_data_t));
            }
            else if(location_data_pingpong_wr_ptr==location_data_pingpong[1])
            {
                location_data_pingpong_wr_ptr=location_data_pingpong[0];
                memcpy(location_data_to_send, location_data_pingpong[1], len_to_read*sizeof(location_data_t));
            }
            init_lat_lon_str();
            uint32_t rd_ptr=0;
            while(rd_ptr<len_to_read)
            {
                lat_lon=location_data_to_send[rd_ptr++];
                append_hex_pair(lat_lon.lat, lat_lon.lon);
            }
            if(!lte_send_data(&lexi_dev))
            {
                evt_q_put.evt_src=LTE_SEND_FAILED_EVT;
			    k_msgq_put(&sys_evt_msgq, &evt_q_put, K_NO_WAIT); 
            }
            memset(pub_data, 0, sizeof(pub_data));
            current_pos=0;
        }

        if(ret&0x1)
        {
            k_event_clear(&lte_ctrl_thread_evt, 0x1);
            printk("\nLTE_SEND_DATA_THREAD: send done!");
        }
        if(ret&0x2)
        {
            k_event_clear(&lte_ctrl_thread_evt, 0x2);
            k_event_post(&modules_rx_stopped_evt, 0x2);
            printk("\nLTE_SEND_DATA_THREAD: send remaining data in buffer done!");
        }
    }
}
K_THREAD_DEFINE(lte_ctrl_thread_tid, 1024, lte_ctrl_thread, NULL, NULL, NULL, 7, 0, 0);
/*******************************************************************************************************/
/*                                          LTE SEND DATA END                                          */
/*******************************************************************************************************/
const struct lte_api_t lexi_api=
{
    .lte_com_check=lexi_com_check,
    .lte_sim_check=lexi_sim_check,
    .lte_pwron=lexi_pwron,
    .lte_pwroff=lexi_pwroff,
    .lte_hw_reset=lexi_hw_reset,
    .lte_sleep_enter=lexi_sleep_enter,
    .lte_mqtt_server_open=lexi_mqtt_server_open,
    .lte_mqtt_server_conn=lexi_mqtt_server_conn,
    .lte_mqtt_pub=lexi_mqtt_pub,
    .lte_mqtt_server_disc=lexi_mqtt_server_disc
};
const struct device_t lexi_dev=
{
    .name="ublox_lexi_r10",
    .data=&lexi_data,
    .api=&lexi_api
};