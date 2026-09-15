#ifndef LEXI_R10_LIB_H_
#define LEXI_R10_LIB_H_

#include <string.h>
#include <stdlib.h>
#include <math.h>
#include <stdint.h>
#include <stdio.h>
#include <stddef.h>
#include <ctype.h>
#include <zephyr/kernel.h>
#include <zephyr/pm/device.h>
#include <zephyr/drivers/gpio.h>
#include <zephyr/drivers/uart.h>
#include <zephyr/drivers/i2c.h>
#include <zephyr/types.h>
#include <zephyr/settings/settings.h>
#include <zephyr/sys/printk.h>
#include <zephyr/sys/util.h>
#include <zephyr/sys/byteorder.h>
#include <zephyr/bluetooth/bluetooth.h>
#include <zephyr/bluetooth/hci.h>
#include <zephyr/bluetooth/hci_vs.h>
#include <zephyr/bluetooth/gap.h>
#include <zephyr/bluetooth/uuid.h>
#include <zephyr/bluetooth/conn.h>
#include <bluetooth/services/lbs.h>
#include "hwDefine.h"
/*
*
*
*
*
*/
/*******************************************************************************************************/
/*                                      LEXI-R10 TIMEOUT DEFINE START                                  */
/*******************************************************************************************************/
#define COM_CHECK_MS 1000
#define SIM_CHECK_MS 1000
#define NW_REGIS_MS
#define NW_REGIS_CHECK_MS 1000
#define CS_REGIS_MS
#define CS_REGIS_CHECK_MS
#define APN_SET_MS
#define APN_CHECK_MS 1000
#define PS_ATTACH_MS
#define PS_ATTACH_CHECK_MS 1000
#define PDP_ACTIVE_MS
#define PDP_ACTIVE_CHECK_MS 1000
#define HW_RESET_MS 3000
#define SW_RESET_MS 3000
#define MQTT_SERVER_OPEN_MS 5000
#define MQTT_SERVER_CONN_MS 10000
#define MQTT_PUB_MS 5000
#define MQTT_SERVER_DISC_MS 5000
#define HIBERNATE_MODE_MS 3000
#define WAKEUP_MS 3000
#define PWRON_MS 3000
#define PWROFF_MS 3000
/*******************************************************************************************************/
/*                                      LEXI-R10 TIMEOUT DEFINE END                                    */
/*******************************************************************************************************/
/*
*/
/*******************************************************************************************************/
/*                                         LEXI-R10 DRIVER START                                       */
/*******************************************************************************************************/
typedef struct 
{
    uint8_t* com_check;
    uint8_t* sim_check;
    uint8_t* nw_regis;
    uint8_t* nw_regis_check;
    uint8_t* cs_regis;
    uint8_t* cs_regis_check;
    uint8_t* apn_set;
    uint8_t* apn_check;
    uint8_t* ps_attach;
    uint8_t* ps_attach_check;
    uint8_t* pdp_active;
    uint8_t* pdp_active_check;
    uint8_t* sw_reset;
    uint8_t* mqtt_server_open;
    uint8_t* mqtt_server_conn;
    uint8_t* topic_sub;
    uint8_t* mqtt_pub;
    uint8_t* mqtt_server_disc;
    uint8_t* hibernate_mode_enter;
    uint8_t* wakeup;
    uint8_t* graceful_pwroff;
}lexi_cmd;
typedef struct 
{
    uint8_t* com_check;
    uint8_t* sim_check;
    uint8_t* nw_regis;
    uint8_t* nw_regis_check;
    uint8_t* cs_regis;
    uint8_t* cs_regis_check;
    uint8_t* apn_set;
    uint8_t* apn_check;
    uint8_t* ps_attach;
    uint8_t* ps_attach_check;
    uint8_t* pdp_active;
    uint8_t* pdp_active_check;
    uint8_t* hw_reset;
    uint8_t* sw_reset;
    uint8_t* mqtt_server_open;
    uint8_t* mqtt_server_conn;
    uint8_t* topic_sub;
    uint8_t* mqtt_pub;
    uint8_t* mqtt_server_disc;
    uint8_t* hibernate_mode_enter;
    uint8_t* wakeup;
    uint8_t* graceful_pwroff;
    uint8_t* pwron;
}lexi_rsp;
struct lexi_data_t
{
    lexi_cmd cmd;
    lexi_rsp rsp;
};
extern const struct lexi_data_t lexi_data;
void uart1_cb_handler(const struct device *dev, struct uart_event *evt, void *user_data);
int lexi_com_check(const struct device_t* dev);
int lexi_sim_check(const struct device_t* dev);
int lexi_nw_regis_check(const struct device_t* dev);
int lexi_apn_check(const struct device_t* dev);
int lexi_ps_attach_check(const struct device_t* dev);
int lexi_pdp_active_check(const struct device_t* dev);
int lexi_sleep_enter(const struct device_t* dev);
int lexi_wakeup(const struct device_t* dev);
void lexi_hw_reset();
int lexi_sw_reset(const struct device_t* dev);
void lexi_pwron();
void lexi_pwroff(const struct device_t* dev);
int lexi_mqtt_server_open(const struct device_t* dev);
int lexi_mqtt_server_conn(const struct device_t* dev);
int lexi_mqtt_pub(const struct device_t* dev);
int lexi_mqtt_server_disc(const struct device_t* dev);

int lexi_tracking_status_alert(const struct device_t* dev, char* message);

#define PWRKEY_PIN 21
#define PWRKEY_PIN_LOW_TIME 100
#define lexi_pwrkey_pull_low() gpio_pin_set(gpio0, PWRKEY_PIN, 0);
#define lexi_pwrkey_idle() gpio_pin_set(gpio0, PWRKEY_PIN, 1);

#define RESET_PIN 20
#define RESET_PIN_LOW_TIME 1
#define lexi_reset_pin_pull_low() gpio_pin_set(gpio0, RESET_PIN, 0);
#define lexi_reset_pin_idle() gpio_pin_set(gpio0, RESET_PIN, 1);

#define LTE_RSP_BUF_SIZE 64
extern uint8_t lexi_rsp_buf[2][LTE_RSP_BUF_SIZE];
extern char pub_data[256];

int lexi_mqtt_pub_test(const struct device_t* dev, char* pub_data_test);
/*******************************************************************************************************/
/*                                         LEXI-R10 DRIVER END                                         */
/*******************************************************************************************************/
/*
*
*
*
*
*/
#endif