/*
 * Copyright (c) 2023 Nordic Semiconductor ASA
 *
 * SPDX-License-Identifier: LicenseRef-Nordic-5-Clause
 */
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
#include <zephyr/logging/log.h>
#include <zephyr/drivers/watchdog.h>
#include <bluetooth/services/lbs.h>
#include "bleCtrlTasks.h"
#include "imuCtrlTasks.h"
#include "ppgCtrlTasks.h"
#include "gnssCtrlTasks.h"
#include "lteCtrlTasks.h"
#include "gaugeCtrlTasks.h"
#include "hwDefine.h"

#define IMU_SERIAL_DEBUG 1
/*
*
*
*
*
*/
/*******************************************************************************************************/
/*                                SYSTEM CONTROLLER IMPLEMENTATION START                               */
/*******************************************************************************************************/
bool start_program=true;
K_MSGQ_DEFINE(sys_evt_msgq,sizeof(evt_q_t),1024,4);
void system_ctrl_thread()
{
	evt_q_t evt_q_get;
	static uint8_t* evt_src_str[13]={"CONN_EVT", "DISC_EVT", "LOW_BAT_EVT", "NORMAL_BAT_EVT", "CHARGING_EVT", "NO_WOM_EVT", "WOM_EVT", "NO_FIX_GPS_EVT", "GEOF_IN_EVT", "GEOF_OUT_EVT", "BLE_DATA_RTS_EVT", "LATLON_BUF_TIMEOUT_EVT", "LTE_SEND_FAILED_EVT"}; //"IMU_OFF_EVT", "PPG_OFF_EVT", , "GNSS_OFF_EVT", "LTE_OFF_EVT"};
	while(1)
	{
		k_msgq_get(&sys_evt_msgq, &evt_q_get, K_FOREVER);
		printk("\nSYSCTRL: get %s event success!", evt_src_str[evt_q_get.evt_src]);
		switch(evt_q_get.evt_src)
		{
			case CONN_EVT:
			{
				ble_finite_sm(BLE_CONN_ENTER);
				if(gauge_cs==GAUGE_NORMALBAT) 
				{
					printk("\n-> SYSCTRL: ready stop gnss & lte!");
					k_sem_give(&gnss_rx_stop_sem);
					k_event_post(&lte_ctrl_thread_evt, 0x2);

					/* Tạm thời vô hiệu hóa việc chờ GNSS/LTE để test IMU */
					// k_event_wait_all(&modules_rx_stopped_evt, 0x3, false, K_FOREVER);
					// k_event_clear(&modules_rx_stopped_evt, 0x3);
					printk("\n-> SYSCTRL: stopped thread gnss & lte!");

					printk("\n-> SYSCTRL: location_data_pingpong_wr_idx now = %d", location_data_pingpong_wr_idx);
					location_data_pingpong_wr_idx=0;
					printk("\n-> SYSCTRL: location_data_pingpong_wr_idx now = %d", location_data_pingpong_wr_idx);

					gnss_finite_sm(GNSS_PWROFF_ENTER);
					lte_finite_sm(LTE_PWROFF_ENTER);

					printk("\n-> SYSCTRL: ready start imu & ppg!");
					imu_finite_sm(IMU_NO_MOVE_ENTER);
					ppg_finite_sm(PPG_MEASURE_ENTER);
				}
				break;
			}
			case DISC_EVT:
			{
				ble_finite_sm(BLE_DISC_ENTER);
				if(gauge_cs==GAUGE_NORMALBAT)
				{
					if(start_program) start_program=false;
					else 
					{
						printk("\n-> SYSCTRL: ready stop imu & ppg!");
						k_sem_give(&imu_rx_stop_sem);
						k_sem_give(&ppg_rx_stop_sem);

						k_event_wait_all(&sensors_rx_stopped_evt, 0x3, false, K_FOREVER); 
						k_event_clear(&sensors_rx_stopped_evt, 0x3);
						printk("\n-> SYSCTRL: stopped thread imu & ppg!");

						printk("\n-> SYSCTRL: acc_pingpong_wr_idx now = %d", acc_pingpong_wr_idx);
						acc_pingpong_wr_idx=0;
						printk("\n-> SYSCTRL: acc_pingpong_wr_idx now = %d", acc_pingpong_wr_idx);
						imu_finite_sm(IMU_SLEEP_ENTER);

						printk("\n-> SYSCTRL: ppg_pingpong_wr_idx now = %d", ppg_pingpong_wr_idx);
						ppg_pingpong_wr_idx=0;
						printk("\n-> SYSCTRL: ppg_pingpong_wr_idx now = %d", ppg_pingpong_wr_idx);
						ppg_finite_sm(PPG_PWROFF_ENTER);
					}
					printk("\n-> SYSCTRL: ready start lte!");
					#if !IMU_SERIAL_DEBUG
					gnss_finite_sm(GNSS_ACQUI_ENTER);
					#endif
					lte_finite_sm(LTE_SLEEP_ENTER);
				}
				break;
			}
			case LOW_BAT_EVT:
			{
                if(ble_cs==BLE_CONN && gauge_cs==GAUGE_NORMALBAT)
				{
					printk("\n-> SYSCTRL: ready stop imu & ppg!");
					k_sem_give(&imu_rx_stop_sem);
					k_sem_give(&ppg_rx_stop_sem);

					k_event_wait_all(&sensors_rx_stopped_evt, 0x3, false, K_FOREVER); 
					k_event_clear(&sensors_rx_stopped_evt, 0x3);
					printk("\n-> SYSCTRL: stopped thread imu & ppg!");

					printk("\n-> SYSCTRL: acc_pingpong_wr_idx now = %d", acc_pingpong_wr_idx);
					acc_pingpong_wr_idx=0;
					printk("\n-> SYSCTRL: acc_pingpong_wr_idx now = %d", acc_pingpong_wr_idx);
					imu_finite_sm(IMU_SLEEP_ENTER);

					printk("\n-> SYSCTRL: ppg_pingpong_wr_idx now = %d", ppg_pingpong_wr_idx);
					ppg_pingpong_wr_idx=0;
					printk("\n-> SYSCTRL: ppg_pingpong_wr_idx now = %d", ppg_pingpong_wr_idx);
					ppg_finite_sm(PPG_PWROFF_ENTER);
				}
				if(ble_cs==BLE_DISC && gauge_cs==GAUGE_NORMALBAT)
                {
					printk("\n-> SYSCTRL: ready stop gnss & lte!");
                    k_sem_give(&gnss_rx_stop_sem);
                    k_event_post(&lte_ctrl_thread_evt, 0x2);

					/* Tạm thời vô hiệu hóa việc chờ GNSS/LTE để test IMU */
					// k_event_wait_all(&modules_rx_stopped_evt, 0x3, false, K_FOREVER);
					// k_event_clear(&modules_rx_stopped_evt, 0x3);
					printk("\n-> SYSCTRL: stopped thread gnss & lte!");

					printk("\n-> SYSCTRL: location_data_pingpong_wr_idx now = %d", location_data_pingpong_wr_idx);
					location_data_pingpong_wr_idx=0;
					printk("\n-> SYSCTRL: location_data_pingpong_wr_idx now = %d", location_data_pingpong_wr_idx);

					gnss_finite_sm(GNSS_PWROFF_ENTER);
					lte_finite_sm(LTE_PWROFF_ENTER);
                }
                gauge_finite_sm(GAUGE_LOWBAT_ENTER);
				break;
			}
			case NORMAL_BAT_EVT:
			{
				gauge_finite_sm(GAUGE_NORMALBAT_ENTER);
				if(ble_cs==BLE_DISC) 
                {
					printk("\n-> SYSCTRL: serial debug mode -> start IMU!");
					/*
					 * Khi chưa có BLE connect, IMU vẫn phải chạy để debug sensor trên serial.
					 * Nếu muốn giữ logic app gốc, có thể bỏ dòng dưới và chỉ giữ GNSS/LTE.
					 */
					imu_finite_sm(IMU_NO_MOVE_ENTER);
					#if !IMU_SERIAL_DEBUG
					gnss_finite_sm(GNSS_ACQUI_ENTER);
					#endif
					lte_finite_sm(LTE_SLEEP_ENTER);
                }
				else
				{
					printk("\n-> SYSCTRL: ready start imu & ppg!");
					imu_finite_sm(IMU_NO_MOVE_ENTER);
					ppg_finite_sm(PPG_MEASURE_ENTER);
				}
				break;
			}
			case CHARGING_EVT:
			{	
                if(ble_cs==BLE_CONN && gauge_cs==GAUGE_NORMALBAT)
				{
					printk("\n-> SYSCTRL: ready stop imu & ppg!");
					k_sem_give(&imu_rx_stop_sem);
					k_sem_give(&ppg_rx_stop_sem);

					k_event_wait_all(&sensors_rx_stopped_evt, 0x3, false, K_FOREVER); 
					k_event_clear(&sensors_rx_stopped_evt, 0x3);
					printk("\n-> SYSCTRL: stopped thread imu & ppg!");

					printk("\n-> SYSCTRL: acc_pingpong_wr_idx now = %d", acc_pingpong_wr_idx);
					acc_pingpong_wr_idx=0;
					printk("\n-> SYSCTRL: acc_pingpong_wr_idx now = %d", acc_pingpong_wr_idx);
					imu_finite_sm(IMU_SLEEP_ENTER);

					printk("\n-> SYSCTRL: ppg_pingpong_wr_idx now = %d", ppg_pingpong_wr_idx);
					ppg_pingpong_wr_idx=0;
					printk("\n-> SYSCTRL: ppg_pingpong_wr_idx now = %d", ppg_pingpong_wr_idx);
					ppg_finite_sm(PPG_PWROFF_ENTER);
				}
				if(ble_cs==BLE_DISC && gauge_cs==GAUGE_NORMALBAT)
                {
					printk("\n-> SYSCTRL: ready stop gnss & lte!");
                    k_sem_give(&gnss_rx_stop_sem);
                    k_event_post(&lte_ctrl_thread_evt, 0x2);

					/* Tạm thời vô hiệu hóa việc chờ GNSS/LTE để test IMU */
					// k_event_wait_all(&modules_rx_stopped_evt, 0x3, false, K_FOREVER);
					// k_event_clear(&modules_rx_stopped_evt, 0x3);
					printk("\n-> SYSCTRL: stopped thread gnss & lte!");

					printk("\n-> SYSCTRL: location_data_pingpong_wr_idx now = %d", location_data_pingpong_wr_idx);
					location_data_pingpong_wr_idx=0;
					printk("\n-> SYSCTRL: location_data_pingpong_wr_idx now = %d", location_data_pingpong_wr_idx);

					gnss_finite_sm(GNSS_PWROFF_ENTER);
					lte_finite_sm(LTE_PWROFF_ENTER);
                }
				gauge_finite_sm(GAUGE_CHARGINGBAT_ENTER);
				break;
			}
			case NO_WOM_EVT:
			{
				if(ble_cs==BLE_CONN && gauge_cs==GAUGE_NORMALBAT) imu_finite_sm(IMU_NO_MOVE_ENTER);
				else printk("\n-> SYSCTRL: ble connected or gauge low/charging bat, skip NO_WOM_EVT!");
				break;
			}
			case WOM_EVT:
			{
				if(ble_cs==BLE_CONN && gauge_cs==GAUGE_NORMALBAT) imu_finite_sm(IMU_MOVING_ENTER);
				else printk("\n-> SYSCTRL: ble connected or gauge low/charging bat, skip WOM_EVT!");
				break;
			}
			case NO_FIX_GPS_EVT:
			{
				if(ble_cs==BLE_DISC && gauge_cs==GAUGE_NORMALBAT)
                {
					gnss_finite_sm(GNSS_ACQUI_ENTER);
					lte_finite_sm(LTE_SLEEP_ENTER);
				}
				else printk("\n-> SYSCTRL: ble connected or gauge low/charging bat, skip NO_FIX_GPS_EVT!");
				break;
			}
			case GEOF_IN_EVT:
			{
				if(ble_cs==BLE_DISC && gauge_cs==GAUGE_NORMALBAT)
                {
					gnss_finite_sm(GNSS_TRACKING_ENTER);
					lte_finite_sm(LTE_SLEEP_ENTER);
				}
				else printk("\n-> SYSCTRL: ble connected or gauge low/charging bat, skip GEOF_IN_EVT!");
				break;
			}
			case GEOF_OUT_EVT:
			{
				if(ble_cs==BLE_DISC && gauge_cs==GAUGE_NORMALBAT)
                {
					gnss_finite_sm(GNSS_TRACKING_ENTER);
					lte_finite_sm(LTE_ACTIVE_ENTER);
				}
				else printk("\n-> SYSCTRL: ble connected or gauge low/charging bat, skip GEOF_OUT_EVT!");
				break;
			}
			case BLE_DATA_RTS_EVT:
			{
				printk("\n-> SYSCTRL: ready send acc & ppg data via ble!");
				k_sem_give(&acc_start_send_sem);
				k_sem_give(&ppg_start_send_sem);
				break;
			}
			case LATLON_BUF_TIMEOUT_EVT:
			{
				if(lte_cs!=LTE_PWROFF) k_event_post(&lte_ctrl_thread_evt, 0x1);
				else
				{
					printk("\n-> SYSCTRL: lte powered-off, skip LATLON_BUF_TIMEOUT_EVT!");
					location_data_pingpong_wr_idx=0;
				}
				break;
			}
			case LTE_SEND_FAILED_EVT:
			{
				if(lte_cs!=LTE_PWROFF)
				{
					printk("\n-> SYSCTRL: lte send failed, ready disconnect server & hard reset!");
					lte_mqtt_server_disc(&lexi_dev);
					lte_hw_reset(&lexi_dev);
					uart_rx_enable(uart1, lexi_rsp_buf[0], LTE_RSP_BUF_SIZE, 2000);
					lte_mqtt_server_conn(&lexi_dev);
					lte_sleep(&lexi_dev);
				}
				else printk("\n-> SYSCTRL: lte powered-off, skip LTE_SEND_FAILED_EVT!");
				break;
			}
			default: break;
		}
	}
}
K_THREAD_DEFINE(system_ctrl_thread_tid, 1024, system_ctrl_thread, NULL, NULL, NULL, 7, 0, 0);

