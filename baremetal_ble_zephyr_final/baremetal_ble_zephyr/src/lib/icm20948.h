#ifndef ICM20948_LIB_H_
#define ICM20948_LIB_H_

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
/*                                      ICM-20948 REGS DEFINE START                                    */
/*******************************************************************************************************/
#define BANK_SEL_REG 0x7F
#define BANK0 0x00
#define BANK1 0x10
#define BANK2 0x20
#define BANK3 0x30
#define WHO_AM_I 0x00
#define LP_CONFIG 0x05
#define PWR_MGMT_1 0x06
#define PWR_MGMT_2 0x07
#define ORD_ALIGEN_EN 0x09
#define GYRO_SMPLRT_DIV 0x00
#define GYRO_CONFIG_1 0x01
#define ACCEL_SMPLRT_DIV_1 0x10
#define ACCEL_SMPLRT_DIV_2 0x11
#define ACCEL_CONFIG 0x14
#define ACCEL_CONFIG_2 0x15 
#define USER_CTRL 0x03
#define FIFO_EN_1 0x66
#define FIFO_EN_2 0x67
#define FIFO_MODE 0x69
#define FIFO_RST 0x68
#define INT_ENABLE_3 0x13
#define INT_PIN_CFG 0x0F
#define INT_ENABLE_1 0x11
#define ACCEL 0x2D 
#define FIFO_COUNT_H 0x70
#define FIFO_R_W 0x72
#define ACCEL_INTEL_CTRL 0x12
#define ACCEL_WOM_THR 0x13
#define INT_STATUS 0x19
/*******************************************************************************************************/
/*                                      ICM-20948 REGS DEFINE END                                      */
/*******************************************************************************************************/
/*
*/
/*******************************************************************************************************/
/*                                        ICM-20948 DRIVER START                                       */
/*******************************************************************************************************/

int icm20948_init(const struct device_t* dev);
int icm20948_config(const struct device_t* dev);
int icm20948_fifo_burst_read(const struct device_t* dev, uint8_t* data, uint32_t* len_of_raw);
int icm20948_reg_read(const struct device_t* dev, uint8_t reg_addr, uint8_t* data);
int icm20948_sleep(const struct device_t* dev);
void icm20948_data_proc(uint8_t raw[], uint32_t len_of_raw, imu_data_t result[], uint32_t* len_of_result);
/*******************************************************************************************************/
/*                                         ICM-20948 DRIVER END                                        */
/*******************************************************************************************************/
/*
*
*
*
*
*/
#endif