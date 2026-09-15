#ifndef BMI270_H_
#define BMI270_H_

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

/*******************************************************************************************************/
/*                                    BMI-270 REGS DEFINE START                                       */
/*******************************************************************************************************/
#define BMI270_REG_CHIP_ID          0x00
#define BMI270_REG_INTERNAL_STATUS  0x21
#define BMI270_REG_FIFO_LENGTH_0    0x24
#define BMI270_REG_FIFO_LENGTH_1    0x25
#define BMI270_REG_FIFO_DATA        0x26
#define BMI270_REG_ACC_CONF         0x40
#define BMI270_REG_FIFO_CONFIG_0    0x48
#define BMI270_REG_FIFO_CONFIG_1    0x49
#define BMI270_REG_INIT_CTRL        0x59
#define BMI270_REG_INIT_ADDR_0      0x5B
#define BMI270_REG_INIT_ADDR_1      0x5C
#define BMI270_REG_INIT_DATA        0x5E
#define BMI270_REG_INTERNAL_ERROR   0x5F
#define BMI270_REG_PWR_CONF         0x7C
#define BMI270_REG_PWR_CTRL         0x7D
#define BMI270_REG_CMD              0x7E

#define BMI270_CHIP_ID_VAL          0x24
#define BMI270_CMD_SOFT_RESET       0xB6
#define BMI270_CONFIG_FILE_SIZE     8192
/*******************************************************************************************************/
/*                                    BMI-270 REGS DEFINE END                                         */
/*******************************************************************************************************/

/*******************************************************************************************************/
/*                                     BMI-270 DRIVER START                                           */
/*******************************************************************************************************/


int bmi270_init(const struct device_t* dev);
int bmi270_config(const struct device_t* dev);
int bmi270_fifo_burst_read(const struct device_t* dev, uint8_t* data, uint32_t* len_of_raw);
int bmi270_reg_read(const struct device_t* dev, uint8_t reg_addr, uint8_t* data);
int bmi270_reg_write(const struct device_t* dev, uint8_t reg_addr, uint8_t data);
int bmi270_sleep(const struct device_t* dev);
void bmi270_data_proc(uint8_t raw[], uint32_t len_of_raw, imu_data_t result[], uint32_t* len_of_result);
/*******************************************************************************************************/
/*                                      BMI-270 DRIVER END                                             */
/*******************************************************************************************************/
#endif /* BMI270_H_ */