#ifndef BQ27220_LIB_H_
#define BQ27220_LIB_H_

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
/*                                      BQ27220 REGS DEFINE START                                      */
/*******************************************************************************************************/
#define Voltage 0x08 
#define BatteryStatus 0x0A              
#define RelativeStateOfCharge 0x2C
#define OperationStatus 0x3A
#define UpdateConfig_Port 0x3E
#define MACData 0x40
#define MACDataSum 0x60
#define MACDataLen 0x61
#define RawVoltage 0x7C
#define InternalTemperature 0x28
#define TDA_BIT (1<<2)
#define DSG_BIT (1<<0)
#define LOW_BAT_THRESHOLD 0x0CE4 // 3300mV
typedef enum
{
    BQ_LOW_BAT,
    BQ_NORMAL_BAT,
}bq27220_status_ret;
/*******************************************************************************************************/
/*                                        BQ27220 REGS DEFINE END                                      */
/*******************************************************************************************************/
/*
*/
/*******************************************************************************************************/
/*                                          BQ27220 DRIVER START                                       */
/*******************************************************************************************************/
int bq_soc_read(const struct device_t* dev, uint8_t* ret);
int bq_temp_read(const struct device_t* dev, uint8_t* ret);
int bq_status_check(const struct device_t* dev, bq27220_status_ret* stt_ret);
int bq_irq_config(const struct device_t* dev);
/*******************************************************************************************************/
/*                                          BQ27220 DRIVER END                                         */
/*******************************************************************************************************/
/*
*
*
*
*
*/
#endif