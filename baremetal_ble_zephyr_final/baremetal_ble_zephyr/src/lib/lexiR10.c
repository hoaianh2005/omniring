#include "lexiR10.h"
/*
*
*
*
*
*/
/*******************************************************************************************************/
/*                                         LEXI-R10 DRIVER START                                       */
/*******************************************************************************************************/
char pub_data[256];
char pub_str[512];
uint8_t lexi_rsp_buf[2][LTE_RSP_BUF_SIZE];
uint8_t* lexi_rsp_wr_ptr=lexi_rsp_buf[1];
uint8_t lexi_rev_buf[1024];
uint8_t lexi_rev_buf_idx=0;
uint8_t lexi_buf_to_proc[1024];
bool uart_rx_active_disb=false;
K_MSGQ_DEFINE(lexi_rsp_msgq, sizeof(uint8_t), 1024, 4);
void uart1_cb_handler(const struct device *dev, struct uart_event *evt, void *user_data)
{
    switch(evt->type)
    {
        case UART_RX_RDY:
        {
            for(int i=0; i<evt->data.rx.len; i++)
            {
                uint8_t c=evt->data.rx.buf[evt->data.rx.offset+i];
                if(lexi_rev_buf_idx<sizeof(lexi_rev_buf)-1) lexi_rev_buf[lexi_rev_buf_idx++]=c;
                if(c=='\n')
                {
                    if(lexi_rev_buf_idx>2)
                    {
                        for(int j=0; j<lexi_rev_buf_idx; j++)
                        {
                            k_msgq_put(&lexi_rsp_msgq, &lexi_rev_buf[j], K_NO_WAIT);
                        }
                        char end_str='\0';
                        k_msgq_put(&lexi_rsp_msgq, &end_str, K_NO_WAIT);
                    }
                    lexi_rev_buf_idx=0;
                }
            }
            break;
        }
        case UART_RX_BUF_REQUEST:
        {
            uart_rx_buf_rsp(uart1, lexi_rsp_wr_ptr, LTE_RSP_BUF_SIZE);
            break;
        }
        case UART_RX_BUF_RELEASED:
        {
            if(lexi_rsp_wr_ptr==lexi_rsp_buf[0]) lexi_rsp_wr_ptr=lexi_rsp_buf[1];
            else if(lexi_rsp_wr_ptr==lexi_rsp_buf[1]) lexi_rsp_wr_ptr=lexi_rsp_buf[0];
            break;
        }
        case UART_RX_DISABLED:
        {
            if(!uart_rx_active_disb) uart_rx_enable(uart1, lexi_rsp_buf[0], LTE_RSP_BUF_SIZE, 2000);
            else uart_rx_active_disb=false;
            break;
        }
        case UART_RX_STOPPED: break;
        default: break;
        }
}
uint8_t get_msq(struct k_msgq* msq, uint8_t* buf, uint16_t timeout)
{
    uint8_t tmp;
    uint8_t idx=0;
    uint8_t ret=k_msgq_get(msq, &tmp, K_MSEC(timeout));
    if (ret!=0) return 0;
    buf[idx]=tmp;
    idx++;
    while(tmp!='\0' && idx<1023)
    {
        ret=k_msgq_get(msq, &tmp, K_MSEC(5));
        if(ret!=0) break;
        else buf[idx++]=tmp;
    }
    buf[idx]='\0';
    return idx;
}
uint8_t wait_for_final_response(uint8_t* expect_resp, uint16_t timeout_ms)
{
    uint32_t start_time=k_uptime_get_32();
    while(k_uptime_get_32()-start_time<timeout_ms)
    {
        uint8_t len=get_msq(&lexi_rsp_msgq, lexi_buf_to_proc, 50);
        if(len>0)
        {
            if(strstr((char*)lexi_buf_to_proc, (char*)expect_resp)!=NULL) return 1;
            if(strstr((char*)lexi_buf_to_proc, "ERROR")!=NULL) return 0;
        }
    }
    return 0;
}
static int lexi_at_cmd_send(uint8_t* cmd, uint8_t* final_expect, uint32_t timeout_ms)
{
    k_msgq_purge(&lexi_rsp_msgq);
    uart_tx(uart1, cmd, strlen((char*)cmd), SYS_FOREVER_US);
    return wait_for_final_response(final_expect, timeout_ms);
}
int lexi_com_check(const struct device_t* dev)
{
    const struct lexi_data_t* data=(const struct lexi_data_t*)dev->data;
    return lexi_at_cmd_send(data->cmd.com_check, data->rsp.com_check, COM_CHECK_MS);
}
int lexi_sim_check(const struct device_t* dev)
{
    const struct lexi_data_t* data=(const struct lexi_data_t*)dev->data;
    return lexi_at_cmd_send(data->cmd.sim_check, data->rsp.sim_check, SIM_CHECK_MS);
}
int lexi_nw_regis_check(const struct device_t* dev)
{
    const struct lexi_data_t* data=(const struct lexi_data_t*)dev->data;
    return lexi_at_cmd_send(data->cmd.nw_regis_check, data->rsp.nw_regis_check, NW_REGIS_CHECK_MS);
}
int lexi_apn_check(const struct device_t* dev)
{
    const struct lexi_data_t* data=(const struct lexi_data_t*)dev->data;
    return lexi_at_cmd_send(data->cmd.apn_check, data->rsp.apn_check, APN_CHECK_MS);
}
int lexi_ps_attach_check(const struct device_t* dev)
{
    const struct lexi_data_t* data=(const struct lexi_data_t*)dev->data;
    return lexi_at_cmd_send(data->cmd.ps_attach_check, data->rsp.ps_attach_check, PS_ATTACH_CHECK_MS);
}
int lexi_pdp_active_check(const struct device_t* dev)
{
    const struct lexi_data_t* data=(const struct lexi_data_t*)dev->data;
    return lexi_at_cmd_send(data->cmd.pdp_active_check, data->rsp.pdp_active_check, PDP_ACTIVE_CHECK_MS);
}

