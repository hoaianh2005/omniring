#ifndef GAUGE_CTRL_H_
#define GAUGE_CTRL_H_

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
#include "bq27220.h"
#include "bleCtrlTasks.h"
#include "hwDefine.h"
/*
*
*
*
*
*/
#define CHARGE_INDICATOR_LED 15
typedef struct 
{
    bool on;
}gauge_pwr_state;
extern gauge_pwr_state gauge_pwr_stt;
void gauge_pwr(bool state);
/*******************************************************************************************************/
/*                                   GAUGE FSM IMPLEMENTATION START                                    */
/*******************************************************************************************************/
typedef enum
{
    GAUGE_LOWBAT,
	GAUGE_NORMALBAT,
	GAUGE_CHARGINGBAT
}gauge_state;
typedef enum
{
    GAUGE_LOWBAT_ENTER,
	GAUGE_NORMALBAT_ENTER,
	GAUGE_CHARGINGBAT_ENTER
}gauge_state_input;
typedef struct
{
	gauge_state ns[3];
}gauge_state_t;
extern gauge_state gauge_cs;
extern uint8_t* gauge_stt2str[3];
void gauge_finite_sm(gauge_state_input input);
/*******************************************************************************************************/
/*                                   GAUGE FSM IMPLEMENTATION END                                      */
/*******************************************************************************************************/
/*
*/
/*******************************************************************************************************/
/*                                     GAUGE READ DATA/SIGS START                                      */
/*******************************************************************************************************/
#define STAT_PIN_DBC_TIME 10 // miliseconds
extern struct k_sem pin_soc_read_sem;
void gauge_low_bat_cb_handler(const struct device *dev, struct gpio_callback *cb, uint32_t pins);
void gauge_charging_cb_handler(const struct device *dev, struct gpio_callback *cb, uint32_t pins);
/*******************************************************************************************************/
/*                                     GAUGE READ DATA/SIGS END                                        */
/*******************************************************************************************************/
/*
*/
/*******************************************************************************************************/
/*                                        GAUGE API FUNCTIONS START                                    */
/*******************************************************************************************************/
struct gauge_api
{
    int(*soc_read)(const struct device_t* dev, uint8_t* ret);
    int(*temp_read)(const struct device_t* dev, uint8_t* ret);
    int(*status_check)(const struct device_t* dev, bq27220_status_ret* ret);
    int(*irq_config)(const struct device_t* dev);
};

extern const struct device_t bq27220_dev;
static inline int gauge_soc_read(const struct device_t* dev, uint8_t* soc_ret)
{
    const struct gauge_api* api=(const struct gauge_api*)dev->api;
    int ret=api->soc_read(dev, soc_ret); printk("\n    -- gauge soc read ret: %d", ret);
    return ret;
}
static inline int gauge_temp_read(const struct device_t* dev, uint8_t* soc_ret)
{
    const struct gauge_api* api=(const struct gauge_api*)dev->api;
    int ret=api->temp_read(dev, soc_ret); printk("\n    -- gauge temp read ret: %d", ret);
    return ret;
}
static inline int gauge_status_check(const struct device_t* dev, bq27220_status_ret* stt_ret)
{
    const struct gauge_api* api=(const struct gauge_api*)dev->api;
    int ret=api->status_check(dev, stt_ret); printk("\n    -- gauge status check ret: %d", ret);
    return ret;
}
static inline int gauge_irq_config(const struct device_t* dev)
{
    const struct gauge_api* api=(const struct gauge_api*)dev->api;
    int ret=api->irq_config(dev); printk("\n    -- gauge irq config ret: %d", ret);
    return ret;
}
/*******************************************************************************************************/
/*                                        GAUGE API FUNCTIONS END                                      */
/*******************************************************************************************************/
/*
*
*
*
*
*/
#endif