void all_states_read_thread()
{
	uint8_t cycle_id=0;
	uint8_t* dev="NORDIC_DEVICE";
	while(1)
	{
		/* test code */
		// printk("\nstate of device now: ");
		// printk("BLE: %s | IMU: %s | PPG: %s | GAUGE: %s | GNSS: %s | LTE: %s", 
		// 	ble_stt2str[ble_cs], imu_stt2str[imu_cs], ppg_stt2str[ppg_cs], 
		// 		gauge_stt2str[gauge_cs], gnss_stt2str[gnss_cs], lte_stt2str[lte_cs]);
		// k_msleep(5000);
		
		// k_sem_take(&dev_stt_read_sem, K_FOREVER);
		// printk("\nALL_STATES_READ_THREAD: state of device now: ");
		// printk("BLE: %s | IMU: %s | PPG: %s | GAUGE: %s | GNSS: %s | LTE: %s", 
		// 	ble_stt2str[ble_cs], imu_stt2str[imu_cs], ppg_stt2str[ppg_cs], 
		// 		gauge_stt2str[gauge_cs], gnss_stt2str[gnss_cs], lte_stt2str[lte_cs]);

		k_sem_take(&dev_stt_read_sem, K_FOREVER);
		if(ble_connected) 
		{
			ble_data_framing(DEVICE_STATE_EVT, cycle_id, 0, dev);
			cycle_id=(cycle_id+1)%128;
		}
	}
}
K_THREAD_DEFINE(all_states_read_thread_tid, 1024, all_states_read_thread, NULL, NULL, NULL, 8, 0, 0);

