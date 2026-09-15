#ifndef BLE_CTRL_H_
#define BLE_CTRL_H_

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
#include <zephyr/sys/reboot.h>
#include "gnssCtrlTasks.h"
#include "lteCtrlTasks.h"
#include "imuCtrlTasks.h"
#include "gaugeCtrlTasks.h"
#include "ppgCtrlTasks.h"
/*
*
*
*
*
*/
#define ON 0
#define OFF 1
/*******************************************************************************************************/
/*                                  	BLE FSM IMPLEMENTATION START                                   */
/*******************************************************************************************************/
typedef enum
{
	BLE_CONN,
	BLE_DISC,
}ble_state;
typedef enum
{
	BLE_CONN_ENTER,
	BLE_DISC_ENTER,
}ble_state_input;
typedef struct 
{
	ble_state ns[2];
}ble_state_t;
extern ble_state ble_cs;
extern uint8_t* ble_stt2str[2];
void ble_finite_sm(ble_state_input input);
/*******************************************************************************************************/
/*                                  	BLE FSM IMPLEMENTATION END	                                   */
/*******************************************************************************************************/
/*
*/
/*******************************************************************************************************/
/*                                  	BLE SEND DATA FUNCTIONS START                                  */
/*******************************************************************************************************/
typedef enum
{
	BAT_STATE_EVT=0,
	CHARGE_EVT,
	DEVICE_STATE_EVT,
	GEOFENCE_OUT_EVT,
	ACC_DATA_EVT=5,
	PPG_DATA_EVT
}data_evt_type;
void ble_alert_framing(uint8_t evt_type, uint8_t cycle_id);
void ble_data_framing(uint8_t evt_type, uint8_t cycle_id, uint8_t pkt_idx, void* data);
int ble_send_imu_sample(const imu_data_t *sample);
extern struct k_sem acc_start_send_sem;
extern struct k_sem ppg_start_send_sem;
extern struct k_sem dev_stt_read_sem;
extern bool ble_connected;
#define INT8 1
#define INT16 2
#define INT32 4
int ble_send_packet(void* sensor_value, uint16_t size, uint16_t length);
int ble_send_single_value(void* sensor_value, uint16_t size);
void mylbsbc_ccc_mysensor_cfg_changed(const struct bt_gatt_attr *attr, uint16_t value);
void mylbsbc_ccc_mylng_cfg_changed(const struct bt_gatt_attr *attr, uint16_t value);
void update_phy(struct bt_conn *conn);
void set_tx_power(uint8_t handle_type, uint16_t handle, int8_t tx_pwr_lvl);
// static void read_conn_rssi(uint16_t handle, int8_t *rssi);
// int my_lbs_send_lng_notify(double lng_value);
/*******************************************************************************************************/
/*                                  	BLE SEND DATA FUNCTIONS END                                    */
/*******************************************************************************************************/
/*
*/
/*******************************************************************************************************/
/*                                  	BLE PARAMS & CB DEFINE START                             	   */
/*******************************************************************************************************/
extern struct bt_conn_cb connection_callbacks;
extern struct bt_lbs_cb lbs_callbacs;
extern struct bt_conn *my_conn;
extern const struct bt_le_adv_param *adv_param;
extern const struct bt_data ad[2];
extern const struct bt_data sd[1];
#define BT_UUID_LBS_VAL BT_UUID_128_ENCODE(0x00001523, 0x1212, 0xefde, 0x1523, 0x785feabcd123)
/** @brief Button Characteristic UUID. */
#define BT_UUID_LBS_BUTTON_VAL                                                                     \
	BT_UUID_128_ENCODE(0x00001524, 0x1212, 0xefde, 0x1523, 0x785feabcd123)
/** @brief LED Characteristic UUID. */
#define BT_UUID_LBS_LED_VAL BT_UUID_128_ENCODE(0x00001525, 0x1212, 0xefde, 0x1523, 0x785feabcd123)
/** @brief LED Characteristic UUID. */
#define BT_UUID_LBS_MYSENSOR_VAL                                                                   \
	BT_UUID_128_ENCODE(0x00001527, 0x1212, 0xefde, 0x1523, 0x785feabcd123)
#define BT_UUID_LBS_MYLNG_VAL                                                                   \
	BT_UUID_128_ENCODE(0x00001528, 0x1212, 0xefde, 0x1523, 0x785feabcd123)
#define BT_UUID_LBS BT_UUID_DECLARE_128(BT_UUID_LBS_VAL)
#define BT_UUID_LBS_BUTTON BT_UUID_DECLARE_128(BT_UUID_LBS_BUTTON_VAL)
#define BT_UUID_LBS_LED BT_UUID_DECLARE_128(BT_UUID_LBS_LED_VAL)
#define BT_UUID_LBS_MYSENSOR BT_UUID_DECLARE_128(BT_UUID_LBS_MYSENSOR_VAL)
#define BT_UUID_LBS_MYLNG BT_UUID_DECLARE_128(BT_UUID_LBS_MYLNG_VAL)
#define DEVICE_NAME CONFIG_BT_DEVICE_NAME
#define DEVICE_NAME_LEN (sizeof(DEVICE_NAME) - 1)
/*******************************************************************************************************/
/*                                  	BLE PARAMS & CB DEFINE END                                 	   */
/*******************************************************************************************************/
/*
*
*
*
*
*/
#endif