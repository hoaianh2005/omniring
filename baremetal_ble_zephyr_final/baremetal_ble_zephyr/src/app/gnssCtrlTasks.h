#ifndef GNSS_CTRL_H_
#define GNSS_CTRL_H_

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
#include "bleCtrlTasks.h"
/*
*
*
*
*
*/
#define GNSS_PWR_PIN 24
typedef struct 
{
    bool on;
}gnss_pwr_state;
extern gnss_pwr_state gnss_pwr_stt;
void gnss_pwr(bool state);
/*******************************************************************************************************/
/*                                     GNSS FSM IMPLEMENTATION START                                   */
/*******************************************************************************************************/
typedef enum
{
	GNSS_PWROFF,
	GNSS_ACQUI,
	GNSS_TRACKING,
}gnss_state;
typedef enum
{
	GNSS_PWROFF_ENTER,
	GNSS_ACQUI_ENTER,
	GNSS_TRACKING_ENTER,
}gnss_state_input;
typedef struct 
{
	gnss_state ns[3];
}gnss_state_t;
extern gnss_state gnss_cs;
extern uint8_t* gnss_stt2str[3];
void gnss_finite_sm(gnss_state_input input);
/*******************************************************************************************************/
/*                                     GNSS FSM IMPLEMENTATION END                                     */
/*******************************************************************************************************/
/*
*/
/*******************************************************************************************************/
/*                                    GNSS READ & PROC DATA START                                      */
/*******************************************************************************************************/
#define NMEA_BUF_SIZE 1024
#define GEOFENCING_R 50 // meters
#define MINI_GEOFENCING_R 10 // meters
void uart0_cb_handler(const struct device *dev, struct uart_event *evt, void *user_data);
extern struct k_sem gnss_rx_start_sem;
extern struct k_sem gnss_rx_done_sem;
extern struct k_sem gnss_rx_stop_sem;
#define LOCATION_DATA_PINGPONG_SIZE 128
typedef struct
{
	int32_t lat;
	int32_t lon;
}location_data_t;
extern location_data_t location_data;
extern location_data_t location_data_pingpong[2][LOCATION_DATA_PINGPONG_SIZE];
extern location_data_t* location_data_pingpong_wr_ptr;
extern uint32_t location_data_pingpong_wr_idx;
extern struct k_event modules_rx_stopped_evt;
/*******************************************************************************************************/
/*                                    GNSS READ & PROC DATA END                                        */
/*******************************************************************************************************/
/*
*
*
*
*
*/
#endif