int lexi_sleep_enter(const struct device_t* dev)
{
    const struct lexi_data_t* data=(const struct lexi_data_t*)dev->data;
    lexi_pwrkey_pull_low();
    uint32_t start_time=k_uptime_get_32();
    while(k_uptime_get_32()-start_time<PWRKEY_PIN_LOW_TIME);
    lexi_pwrkey_idle();
    return lexi_at_cmd_send(data->cmd.hibernate_mode_enter, data->rsp.hibernate_mode_enter, HIBERNATE_MODE_MS);
}
int lexi_wakeup(const struct device_t* dev)
{
    const struct lexi_data_t* data=(const struct lexi_data_t*)dev->data;
    return lexi_at_cmd_send(data->cmd.wakeup, data->rsp.wakeup, WAKEUP_MS);
}
void lexi_hw_reset()
{
    uart_rx_active_disb=true;
    uart_rx_disable(uart1);
    k_msgq_purge(&lexi_rsp_msgq);
    lexi_reset_pin_pull_low();
    uint32_t start_time=k_uptime_get_32();
    while(k_uptime_get_32()-start_time<RESET_PIN_LOW_TIME);
    lexi_reset_pin_idle();
    start_time=k_uptime_get_32();
    while(k_uptime_get_32()-start_time<HW_RESET_MS);
}
int lexi_sw_reset(const struct device_t* dev)
{
    const struct lexi_data_t* data=(const struct lexi_data_t*)dev->data;
    return lexi_at_cmd_send(data->cmd.sw_reset, data->rsp.sw_reset, SW_RESET_MS);
}
void lexi_pwron()
{
    lexi_pwrkey_pull_low();
    uint32_t start_time=k_uptime_get_32();
    while(k_uptime_get_32()-start_time<PWRKEY_PIN_LOW_TIME);
    lexi_pwrkey_idle();
    start_time=k_uptime_get_32();
    while(k_uptime_get()-start_time<PWRON_MS);
}
void lexi_pwroff(const struct device_t* dev)
{
    const struct lexi_data_t* data=(const struct lexi_data_t*)dev->data;
    uart_rx_active_disb=true;
    uart_rx_disable(uart1);
    k_msgq_purge(&lexi_rsp_msgq);
    lexi_pwrkey_pull_low();
    uint32_t start_time=k_uptime_get_32();
    while(k_uptime_get_32()-start_time<PWRKEY_PIN_LOW_TIME);
    lexi_pwrkey_idle();
    uart_tx(uart1, data->cmd.graceful_pwroff, strlen(data->cmd.graceful_pwroff), SYS_FOREVER_US);
    start_time=k_uptime_get_32();
    while(k_uptime_get_32()-start_time<PWROFF_MS);
}
int lexi_mqtt_server_open(const struct device_t* dev)
{
    const struct lexi_data_t* data=(const struct lexi_data_t*)dev->data;
    lexi_pwrkey_pull_low();
    uint32_t start_time = k_uptime_get_32();
    while(k_uptime_get_32()-start_time<PWRKEY_PIN_LOW_TIME);
    lexi_pwrkey_idle();
    return lexi_at_cmd_send(data->cmd.mqtt_server_open, data->rsp.mqtt_server_open, MQTT_SERVER_OPEN_MS);
}
int lexi_mqtt_server_conn(const struct device_t* dev)
{
    const struct lexi_data_t* data=(const struct lexi_data_t*)dev->data;
    lexi_pwrkey_pull_low();
    uint32_t start_time=k_uptime_get_32();
    while (k_uptime_get_32()-start_time<PWRKEY_PIN_LOW_TIME);
    lexi_pwrkey_idle();
    return lexi_at_cmd_send(data->cmd.mqtt_server_conn, data->rsp.mqtt_server_conn, MQTT_SERVER_CONN_MS);
}
static void combine_string(const char* pub_cmd, char* payload, char* result)
{
    const char* end_cmd="\"\r\n";
    strcpy(result, pub_cmd);
    strcat(result, payload);
    strcat(result, end_cmd);
}
int lexi_mqtt_pub(const struct device_t* dev)
{
    const struct lexi_data_t* data=(const struct lexi_data_t*)dev->data;
    combine_string(data->cmd.mqtt_pub, pub_data, pub_str);
    // combine_string(data->cmd.mqtt_pub, "AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA", pub_str);
    lexi_pwrkey_pull_low();
    uint32_t start_time=k_uptime_get_32();
    while(k_uptime_get_32()-start_time<PWRKEY_PIN_LOW_TIME);
    lexi_pwrkey_idle();
    int ret=lexi_at_cmd_send(pub_str, data->rsp.mqtt_pub, MQTT_PUB_MS);
    return ret;
}
int lexi_mqtt_pub_test(const struct device_t* dev, char* pub_data_test)
{
    const struct lexi_data_t* data=(const struct lexi_data_t*)dev->data;
    combine_string(data->cmd.mqtt_pub, pub_data_test, pub_str);
    lexi_pwrkey_pull_low();
    uint32_t start_time=k_uptime_get_32();
    while(k_uptime_get_32()-start_time<PWRKEY_PIN_LOW_TIME);
    lexi_pwrkey_idle();
    int ret=lexi_at_cmd_send(pub_str, data->rsp.mqtt_pub, MQTT_PUB_MS);
    return ret;
}
int lexi_mqtt_server_disc(const struct device_t* dev)
{
    const struct lexi_data_t* data=(const struct lexi_data_t*)dev->data;
    lexi_pwrkey_pull_low();
    uint32_t start_time=k_uptime_get_32();
    while(k_uptime_get_32()-start_time<PWRKEY_PIN_LOW_TIME);
    lexi_pwrkey_idle();
    return lexi_at_cmd_send(data->cmd.mqtt_server_disc, data->rsp.mqtt_server_disc, MQTT_SERVER_DISC_MS);
}
const struct lexi_data_t lexi_data=
{
    .cmd=
    {
        .com_check="AT\r\n",
        .sim_check="AT+CPIN?\r\n",
        .nw_regis_check="AT+CEREG?\r\n",
        .apn_check="AT+CGDCONT?\r\n",
        .ps_attach_check="AT+CGATT?\r\n",
        .pdp_active_check="AT+CGACT?\r\n",
        .hibernate_mode_enter="AT+UPSV=1,1000,4\r\n",
        .wakeup="AT+UPSV=0\r\n",
        .sw_reset="AT+CFUN=16\r\n",
        .graceful_pwroff="AT+CPWROFF\r\n",
        .mqtt_server_open="AT+UMQTT=2,\"broker.emqx.io\",1883\r\n",
        .mqtt_server_conn="AT+UMQTTC=1\r\n",
        .mqtt_pub="AT+UMQTTC=2,2,0,1,\"my_topic\",\"",
        .mqtt_server_disc="AT+UMQTTC=0\r\n",
    },
    .rsp=
    {
        .com_check="OK\r\n",
        .sim_check="OK\r\n",
        .nw_regis_check="+CEREG: 0,1\r\n\r\nOK\r\n",
        .apn_check="+CGDCONT: 1,\"IP\",\"m-wap\"",
        .ps_attach_check="+CGATT: 1\r\n\r\nOK\r\n",
        .hibernate_mode_enter="OK",
        .wakeup="OK",
        .pdp_active_check="+CGACT: 1,1\r\n\r\nOK\r\n",
        .hw_reset="READY",
        .sw_reset="READY",
        .pwron="READY",
        .graceful_pwroff="OK",
        .mqtt_server_open="OK\r\n",
        .mqtt_server_conn="1,1\r\n",
        .mqtt_pub="2,1\r\n",
        .mqtt_server_disc="0,1\r\n",
    }
};
/*******************************************************************************************************/
/*                                           LEXI-R10 DRIVER END                                       */
/*******************************************************************************************************/