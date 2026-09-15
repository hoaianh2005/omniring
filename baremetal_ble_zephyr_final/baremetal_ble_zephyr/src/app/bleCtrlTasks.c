#include "bleCtrlTasks.h"
#include <zephyr/bluetooth/hci.h>
#include <zephyr/bluetooth/hci_vs.h>
#include <zephyr/bluetooth/bluetooth.h>
/*
*
*
*
*
*/
/*******************************************************************************************************/
/*                                  	BLE FSM IMPLEMENTATION START                                   */
/*******************************************************************************************************/
ble_state_t ble_fsm[2]=
{
	/* state 0: CONN */
	{
		.ns={BLE_CONN, BLE_DISC}
	},
	/* state 1: DISC */
	{
		.ns={BLE_CONN, BLE_DISC}
	}
};
ble_state ble_cs=BLE_DISC;
uint8_t* ble_stt2str[2]={"CONNECTED", "DISCONNECTED"};
void ble_finite_sm(ble_state_input input)
{
	ble_cs=ble_fsm[ble_cs].ns[input];
	printk("\n----> BLE: %s", ble_stt2str[ble_cs]);
}
/*******************************************************************************************************/
/*                                  	BLE FSM IMPLEMENTATION END	                                   */
/*******************************************************************************************************/
/*
*/
/*******************************************************************************************************/
/*                                  	BLE SEND DATA FUNCTIONS START                                  */
/*******************************************************************************************************/
// uint8_t dev_mac_addr[]={0xC7, 0x30, 0x24, 0x02, 0x88, 0x33}; // lg76g-pb
uint8_t dev_mac_addr[]={0xCC, 0x7C, 0xAE, 0xE3, 0xDC, 0x54}; // sim65m-w
void ble_data_framing(uint8_t evt_type, uint8_t cycle_id, uint8_t pkt_idx, void* data)
{
	switch(evt_type)
	{
		case BAT_STATE_EVT:
		{
			uint8_t bat_state_frame[12];
			uint16_t* bat_stt_data=(uint16_t*)data;
			uint8_t bat_stt_extr[4]; uint16_t idx=0;
			for(int i=0; i<2; i++)
			{
				bat_stt_extr[idx++]=(bat_stt_data[i]>>8)&0xFF;
				bat_stt_extr[idx++]=bat_stt_data[i]&0xFF;
			}
			memcpy(bat_state_frame, dev_mac_addr, sizeof(dev_mac_addr));
			uint8_t header[2];
			header[0]=((0x0F&evt_type)<<4)|((cycle_id>>3)&0x0F);
			header[1]=((cycle_id&0x07)<<5)|(0x00&0x1F);
			memcpy(bat_state_frame+6, header, sizeof(header));
			memcpy(bat_state_frame+8, bat_stt_extr, sizeof(bat_stt_extr));
			ble_send_packet(bat_state_frame, INT8, 12);
			break;
		}
        case CHARGE_EVT:
		{		
			uint8_t charge_frame[9];
			uint8_t* chrg_stt=(uint8_t*)data;
            memcpy(charge_frame, dev_mac_addr, sizeof(dev_mac_addr));
            uint8_t header[2];
            header[0]=((0x0F&evt_type)<<4)|((cycle_id>>3)&0x0F);
            header[1]=((cycle_id&0x07)<<5)|(0x00&0x1F);
            memcpy(charge_frame+6, &header, sizeof(header));
			memcpy(charge_frame+8, chrg_stt, 1);
            ble_send_packet(charge_frame, INT8, 9);
            break;
		}
		case DEVICE_STATE_EVT:
		{
			uint8_t dev_stt_frame[11];
			memcpy(dev_stt_frame, dev_mac_addr, sizeof(dev_mac_addr));
			uint8_t header[2];
			header[0]=((0x0F&evt_type)<<4)|((cycle_id>>3)&0x0F);
			header[1]=((cycle_id&0x07)<<5)|(pkt_idx&0x1F);
			memcpy(dev_stt_frame+6, header, sizeof(header));
			uint8_t modules_stt[3];
			modules_stt[0]=((0x0F&ble_cs)<<4)|(imu_cs&0x0F);
			modules_stt[1]=((0x0F&ppg_cs)<<4)|(gauge_cs&0x0F);
			modules_stt[2]=((0x0F&gnss_cs)<<4)|(lte_cs&0x0F);
			memcpy(dev_stt_frame+8, modules_stt, sizeof(modules_stt));
			ble_send_packet(dev_stt_frame, INT8, 11);
			break;
		}
		case ACC_DATA_EVT:
		{
			static uint8_t imu_data_frame[506];
			imu_data_t* imu_data=(imu_data_t*)data;
			static uint8_t imu_data_extr[498]; uint16_t idx=0;
			for(int i=0; i<83; i++)
			{
				imu_data_extr[idx++]=(imu_data[i].x>>8)&0xFF; imu_data_extr[idx++]=imu_data[i].x&0xFF;
				imu_data_extr[idx++]=(imu_data[i].y>>8)&0xFF; imu_data_extr[idx++]=imu_data[i].y&0xFF;
				imu_data_extr[idx++]=(imu_data[i].z>>8)&0xFF; imu_data_extr[idx++]=imu_data[i].z&0xFF;
			}
			memcpy(imu_data_frame, dev_mac_addr, sizeof(dev_mac_addr));
			uint8_t header[2];
			header[0]=((0x0F&evt_type)<<4)|((cycle_id>>3)&0x0F);
			header[1]=((cycle_id&0x07)<<5)|(pkt_idx&0x1F);
			memcpy(imu_data_frame+6, header, sizeof(header));
			memcpy(imu_data_frame+8, imu_data_extr, sizeof(imu_data_extr));
			ble_send_packet(imu_data_frame, INT8, 506);
			break;
		}
		case PPG_DATA_EVT:
		{
			static uint8_t ppg_data_frame[508];
			int32_t* ppg_data=(int32_t*)data;
			static uint8_t ppg_data_extr[500]; uint16_t idx=0;
			for(int i=0; i<125; i++)
			{
				ppg_data_extr[idx++]=(ppg_data[i]>>24)&0xFF;
				ppg_data_extr[idx++]=(ppg_data[i]>>16)&0xFF;
				ppg_data_extr[idx++]=(ppg_data[i]>>8)&0xFF;
				ppg_data_extr[idx++]=ppg_data[i]&0xFF;
			}
			memcpy(ppg_data_frame, dev_mac_addr, sizeof(dev_mac_addr));
			uint8_t header[2];
			header[0]=((0x0F&evt_type)<<4)|((cycle_id>>3)&0x0F);
			header[1]=((cycle_id&0x07)<<5)|(pkt_idx&0x1F);
			memcpy(ppg_data_frame+6, header, sizeof(header));
			memcpy(ppg_data_frame+8, ppg_data_extr, sizeof(ppg_data_extr));
			ble_send_packet(ppg_data_frame, INT8, 508);
			break;
		}
		default: break;
	}
}

