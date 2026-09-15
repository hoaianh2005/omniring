#ifndef HW_DEF_H_
#define HW_DEF_H_

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

/* Cấu trúc dữ liệu IMU dùng chung độc lập phần cứng */
typedef struct {
    int16_t x;
    int16_t y;
    int16_t z;
} imu_data_t;

extern struct k_msgq sys_evt_msgq;
typedef enum
{
    CONN_EVT,
    DISC_EVT,
    LOW_BAT_EVT,
    NORMAL_BAT_EVT,
    CHARGING_EVT,
    NO_WOM_EVT,
    WOM_EVT,
    NO_FIX_GPS_EVT,
    GEOF_IN_EVT,
    GEOF_OUT_EVT,
    BLE_DATA_RTS_EVT,
    LATLON_BUF_TIMEOUT_EVT,
    LTE_SEND_FAILED_EVT
} event_source;

typedef struct
{
    uint8_t evt_src;
} evt_q_t;

struct device_t
{
    const char* name;
    const void* data;
    const void* api;
};

extern const struct device* gpio0;
extern const struct device* gpio1;
extern const struct device* uart0;
extern const struct device* uart1;
//extern const struct i2c_dt_spec icm20948;
extern const struct i2c_dt_spec bmi270;
extern const struct i2c_dt_spec as7057;
extern const struct device *osram_dev;
extern const struct i2c_dt_spec bq27220;

#define uart0_resume() pm_device_action_run(uart0, PM_DEVICE_ACTION_RESUME)
#define uart1_resume() pm_device_action_run(uart1, PM_DEVICE_ACTION_RESUME)
#define uart0_suspend() pm_device_action_run(uart0, PM_DEVICE_ACTION_SUSPEND)
#define uart1_suspend() pm_device_action_run(uart1, PM_DEVICE_ACTION_SUSPEND)

#endif