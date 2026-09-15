#ifndef IMU_CTRL_H_
#define IMU_CTRL_H_

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

#include "bmi270.h"      
#include "icm20948.h"
#include "bleCtrlTasks.h"
#include "hwDefine.h"

#define IMU_PWR_PIN 8
typedef struct 
{
    bool on;
} imu_pwr_state;

extern imu_pwr_state imu_pwr_stt;
void imu_pwr(bool state);

typedef enum
{
    IMU_SLEEP,
    IMU_NO_MOVE,
    IMU_MOVING
} imu_state;

typedef enum
{
    IMU_SLEEP_ENTER,
    IMU_NO_MOVE_ENTER,
    IMU_MOVING_ENTER
} imu_state_input;

typedef struct
{
    imu_state ns[3];
} imu_state_t;

extern imu_state imu_cs;
extern uint8_t* imu_stt2str[3];
void imu_finite_sm(imu_state_input input);

#define ACC_PINGPONG_SIZE 2048
extern imu_data_t acc_pingpong[2][ACC_PINGPONG_SIZE];
extern imu_data_t* acc_pingpong_wr_ptr;
extern imu_data_t* acc_pingpong_rd_ptr;
extern uint32_t acc_pingpong_wr_idx;

#define ACC_RAW_BUF_SIZE 1024
#define ACC_RESULT_BUF_SIZE 256
extern struct k_sem imu_rx_start_sem;
extern struct k_sem imu_rx_stop_sem;
extern struct k_event data_is_ready_evt;
extern struct k_event sensors_rx_stopped_evt;

struct imu_api_t
{
    int(*init)(const struct device_t* dev);
    int(*config)(const struct device_t* dev);
    int(*reg_read)(const struct device_t* dev, uint8_t reg_addr, uint8_t* data);
    int(*sleep)(const struct device_t* dev);
    int(*fifo_burst_read)(const struct device_t* dev, uint8_t* data, uint32_t* len_of_raw);
    void(*data_proc)(uint8_t raw[], uint32_t len_of_raw, imu_data_t result[], uint32_t* len_of_result);
};

/* Khai báo extern cho BMI270 device */
extern const struct device_t bmi270_dev;

static inline int imu_init(const struct device_t* dev)
{
    const struct imu_api_t* api=(const struct imu_api_t*)dev->api;
    int ret=api->init(dev); printk("\n    -- imu init ret: %d", ret);
    return ret;
}

static inline int imu_config(const struct device_t* dev)
{
    const struct imu_api_t* api=(const struct imu_api_t*)dev->api;
    int ret=api->config(dev); printk("\n    -- imu config ret: %d", ret);
    return ret;
}

static inline int imu_reg_read(const struct device_t* dev, uint8_t reg_addr, uint8_t* data)
{
    const struct imu_api_t* api=(const struct imu_api_t*)dev->api;
    int ret=api->reg_read(dev, reg_addr, data); printk("\n    -- imu reg read ret: %d", ret);
    return ret;
}

static inline int imu_sleep(const struct device_t* dev)
{
    const struct imu_api_t* api=(const struct imu_api_t*)dev->api;
    int ret=api->sleep(dev); printk("\n    -- imu sleep ret: %d", ret);
    return ret;
}

static inline int imu_fifo_burst_read(const struct device_t* dev, uint8_t* data, uint32_t* len_of_raw)
{
    const struct imu_api_t* api=(const struct imu_api_t*)dev->api;
    int ret=api->fifo_burst_read(dev, data, len_of_raw); printk("\n    -- imu fifo read ret: %d", ret);
    return ret; 
}

static inline void imu_data_proc(const struct device_t* dev, uint8_t raw[], uint32_t len_of_raw, imu_data_t result[], uint32_t* len_of_result)
{
    const struct imu_api_t* api=(const struct imu_api_t*)dev->api;
    api->data_proc(raw, len_of_raw, result, len_of_result);
}

#endif