K_SEM_DEFINE(ppg_start_send_sem,0,1);
void ppg_send_data_thread()
{
	static int32_t ppg_data_cpy[2048];
	static int32_t ppg_data_to_send[128];
	uint8_t cycle_id=0;
    while(1)
    {
        k_sem_take(&ppg_start_send_sem, K_FOREVER);
		printk("\nPPG_SEND_DATA_THREAD: start send!");
		memcpy(ppg_data_cpy, ppg_pingpong_rd_ptr, 1500*sizeof(int32_t));

		int rd_idx=0;
		for(int i=0; i<12; i++)
		{
			for(int j=0; j<125; j++)
			{
				ppg_data_to_send[j]=ppg_data_cpy[rd_idx++];
			}
			if(ble_connected) 
			{
				ble_data_framing(PPG_DATA_EVT, cycle_id, i, ppg_data_to_send);
				printk("\nPPG_SEND_DATA_THREAD: cycle %d, packet %d send done!", cycle_id+1, i+1);
				k_msleep(500);
			}
			else
			{
				printk("\nPPG_SEND_DATA_THREAD: ble disconnected, stop send!");
				break;
			}
		}
		if(ble_connected)
		{
			printk("\nPPG_SEND_DATA_THREAD: cycle %d send done!", cycle_id+1);
			cycle_id=(cycle_id+1)%128;
		}
		else cycle_id=0;
	}
}
K_THREAD_DEFINE(ppg_send_data_thread_tid, 1024, ppg_send_data_thread, NULL, NULL, NULL, 7, 0, 0);

K_SEM_DEFINE(acc_start_send_sem,0,1);
void acc_send_data_thread()
{
	static imu_data_t acc_data_cpy[2048];
	static imu_data_t acc_data_to_send[128];
	uint8_t cycle_id=0;
    while(1)
    {
        k_sem_take(&acc_start_send_sem, K_FOREVER);
		printk("\nACC_SEND_DATA_THREAD: start send!");
		memcpy(acc_data_cpy, acc_pingpong_rd_ptr, 1500*sizeof(imu_data_t));

		int rd_idx=0;
		for(int i=0; i<19; i++)
		{
			for(int j=0; j<83; j++)
			{
				acc_data_to_send[j]=acc_data_cpy[rd_idx++];
			}
			if(ble_connected) 
			{
				ble_data_framing(ACC_DATA_EVT, cycle_id, i, acc_data_to_send);
				printk("\nACC_SEND_DATA_THREAD: cycle %d, packet %d send done!", cycle_id+1, i+1);
				k_msleep(500);
			}
			else
			{
				printk("\nACC_SEND_DATA_THREAD: ble disconnected, stop send!");
				break;
			}	
		}
		if(ble_connected) 
		{
			printk("\nACC_SEND_DATA_THREAD: cycle %d send done!", cycle_id+1);
			cycle_id=(cycle_id+1)%128;
		}
		else cycle_id=0;
    }
}
K_THREAD_DEFINE(acc_send_data_thread_tid, 1024, acc_send_data_thread, NULL, NULL, NULL, 7, 0, 0);

