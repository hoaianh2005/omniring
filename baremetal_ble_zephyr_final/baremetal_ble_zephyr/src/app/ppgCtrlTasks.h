#ifndef BIO_CTRL_H_
#define BIO_CTRL_H_

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
#include "as7057.h"
#include "bleCtrlTasks.h"
/*
*
*
*
*
*/
#define PPG_PWR_PIN 31 
typedef struct 
{
    bool on;  
}ppg_pwr_state;
extern ppg_pwr_state ppg_pwr_stt;
void ppg_pwr(bool state);
/*******************************************************************************************************/
/*                                     PPG FSM IMPLEMENTATION START                                    */
/*******************************************************************************************************/
typedef enum
{
    PPG_PWROFF,
	PPG_MEASURE
}ppg_state;
typedef enum
{
    PPG_PWROFF_ENTER,
	PPG_MEASURE_ENTER
}ppg_state_input;
typedef struct
{
	ppg_state ns[2];
}ppg_state_t;
extern ppg_state ppg_cs;
extern uint8_t* ppg_stt2str[2];
void ppg_finite_sm(ppg_state_input input);
/*******************************************************************************************************/
/*                                     PPG FSM IMPLEMENTATION END                                      */
/*******************************************************************************************************/
/*
*/
/*******************************************************************************************************/
/*                                      PPG READ & PROC DATA START                                     */
/*******************************************************************************************************/
#define PPG_PINGPONG_SIZE 2048
extern int32_t ppg_pingpong[2][PPG_PINGPONG_SIZE];
extern int32_t* ppg_pingpong_wr_ptr;
extern int32_t* ppg_pingpong_rd_ptr;
extern uint32_t ppg_pingpong_wr_idx;
#define PPG_RAW_BUF_SIZE 1024
#define PPG_RESULT_BUF_SIZE 128
extern struct k_sem ppg_rx_start_sem;
extern struct k_sem ppg_rx_stop_sem;
/*******************************************************************************************************/
/*                                      PPG READ & PROC DATA END                                       */
/*******************************************************************************************************/
/*
*/
/*******************************************************************************************************/
/*                                          PPG API FUNCTIONS START                                    */
/*******************************************************************************************************/
struct ppg_api_t
{
    int(*init)(const struct device_t* dev);
    int(*config)(const struct device_t* dev);
    int(*reg_read)(const struct device_t* dev, uint8_t reg_addr, uint8_t* data);
    int(*fifo_burst_read)(const struct device_t* dev, uint8_t* data, uint32_t* len_of_raw);
    void(*data_proc)(uint8_t raw[], uint32_t len_of_raw, int32_t result[], uint8_t* len_of_result);
};
extern const struct device_t as7057_dev;
static inline int ppg_init(const struct device_t* dev)
{
    const struct ppg_api_t* api=(const struct ppg_api_t*)dev->api;
    int ret=api->init(dev); printk("\n    -- ppg init ret: %d", ret);
    return ret;
}
static inline int ppg_config(const struct device_t* dev)
{
    const struct ppg_api_t* api=(const struct ppg_api_t*)dev->api;
    int ret=api->config(dev); printk("\n    -- ppg config ret: %d", ret);
    return ret;
}
static inline int ppg_reg_read(const struct device_t* dev, uint8_t reg_addr, uint8_t* data)
{
    const struct ppg_api_t* api=(const struct ppg_api_t*)dev->api;
    int ret=api->reg_read(dev, reg_addr, data); printk("\n    -- ppg reg read ret: %d", ret);
    return ret;
}
static inline int ppg_fifo_burst_read(const struct device_t* dev, uint8_t* data, uint32_t* len_of_raw)
{
    const struct ppg_api_t* api=(const struct ppg_api_t*)dev->api;
    int ret=api->fifo_burst_read(dev, data, len_of_raw); printk("\n    -- ppg fifo read ret: %d", ret);
    return ret;
}
static inline void ppg_data_proc(const struct device_t* dev, uint8_t raw[], uint32_t len_of_raw, int32_t result[], uint8_t* len_of_result)
{
    const struct ppg_api_t* api=(const struct ppg_api_t*)dev->api;
    api->data_proc(raw, len_of_raw, result, len_of_result);
}
/*******************************************************************************************************/
/*                                          PPG API FUNCTIONS END                                      */
/*******************************************************************************************************/
/*
*
*
*
*
*/
#endif