// static int wdt_channel_id;
// int setup_hardware_watchdog(const struct device *wdt_dev)
// {
//     if(!device_is_ready(wdt_dev)) return -ENODEV;
//     struct wdt_timeout_cfg wdt_config= 
// 	{
//         .window= 
// 		{
//             .min=0U,                    // don't use window WDT
//             .max=30000,     			// ms
//         },
//         .callback=NULL,                 
//         .flags=WDT_FLAG_RESET_SOC,      // reset
//     };
//     wdt_channel_id=wdt_install_timeout(wdt_dev, &wdt_config);
//     if(wdt_channel_id<0) return wdt_channel_id;
//     int err=wdt_setup(wdt_dev, WDT_OPT_PAUSE_IN_SLEEP);
//     if(err<0) return err;
//     return 0;
// }
// const struct device *wdt_dev;

// void wdt_feed_thread(void *p1, void *p2, void *p3)
// {
//     while(1) 
// 	{
//         int err=wdt_feed(wdt_dev, wdt_channel_id);	// feed dog
//         if(!err) printk("\nWDT_FEED_THREAD: feed dog done!");
//         else printk("\nWDT_FEED_THREAD: feed dog fail! (err = %d)", err);
//         k_msleep(100);
//     }
// }
// K_THREAD_DEFINE(wdt_feed_thread_tid, 1024, wdt_feed_thread, NULL, NULL, NULL, 9, 0, 0);