void ble_ctrl_thread()
{
	evt_q_t evt_q_put;
	while(1)
	{
		/* Chỉ chờ IMU (0x1), tạm thời bỏ qua PPG (0x2) để test IMU */
		k_event_wait_all(&data_is_ready_evt, 0x1, false, K_FOREVER);
		k_event_clear(&data_is_ready_evt, 0x1);
		printk("\nBLE_SEND_CTRL_THREAD: events get, clear done!");
		evt_q_put.evt_src=BLE_DATA_RTS_EVT;
		k_msgq_put(&sys_evt_msgq, &evt_q_put, K_NO_WAIT);
	}
}
K_THREAD_DEFINE(ble_ctrl_thread_tid, 1024, ble_ctrl_thread, NULL, NULL, NULL, 7, 0, 0);
/*******************************************************************************************************/
/*                                  	BLE SEND DATA FUNCTIONS END                                    */
/*******************************************************************************************************/
/*
*/
/*******************************************************************************************************/
/*                                  	BLE PARAMS & CB DEFINE START                             	   */
/*******************************************************************************************************/
BT_GATT_SERVICE_DEFINE(
	my_lbs_svc, BT_GATT_PRIMARY_SERVICE(BT_UUID_LBS),
	BT_GATT_CHARACTERISTIC(BT_UUID_LBS_MYSENSOR, BT_GATT_CHRC_NOTIFY, BT_GATT_PERM_NONE, NULL,
			       NULL, NULL),
	BT_GATT_CCC(mylbsbc_ccc_mysensor_cfg_changed, BT_GATT_PERM_READ | BT_GATT_PERM_WRITE),
    BT_GATT_CHARACTERISTIC(BT_UUID_LBS_MYLNG, BT_GATT_CHRC_NOTIFY, BT_GATT_PERM_NONE, NULL,
        NULL, NULL),
    BT_GATT_CCC(mylbsbc_ccc_mylng_cfg_changed, BT_GATT_PERM_READ | BT_GATT_PERM_WRITE),
);
static bool notify_mysensor_enabled;
static bool notify_mylng_enabled;

int ble_send_imu_sample(const imu_data_t *sample)
{
	if (!sample || !ble_connected || !notify_mysensor_enabled) {
		return -EACCES;
	}

	uint8_t payload[6];
	payload[0] = (uint8_t)(sample->x & 0xFF);
	payload[1] = (uint8_t)((sample->x >> 8) & 0xFF);
	payload[2] = (uint8_t)(sample->y & 0xFF);
	payload[3] = (uint8_t)((sample->y >> 8) & 0xFF);
	payload[4] = (uint8_t)(sample->z & 0xFF);
	payload[5] = (uint8_t)((sample->z >> 8) & 0xFF);

	return bt_gatt_notify(NULL, &my_lbs_svc.attrs[2], payload, sizeof(payload));
}

void mylbsbc_ccc_mysensor_cfg_changed(const struct bt_gatt_attr *attr, uint16_t value)
{
	notify_mysensor_enabled = (value == BT_GATT_CCC_NOTIFY);
	printk("\nBLE IMU notify enabled=%d\n", notify_mysensor_enabled);
}
void mylbsbc_ccc_mylng_cfg_changed(const struct bt_gatt_attr *attr, uint16_t value)
{
	notify_mylng_enabled = (value == BT_GATT_CCC_NOTIFY);
}
void update_phy(struct bt_conn *conn)
{
	int err;
	const struct bt_conn_le_phy_param preferred_phy = {
		.options = BT_CONN_LE_PHY_OPT_NONE,
		.pref_rx_phy = BT_GAP_LE_PHY_2M,
		.pref_tx_phy = BT_GAP_LE_PHY_2M,
	};
	err = bt_conn_le_phy_update(conn, &preferred_phy);
}
void set_tx_power(uint8_t handle_type, uint16_t handle, int8_t tx_pwr_lvl)
{
	/* 
	 * Zephyr 3.4.0 đã thay đổi/ẩn đi các API can thiệp HCI (như bt_hci_cmd_create). 
	 * Tạm thời vô hiệu hóa hàm set_tx_power (mặc định nRF52840 phát sóng ở mức 0 dBm là rất ổn định).
	 */
}
int ble_send_packet(void* sensor_value, uint16_t size, uint16_t length)
{
	return bt_gatt_notify(NULL, &my_lbs_svc.attrs[2], sensor_value, size*length);
}
int ble_send_single_value(void* sensor_value, uint16_t size)
{
	// if (!notify_mysensor_enabled) 
	// {
	// 	return -EACCES;
	// }
	return bt_gatt_notify(NULL, &my_lbs_svc.attrs[2], sensor_value, size);
}
void exchange_func(struct bt_conn *conn, uint8_t err, struct bt_gatt_exchange_params *params) 
{
    if (!err) printk("\nSuccessfully handshake!, MTU now is: %d", bt_gatt_get_mtu(conn));
    else printk("\nFail handshake! (err %d)", err);
}
struct bt_gatt_exchange_params exchange_params=
{
	.func=exchange_func,
};

