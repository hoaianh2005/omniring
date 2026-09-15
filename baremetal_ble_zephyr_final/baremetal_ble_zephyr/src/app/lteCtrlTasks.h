#ifndef LTE_CTRL_H_
#define LTE_CTRL_H_

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
#include <zephyr/sys/util.h>
#include <zephyr/sys/byteorder.h>
#include <zephyr/bluetooth/bluetooth.h>
#include <zephyr/bluetooth/hci.h>
#include <zephyr/bluetooth/hci_vs.h>
#include <zephyr/bluetooth/hci_types.h>
#include <zephyr/bluetooth/gap.h>
#include <zephyr/bluetooth/gatt.h>
#include <zephyr/bluetooth/uuid.h>
#include <zephyr/bluetooth/conn.h>
#include <bluetooth/services/lbs.h>
#include "hwDefine.h"
#include "lexiR10.h"
#include "bleCtrlTasks.h"
/*
*
*
*
*
*/
#define LTE_PWR_PIN 13
typedef struct 
{
    bool on;
}lte_pwr_state;
extern lte_pwr_state lte_pwr_stt;
void lte_pwr(bool state);
/*******************************************************************************************************/
/*                                     LTE FSM IMPLEMENTATION START                                    */
/*******************************************************************************************************/
typedef enum
{
	LTE_PWROFF,
	LTE_SLEEP,
	LTE_ACTIVE
}lte_state;
typedef enum
{
	LTE_PWROFF_ENTER,
	LTE_SLEEP_ENTER,
	LTE_ACTIVE_ENTER
}lte_state_input;
typedef struct 
{
	lte_state ns[3];
}lte_state_t;
extern lte_state lte_cs;
extern uint8_t* lte_stt2str[3];
void lte_finite_sm(lte_state_input input);
extern struct k_event lte_ctrl_thread_evt;
extern struct k_timer lte_start_tx_timer;
/*******************************************************************************************************/
/*                                     LTE FSM IMPLEMENTATION END                                      */
/*******************************************************************************************************/
/*
*/
/*******************************************************************************************************/
/*                                          LTE API FUNCTIONS START                                    */
/*******************************************************************************************************/
struct lte_api_t
{
    int(*lte_com_check)(const struct device_t* dev);
	int(*lte_sim_check)(const struct device_t* dev);
	int(*lte_nw_regis_check)(const struct device_t* dev);
	int(*lte_apn_check)(const struct device_t* dev);
	int(*lte_ps_attach_check)(const struct device_t* dev);
	int(*lte_pdp_active_check)(const struct device_t* dev);
	int(*lte_sleep_enter)(const struct device_t* dev);
	int(*lte_wakeup)(const struct device_t* dev);
	void(*lte_hw_reset)();
	int(*lte_sw_reset)(const struct device_t* dev);
	void(*lte_pwron)();
	void(*lte_pwroff)(const struct device_t* dev);
	int(*lte_mqtt_server_open)(const struct device_t* dev);
	int(*lte_mqtt_server_conn)(const struct device_t* dev);
	int(*lte_mqtt_pub)(const struct device_t* dev);
	int(*lte_mqtt_server_disc)(const struct device_t* dev);
};
extern const struct device_t lexi_dev;
static inline int lte_com_check(const struct device_t* dev)
{
	const struct lte_api_t* api=(const struct lte_api_t*)dev->api;
	int ret=api->lte_com_check(dev); printk("\n    -- lte communication check ret: %d", ret);
    return ret;
}
static inline int lte_sim_check(const struct device_t* dev)
{
	const struct lte_api_t* api=(const struct lte_api_t*)dev->api;
	int ret=api->lte_sim_check(dev); printk("\n    -- lte sim check ret: %d", ret);
    return ret;
}
static inline void lte_turn_on(const struct device_t* dev)
{
	const struct lte_api_t* api=(const struct lte_api_t*)dev->api;
	api->lte_pwron(dev);
}
static inline void lte_turn_off(const struct device_t* dev)
{
	const struct lte_api_t* api=(const struct lte_api_t*)dev->api;
	api->lte_pwroff(dev);
}
static inline void lte_hw_reset(const struct device_t* dev)
{
	const struct lte_api_t* api=(const struct lte_api_t*)dev->api;
	api->lte_hw_reset(dev);
}
static inline int lte_sleep(const struct device_t* dev)
{
	const struct lte_api_t* api=(const struct lte_api_t*)dev->api;
	int ret=api->lte_sleep_enter(dev); printk("\n    -- lte sleep enter ret: %d", ret);
	return ret;
}
static inline int lte_mqtt_server_conn(const struct device_t* dev)
{
	const struct lte_api_t* api=(const struct lte_api_t*)dev->api;
	int ret=api->lte_mqtt_server_open(dev); printk("\n    -- lte server open ret: %d", ret);
	k_msleep(1000);
	ret=api->lte_mqtt_server_conn(dev); printk("\n    -- lte server connect ret: %d", ret);
	k_msleep(1000);
	return ret;
}
static inline int lte_send_data(const struct device_t* dev)
{
	const struct lte_api_t* api=(const struct lte_api_t*)dev->api;
	int ret=api->lte_mqtt_pub(dev); printk("\n    -- lte data send ret: %d", ret);
	return ret;
}
static inline int lte_mqtt_server_disc(const struct device_t* dev)
{
	const struct lte_api_t* api=(const struct lte_api_t*)dev->api;
	int ret=api->lte_mqtt_server_disc(dev); printk("\n    -- lte server disc ret: %d", ret);
	k_msleep(1000);
	return ret;
}
/*******************************************************************************************************/
/*                                          LTE API FUNCTIONS END                                      */
/*******************************************************************************************************/
/*
*
*
*
*
*/
#endif