/*******************************************************************************************************/
/*                                SYSTEM CONTROLLER IMPLEMENTATION END	                               */
/*******************************************************************************************************/
static struct gpio_callback gauge_low_bat_cb_data;
static struct gpio_callback gauge_charging_cb_data;
#define PROGRAM_SETUP_TIME_START 3000000 // microseconds
#define PROGRAM_SETUP_TIME_STOP 3000000  // microseconds
int main(void)
{
	/**********************************************************************/
	/*							   BLE init - start		 		  	      */
	/**********************************************************************/
	int err;
	printk("\nAPP: main entered");
	printk("\nAPP: starting bt_enable");
	err=bt_enable(NULL);
	printk("\nAPP: bt_enable returned %d", err);
	if(err) {
		printk("\nAPP: bt_enable failed");
		return -1;
	}
	bt_conn_cb_register(&connection_callbacks);
	printk("\nAPP: BLE callbacks registered");
	set_tx_power(BT_HCI_VS_LL_HANDLE_TYPE_ADV, 0, 0);
	set_tx_power(BT_HCI_VS_LL_HANDLE_TYPE_CONN, 0, 8);
	update_phy(my_conn);
	err=bt_le_adv_start(adv_param, ad, ARRAY_SIZE(ad), sd, ARRAY_SIZE(sd));
	printk("\nAPP: advertising start returned %d", err);
    if(err) return -1;
	err=bt_lbs_init(&lbs_callbacs);
	printk("\nAPP: LBS init returned %d", err);
	if(err) return -1;
	/**********************************************************************/
	/*							   BLE init - end		 		  	      */
	/**********************************************************************/
	/*
	*/
	/**********************************************************************/
	/*							   WDT init - start		 		  	      */
	/**********************************************************************/
	// wdt_dev=DEVICE_DT_GET(DT_NODELABEL(wdt0));
    // if(setup_hardware_watchdog(wdt_dev)<0) printk("\nMAIN THREAD: setup hardware watchdog fail!");
	/**********************************************************************/
	/*							   WDT init - end		 		  	      */
	/**********************************************************************/
	/*
	*/
	/**********************************************************************/
	/*							  IO init - start		 		  	      */
	/**********************************************************************/
	gpio0=device_get_binding("gpio@50000000");
	gpio1=device_get_binding("gpio@50000300");
	printk("\nAPP: GPIO bindings gpio0=%p gpio1=%p", gpio0, gpio1);
	if (!gpio0 || !gpio1) {
		printk("\nAPP: GPIO binding failed");
		return -1;
	}

	/* config IO pin (power, reset,...) for all modules */
	gpio_pin_configure(gpio0, 24, GPIO_OUTPUT | GPIO_OPEN_DRAIN);	// GNSS
	gpio_pin_configure(gpio0, 13, GPIO_OUTPUT | GPIO_OPEN_DRAIN);	// LTE
	gpio_pin_configure(gpio0, 31, GPIO_OUTPUT | GPIO_OPEN_DRAIN);	// PPG
	gpio_pin_configure(gpio1, 8, GPIO_OUTPUT | GPIO_OPEN_DRAIN);	// IMU
	gpio_pin_configure(gpio0, 21, GPIO_OUTPUT | GPIO_OPEN_DRAIN);	// LTE's PWRKEY_ON
	gpio_pin_configure(gpio0, 20, GPIO_OUTPUT | GPIO_OPEN_DRAIN);	// LTE's RESET

	/* turn on-off for modules */
	gpio_pin_set(gpio0, 24, 1);
	gpio_pin_set(gpio0, 13, 0);	// LTE power on
	gpio_pin_set(gpio0, 31, 1);
	gpio_pin_set(gpio1, 8, 0);	// IMU power on
	gpio_pin_set(gpio0, 21, 1);
	gpio_pin_set(gpio0, 20, 1);
	/**********************************************************************/
	/*							  IO init - end		 		  	          */
	/**********************************************************************/
	/*
	*/
	/**********************************************************************/
	/*						  Callbacks init - start		 			  */
	/**********************************************************************/
	/* bq27220 interrupt init -> to receive LOW_BAT alert */
	gpio_pin_configure(gpio0, 8, GPIO_INPUT | GPIO_OPEN_DRAIN);
	gpio_pin_interrupt_configure(gpio0, 8, GPIO_INT_EDGE_FALLING);
	gpio_init_callback(&gauge_low_bat_cb_data, gauge_low_bat_cb_handler, BIT(8));
	gpio_add_callback(gpio0, &gauge_low_bat_cb_data);

	/* charge IC interrupt init -> to receive CHARGING_BAT alert */
	gpio_pin_configure(gpio1, 15, GPIO_INPUT | GPIO_PULL_UP);
	gpio_pin_interrupt_configure(gpio1, 15, GPIO_INT_EDGE_BOTH);
	gpio_init_callback(&gauge_charging_cb_data, gauge_charging_cb_handler, BIT(15));
	gpio_add_callback(gpio1, &gauge_charging_cb_data);

	/* uart callbacks init */
	uart_callback_set(uart0, uart0_cb_handler, NULL);
	uart_callback_set(uart1, uart1_cb_handler, NULL);
	/**********************************************************************/
	/*						  Callbacks init - end		 			  	  */
	/**********************************************************************/
	/*
	*/
	/**********************************************************************/
	/*						  Program setup - start		 			  	  */
	/**********************************************************************/
	/* setup time, soft power-off LTE module */
	printk("\nMAIN_THREAD: PROGRAM CONFIG START!");
	k_busy_wait(PROGRAM_SETUP_TIME_START);
	lte_pwr(OFF);
	k_busy_wait(PROGRAM_SETUP_TIME_STOP);
	printk("\nMAIN_THREAD: PROGRAM CONFIG DONE!");
	
	/* put start state for BLE & GAUGE */
	evt_q_t evt_q_put;
	evt_q_put.evt_src=DISC_EVT;
	k_msgq_put(&sys_evt_msgq, &evt_q_put, K_NO_WAIT);
	evt_q_put.evt_src=NORMAL_BAT_EVT;
	k_msgq_put(&sys_evt_msgq, &evt_q_put, K_NO_WAIT);
	printk("\nAPP: startup events queued");
	/**********************************************************************/
	/*						  Proram setup - end		 			  	  */
	/**********************************************************************/
}





	// uart1_resume();
	// gnss_pwr(ON);
	// lte_pwr(ON);
	// lte_mqtt_server_conn(&lexi_dev);
    // lte_sleep(&lexi_dev);
	// while(1)
	// {
	// 	k_msleep(30000);
	// 	lte_send_data(&lexi_dev);
	// }



	/**********************************************************************/
	/*					10/5/2026: summary test - start		 		  	  */
	/**********************************************************************/
	/* test code: over-voltage */
	// uint8_t ret=0;
	// uint8_t fifo_count_l_h[2];
	// imu_pwr(ON);
    // imu_init(&icm20948_dev);
    // imu_config(&icm20948_dev);
	// imu_reg_read(&icm20948_dev, PWR_MGMT_1, &ret); printk("\nimu sleep check before off: %d", ret);
    // i2c_burst_read_dt(&icm20948, FIFO_COUNT_H, fifo_count_l_h, 2); 
    // uint32_t len_of_raw=(uint32_t)((fifo_count_l_h[0]<<8) | (fifo_count_l_h[1])); printk("\nimu fifo count before: %d", len_of_raw);
	// ppg_pwr(ON);
	// ppg_init(&as7057_dev);
	// ppg_config(&as7057_dev);
	// ppg_reg_read(&as7057_dev, 0xD0, &ret); printk("\nppg fifo threashold config before: %d", ret);
	// ppg_reg_read(&as7057_dev, 0x11, &ret); printk("\nppg sysclk config before: %d", ret);
	// ppg_reg_read(&as7057_dev, 0xF0, &ret); printk("\nppg seq start before: %d", ret);
	// ppg_reg_read(&as7057_dev, 0x3F, &ret); printk("\nppg irq config before: %d", ret);
	// // k_busy_wait(1000000);
	// gnss_finite_sm(GNSS_ACQUI_ENTER);
	// lte_finite_sm(LTE_SLEEP_MODE_ENTER);
	// imu_reg_read(&icm20948_dev, PWR_MGMT_1, &ret); printk("\nimu sleep check before: %d", ret);
    // i2c_burst_read_dt(&icm20948, FIFO_COUNT_H, fifo_count_l_h, 2); 
    // len_of_raw=(uint32_t)((fifo_count_l_h[0]<<8) | (fifo_count_l_h[1])); printk("\nimu fifo count after: %d", len_of_raw);
	// ppg_reg_read(&as7057_dev, 0xD0, &ret); printk("\nppg fifo threshold config after: %d", ret);
	// ppg_reg_read(&as7057_dev, 0x11, &ret); printk("\nppg sysclk config after: %d", ret);
	// ppg_reg_read(&as7057_dev, 0xF0, &ret); printk("\nppg seq start after: %d", ret);
	// ppg_reg_read(&as7057_dev, 0x3F, &ret); printk("\nppg irq config after: %d", ret);
	// while(1)
	// {
	// 	k_busy_wait(5000000);
	// 	lte_send_data(&lexi_dev);
	// 	// k_busy_wait(1000000);
	// 	uint32_t start_time=k_uptime_get_32();
    // 	while(k_uptime_get_32()-start_time<10000)
	// 	{
	// 		imu_reg_read(&icm20948_dev, PWR_MGMT_1, &ret); printk("\nimu sleep check after: %d", ret);
	// 		i2c_burst_read_dt(&icm20948, FIFO_COUNT_H, fifo_count_l_h, 2); 
	// 		len_of_raw=(uint32_t)((fifo_count_l_h[0]<<8) | (fifo_count_l_h[1])); printk("\nimu fifo count v: %d", len_of_raw);
	// 		ppg_reg_read(&as7057_dev, 0xD0, &ret); printk("\nppg fifo threashold config after: %d", ret);
	// 		ppg_reg_read(&as7057_dev, 0x11, &ret); printk("\nppg sysclk config after: %d", ret);
	// 		ppg_reg_read(&as7057_dev, 0xF0, &ret); printk("\nppg seq start after: %d", ret);
	// 		ppg_reg_read(&as7057_dev, 0x3F, &ret); printk("\nppg irq config after: %d", ret);
	// 	}
	// }
	
	/* test transmit data via lte-mqtt */
	// char bat_state_pkt[]="D1DB9C18074900000032";
	// // char charging_pkt[]="D1DB9C1807491000";
	// // char device_state_pkt[]="D1DB9C1807492000012100";
	// char geof_out_pkt[]="D1DB9C1807493000013F3C8D064F3A6B";
	// // char acc_data_pkt[]="D1DB9C180749";
	// // char ppg_data_pkt[]="D1DB9C180749";
	// lte_pwr(ON);
	// lte_mqtt_server_conn(&lexi_dev);
	// lte_sleep(&lexi_dev);
	// while(1)
	// {
	// 	lexi_mqtt_pub_test(&lexi_dev, bat_state_pkt);
	// 	k_msleep(1000);
	// 	// lexi_mqtt_pub_test(&lexi_dev, charging_pkt);
	// 	// k_msleep(1000);
	// 	// lexi_mqtt_pub_test(&lexi_dev, device_state_pkt);
	// 	// k_msleep(1000);
	// 	lexi_mqtt_pub_test(&lexi_dev, geof_out_pkt);
	// 	k_msleep(1000);
	// 	// lexi_mqtt_pub_test(&lexi_dev, acc_data_pkt);
	// 	// k_msleep(1000);
	// 	// lexi_mqtt_pub_test(&lexi_dev, ppg_data_pkt);
	// 	// k_msleep(1000);
	// }
	
	/* test code: states input */
	// evt_q_t evt_q_put;
    // imu_evt_t imu_evt_put=IMU_EVT_UPDATE(false, true, false);
    // evt_q_put.evt_src=IMU_EVT; evt_q_put.evt_data.imu_evt=imu_evt_put;
    // k_msgq_put(&sys_evt_msgq, &evt_q_put, K_NO_WAIT);
    // ppg_evt_t ppg_evt_put=PPG_EVT_UPDATE(false, false, true);
    // evt_q_put.evt_src=PPG_EVT; evt_q_put.evt_data.ppg_evt=ppg_evt_put;
    // k_msgq_put(&sys_evt_msgq, &evt_q_put, K_NO_WAIT);

	/* suspend uart to check current consumption */
	// uart_rx_enable(uart0, nmea_buf, sizeof(nmea_buf), SYS_FOREVER_US);
	// k_msleep(1000);
	// uart0_suspend();
	// uart_rx_disable(uart0);
	
	/* send data through BLE */
	// uint8_t test_data1=0x03;
	// uint16_t test_data2=0x8386;
	// uint32_t test_data3=0x45678910;
	// int32_t test_data4=0x87654321;
	// int32_t test_data5[5]={10,9,8,7,6};
	// while(1)	
	// {
	// 	ble_send_single_value(&test_data1, INT8);
	// 	k_msleep(1000);
	// 	ble_send_single_value(&test_data2, INT16);
	// 	k_msleep(1000);
	// 	ble_send_single_value(&test_data3, INT32);
	// 	k_msleep(1000);
	// 	ble_send_single_value(&test_data4, INT32);
	// 	k_msleep(1000);
	// 	ble_send_packet_data(test_data5, INT32, 5);
	// 	k_msleep(1000);
	// }
	
	/* get BLE connected rssi */
	// int8_t rssi_get=0xFF;
	// uint16_t handle;
	// uint8_t ret=bt_hci_get_conn_handle(my_conn, &handle);
	// while(1)
	// {
	// 	read_conn_rssi(handle, &rssi_get);
	// 	printk("Connected, RSSI = %d\n", rssi_get);
	// 	k_msleep(100);
	// }
	
	/* Jlink log test */
	// while(1)
	// {
	 	//LOG_INF("START PROGRAM...\n");
		//k_msleep(1000);
	// }
	
	/* lexi connect server, sleep and send data */
	// k_msleep(1000);
	// lte_pwr(ON);
	// uart_rx_enable(uart1, lexi_uart_buffer[0], LTE_BUFFER_SIZE, 2000);
	// k_msleep(3000);

	// int ret=lexi_at_cmd_send(lexi_data.cmd.com_check, lexi_data.rsp.com_check, COM_CHECK_MS); printk("ret at: %d\n", ret);
	// if(ret) 
	// {
	// 	ret=lexi_at_cmd_send(lexi_data.cmd.sim_check, lexi_data.rsp.sim_check, SIM_CHECK_MS); printk("ret cpin: %d\n", ret); 
	// }
	// if(ret) 
	// {
	// 	ret=lexi_at_cmd_send(lexi_data.cmd.mqtt_server_open, lexi_data.rsp.mqtt_server_open, MQTT_SERVER_OPEN_MS); printk("ret server open: %d\n", ret); 
	// }
	// k_msleep(2000);
	// if(ret)
	// {
	// 	ret=lexi_at_cmd_send(lexi_data.cmd.mqtt_server_conn, lexi_data.rsp.mqtt_server_conn, MQTT_SERVER_CONN_MS); printk("ret server conn: %d\n", ret); 
	// }
	// ret=lexi_at_cmd_send(lexi_data.cmd.hibernate_mode_enter, lexi_data.rsp.hibernate_mode_enter, HIBERNATE_MODE_MS); printk("ret sleep: %d\n", ret);

	// int ret=lte_com_check(&lexi_dev);
	// if(ret) lte_sim_check(&lexi_dev);

	// ret=lte_mqtt_server_conn(&lexi_dev);
	// if(ret) lte_sleep(&lexi_dev);

	// while(1)
	// {
	// 	// ret=lexi_mqtt_pub(&lexi_dev);
	// 	lte_send_data(&lexi_dev);
	// 	// printk("ret pub: %d\n", ret);
	// 	k_msleep(5000);
	// }
	/**********************************************************************/
	/*					10/5/2026: summary test - end		 		  	  */
	/**********************************************************************/
	/*
	*/
	/**********************************************************************/
	/*					19/3/2026: test as7057 in evk - start		  	  */
	/**********************************************************************/
	// imu_init(&icm20948_dev);
	// k_msleep(1000);
	// imu_config(&icm20948_dev);
	// while(1)
	// {
	// 	ppg_init(&as7057_dev);
	// 	k_msleep(1000);
	// 	ppg_config(&as7057_dev);
	// }

	// imu_finite_sm(IMU_NO_MOVE_ENTER);
	// ppg_finite_sm(PPG_MEASURE_ENTER);

	// evt_q_t evt_q_put;
	// imu_evt_t imu_evt_put=IMU_EVT_UPDATE(false, true, false);
	// evt_q_put.evt_src=IMU_EVT; evt_q_put.evt_data.imu_evt=imu_evt_put;
	// k_msgq_put(&sys_evt_msgq, &evt_q_put, K_NO_WAIT);
	// ppg_evt_t ppg_evt_put=PPG_EVT_UPDATE(false, false, true);
	// evt_q_put.evt_src=PPG_EVT; evt_q_put.evt_data.ppg_evt=ppg_evt_put;
	// k_msgq_put(&sys_evt_msgq, &evt_q_put, K_NO_WAIT);
	
	// while(ppg_init(&as7057_dev)!=1)
	// {
	// 	k_msleep(1000);
	// }
	// k_msleep(2000);
	// if(ppg_init(&as7057_dev)) 
    // {
    	// printk("as7057 successfully init!\n");
        // while(1) 
		// {
		// 	ppg_config(&as7057_dev);
		// 	k_msleep(1000);
		// }
		// {
		// 	k_msleep(1000);
		// } 
        // printk("as7057 successfully config!\n");
        // k_sem_give(&ppg_rx_start_sem);
        // }
        // else printk("as7057 fail config!\n");
    // }
    // else printk("as7057 fail init!\n");
	// if(ret) 
	// {
	// 	as7057_config_2();
	// 	k_sem_give(&as7057_rx_start_sem);
	// }
	// ppg_evt.measuring=true;
	// k_sem_give(&ppg_state_update_sem);
	// imu_evt.no_wom=true;
	// k_sem_give(&imu_state_update_sem);
	/**********************************************************************/
	/*				    19/3/2026: test as7057 in evk - end				  */
	/**********************************************************************/
	/*
	*/
	/**********************************************************************/
	/*					25/3/2026: test gauge in evk - start			  */
	/**********************************************************************/
	// uint16_t ret;
    // if(gauge_soc_read(&bq27220_dev, &ret))
    // {
    //     printk("bq27220 successfully init!\n");
    //     if(gauge_irq_config(&bq27220_dev))
    //     {
    //         printk("bq27220 irq config successfully!\n");
    //     }
    //     else printk("bq27220 irq config fail!\n");
    // }
    // else printk("bq27220 fail init\n");
	// while(1)
	// {
	// 	k_msleep(2000);
	// 	if(gauge_soc_read(&bq27220_dev, &ret))
    // 	{
    //     	printk("bq27220 successfully init!\n");
	// 	}
	// 	else printk("bq27220 fail init!\n");

	// 	bq27220_status_ret stt_ret[2]={0, 0};
	// 	gauge_status_check(&bq27220_dev, stt_ret);
	// 	printk("low(0: low bat, 1: normal bat)?: %d, charge(2: charging, 3: discharging)?: %d\n", stt_ret[0], stt_ret[1]);
	// }
	/**********************************************************************/
	/*				    25/3/2026: test gauge in evk - end				  */
	/**********************************************************************/
// }