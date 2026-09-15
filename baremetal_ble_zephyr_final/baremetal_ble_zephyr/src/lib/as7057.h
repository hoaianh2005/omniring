#ifndef AS7057_LIB_H_
#define AS7057_LIB_H_

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
/*                                        AS7057 REGS DEFINE START                                     */
/*******************************************************************************************************/
#define CONTROL 0x10
#define CGB_CFG 0x11
#define INT_CFG 0x12
#define CSXN_CFG 0x13
#define IO_CFG 0x14
#define REF_CFGA 0x15
#define REF_CFGB 0x16
#define MOD_CFGA 0x19
#define MOD1_CFGA 0x1B
#define MOD1_CFGB 0x1C
#define MOD1_CFGC 0x1D
#define MOD1_CFGD 0x1E
#define MOD1_CFGE 0x1F
#define MOD1_CFGF 0x20
#define MOD2_CFGA 0x21
#define MOD2_CFGB 0x22
#define MOD2_CFGC 0x23
#define MOD2_CFGD 0x24
#define SEQ1_LED1_CURR 0x29
#define SEQ2_LED1_CURR 0x2A
#define SEQ1_LED2_CURR 0x2B
#define SEQ1_LED3_CURR 0x2D
#define LED_SEQ1_SUB12 0x2F
#define LED_SEQ2_SUB12 0x33
#define IRQ_ENABLE 0x3F
#define SEQ_SAMPLE 0x40
#define SEQ_SUB_WAIT 0x41
#define SEQ_MODCONF 0x42
#define SEQ_CONFIG 0x43
#define SEQ_SAR_WAIT 0x44
#define SEQ_LED_INIT 0x45
#define SEQ_FREQL 0x46
#define SEQ_FREQH 0x47
#define SEQ1_FREQDIVL 0x48
#define SEQ1_FREQDIVH 0x49
#define SEQ2_FREQDIVL 0x4A
#define MOD1_SEQ1_SUB_EN 0x4C
#define MOD1_SEQ2_SUB_EN 0x4D
#define SEQ1_MODE_A 0x50
#define PD_SEQ1_SUB1 0x53
#define PD_SEQ1_SUB2 0x54
#define PD_SEQ1_SUB3 0x55
#define PD_SEQ2_SUB1 0x5B
#define PDSEL_CFG 0x5F
#define SEQ1_SINC_CFGA 0x61
#define SEQ1_SINC_CFGB 0x62
#define SEQ1_SINC_CFGC 0x63
#define SEQ2_SINC_CFGA 0x64
#define SEQ2_SINC_CFGB 0x65
#define AOC_CFG 0x85
#define AOC_MOD1_THH 0x86
#define AOC_MOD1_THL 0x87
#define AOC_SAR_THRES 0x8A
#define MOD1_SEQ1_AOC_EN 0x8B
#define STANDBY_ON 0xA0
#define STANDBY_EN1 0xA1
#define STANDBY_EN2 0xA2
#define STANDBY_EN3 0xA3
#define STANDBY_EN4 0xA4
#define STANDBY_EN5 0xA5
#define STANDBY_EN6 0xA6
#define STANDBY_EN7 0xA7
#define FIFO_THRESHOLD 0xD0
#define FIFO_CTRL 0xD1
#define SILICON_ID 0xED
#define CHIP_CTRL 0xEF
#define SEQ_START 0xF0
#define STATUS_SEQ 0xF5
#define STATUS_ASAT 0xF7
#define STATUS 0xFA
#define FIFO_LEVEL0 0xFB
#define FIFOL 0xFD
/*******************************************************************************************************/
/*                                        AS7057 REGS DEFINE END                                       */
/*******************************************************************************************************/
/*
*/
/*******************************************************************************************************/
/*                                          AS7057 DRIVER START                                        */
/*******************************************************************************************************/
int as7057_init(const struct device_t* dev);
int as7057_config(const struct device_t* dev);
int as7057_fifo_burst_read(const struct device_t* dev, uint8_t* data, uint32_t* len_of_raw);
int as7057_reg_read(const struct device_t* dev, uint8_t reg_addr, uint8_t* data);
void as7057_data_proc(uint8_t raw[], uint32_t len_of_raw, int32_t result[], uint8_t* len_of_result);
/*******************************************************************************************************/
/*                                           AS7057 DRIVER END                                         */
/*******************************************************************************************************/
/*
*
*
*
*
*/
#endif