bool ble_connected=false;
void on_connected(struct bt_conn *conn, uint8_t err)
{
	if(err) 
	{
		evt_q_t evt_q_put;
		evt_q_put.evt_src=CONN_EVT;
		k_msgq_put(&sys_evt_msgq, &evt_q_put, K_NO_WAIT);
		return;
	}
	my_conn=bt_conn_ref(conn);
	bt_gatt_exchange_mtu(my_conn, &exchange_params);

	ble_connected=true;

	evt_q_t evt_q_put;
	evt_q_put.evt_src=CONN_EVT;
	k_msgq_put(&sys_evt_msgq, &evt_q_put, K_NO_WAIT);
}
void on_disconnected(struct bt_conn *conn, uint8_t reason)
{
	bt_conn_unref(my_conn);

	ble_connected=false;

	evt_q_t evt_q_put;
	evt_q_put.evt_src=DISC_EVT;
	k_msgq_put(&sys_evt_msgq, &evt_q_put, K_NO_WAIT);
}
struct bt_conn_cb connection_callbacks = {
	.connected          = on_connected,
	.disconnected       = on_disconnected,
};

int gauge_stt_cnt=0;
int gauge_read_cnt=0;
K_SEM_DEFINE(dev_stt_read_sem,0,1);
void app_led_cb(const bool val_bool)
{
	uint8_t val = (uint8_t)val_bool;
	/* uint8_t val -> needing to confirm with App's developer */
	switch(val)
	{
		case 0x00:	// OFF button: Gauge low bat simulation
		{
			// if(gauge_stt_cnt%2==0)
			// {
			// 	evt_q_t evt_q_put;
			// 	evt_q_put.evt_src=LOW_BAT_EVT;
			// 	k_msgq_put(&sys_evt_msgq, &evt_q_put, K_NO_WAIT);
			// }
			// else
			// {	
			// 	evt_q_t evt_q_put;
			// 	evt_q_put.evt_src=NORMAL_BAT_EVT;
			// 	k_msgq_put(&sys_evt_msgq, &evt_q_put, K_NO_WAIT);
			// }
			// gauge_stt_cnt+=1;

			k_sem_give(&pin_soc_read_sem);
			break;
		}
		case 0x01:	// ON button: Gauge read soc
		{
			// if(gauge_read_cnt%2==0)
			// {
			// 	k_sem_give(&pin_soc_read_sem);
			// }
			// else
			// {
			// 	k_sem_give(&dev_stt_read_sem);
			// }
			// gauge_read_cnt+=1;

			k_sem_give(&dev_stt_read_sem);
			break;
		}
		case 0x02:
		{
			sys_reboot(0);
			break;
		}
		default: break;
	}
}
struct bt_lbs_cb lbs_callbacs = 
{
	.led_cb = app_led_cb,
};

struct bt_conn *my_conn = NULL;
const struct bt_le_adv_param *adv_param = BT_LE_ADV_PARAM(
	( BT_LE_ADV_OPT_CONN |
	 BT_LE_ADV_OPT_USE_IDENTITY), // connectable advertising and use identity address 
	800, // 0x30 units, 48 units, 30ms 
	801, // 0x60 units, 96 units, 60ms 
	NULL); // set to NULL for undirected advertising 
const struct bt_data ad[] = {
	BT_DATA_BYTES(BT_DATA_FLAGS, (BT_LE_AD_GENERAL | BT_LE_AD_NO_BREDR)),
	BT_DATA(BT_DATA_NAME_COMPLETE, DEVICE_NAME, DEVICE_NAME_LEN),
};
const struct bt_data sd[] = {
	BT_DATA_BYTES(BT_DATA_UUID128_ALL, BT_UUID_LBS_VAL),
};
/*******************************************************************************************************/
/*                                  	BLE PARAMS & CB DEFINE END	                             	   */
/*******************************************************************************************************/