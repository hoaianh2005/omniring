#include "bq27220.h"
/*
*
*
*
*
*/
/*******************************************************************************************************/
/*                                          BQ27220 DRIVER START                                       */
/*******************************************************************************************************/
int bq_soc_read(const struct device_t* dev, uint8_t* ret)
{
    const struct i2c_dt_spec* i2c_dev=(const struct i2c_dt_spec*)dev->data;
    if(i2c_burst_read_dt(i2c_dev, RawVoltage, ret, 2)!=0) return 0;
    return 1;
}
int bq_temp_read(const struct device_t* dev, uint8_t* ret)
{
    const struct i2c_dt_spec* i2c_dev=(const struct i2c_dt_spec*)dev->data;
    if(i2c_burst_read_dt(i2c_dev, InternalTemperature, ret, 2)!=0) return 0;
    return 1;
}
int bq_status_check(const struct device_t* dev, bq27220_status_ret* stt_ret) 
{
    const struct i2c_dt_spec* i2c_dev=(const struct i2c_dt_spec*)dev->data;
    uint8_t ret[2];
    if(i2c_burst_read_dt(i2c_dev, BatteryStatus, ret, 2)!=0) return 0;
    uint8_t low_byte=ret[0];
    if (low_byte&TDA_BIT) (*stt_ret)=BQ_LOW_BAT;
    else (*stt_ret)=BQ_NORMAL_BAT;
    return 1;
}
static int wait_cfg_update(const struct i2c_dt_spec* i2c_dev, uint8_t target_bit_val) 
{
    uint8_t status[2];
    int timeout=100;
    while(timeout--) 
    {
        i2c_burst_read_dt(i2c_dev, OperationStatus, status, 2);
        if(((status[1]>>2)&0x01)==target_bit_val) return 1;
        k_msleep(20);
    }
    return 0;
}
int bq_irq_config(const struct device_t* dev)
{
    const struct i2c_dt_spec* i2c_dev=(const struct i2c_dt_spec*)dev->data;
    uint8_t mac_data[2];
    uint8_t tmp_addr[2];
    uint16_t sum;
    uint8_t enter_mode[]={0x90, 0x00};
    if(i2c_burst_write_dt(i2c_dev, UpdateConfig_Port, enter_mode, 2)!=0) return 0;
    if(!wait_cfg_update(i2c_dev, 1)) return 0;
    tmp_addr[0]=0x06; tmp_addr[1]=0x92;
    if(i2c_burst_write_dt(i2c_dev, UpdateConfig_Port, tmp_addr, 2)!=0) return 0;
    if(i2c_burst_read_dt(i2c_dev, MACData, mac_data, 2)!=0) return 0;
    mac_data[0]&=~(1 << 7);
    if(i2c_burst_write_dt(i2c_dev, MACData, mac_data, 2)!=0) return 0;
    sum=tmp_addr[0]+tmp_addr[1]+mac_data[0]+mac_data[1];
    uint8_t sum_len_a[]={255-(uint8_t)(sum%256), 4};
    if(i2c_burst_write_dt(i2c_dev, MACDataSum, sum_len_a, 2)!=0) return 0;
    tmp_addr[0]=0x08; tmp_addr[1]=0x92;
    if(i2c_burst_write_dt(i2c_dev, UpdateConfig_Port, tmp_addr, 2)!=0) return 0;
    if(i2c_burst_read_dt(i2c_dev, MACData, mac_data, 2)!=0) return 0;
    mac_data[0]=(mac_data[0]|(1<<6))&~((1<<5)|(1<<1)); 
    if(i2c_burst_write_dt(i2c_dev, MACData, mac_data, 2)!=0) return 0;
    sum=tmp_addr[0]+tmp_addr[1]+mac_data[0]+mac_data[1];
    uint8_t sum_len_b[]={255-(uint8_t)(sum%256), 4};
    if(i2c_burst_write_dt(i2c_dev, MACDataSum, sum_len_b, 2)!=0) return 0;
    tmp_addr[0]=0x7F; tmp_addr[1]=0x92;
    if(i2c_burst_write_dt(i2c_dev, UpdateConfig_Port, tmp_addr, 2)!=0) return 0;
    if(i2c_burst_read_dt(i2c_dev, MACData, mac_data, 2)!=0) return 0;
    mac_data[0]|=(1<<0);
    if(i2c_burst_write_dt(i2c_dev, MACData, mac_data, 2)!=0) return 0;
    sum=tmp_addr[0]+tmp_addr[1]+mac_data[0]+mac_data[1];
    uint8_t sum_len_c[]={255-(uint8_t)(sum%256), 4};
    if(i2c_burst_write_dt(i2c_dev, MACDataSum, sum_len_c, 2)!=0) return 0;
    tmp_addr[0]=0x8E; tmp_addr[1]=0x92;
    if(i2c_burst_write_dt(i2c_dev, UpdateConfig_Port, tmp_addr, 2)!=0) return 0;
    uint8_t v_data[]={(LOW_BAT_THRESHOLD&0xFF), (LOW_BAT_THRESHOLD>>8)};
    if(i2c_burst_write_dt(i2c_dev, MACData, v_data, 2)!=0) return 0;
    sum=tmp_addr[0]+tmp_addr[1]+v_data[0]+v_data[1];
    uint8_t sum_len_d[]={255-(uint8_t)(sum%256), 4};
    if(i2c_burst_write_dt(i2c_dev, MACDataSum, sum_len_d, 2)!=0) return 0;
    uint8_t exit_mode[]={0x92, 0x00};
    if(i2c_burst_write_dt(i2c_dev, UpdateConfig_Port, exit_mode, 2)!=0) return 0;
    if(!wait_cfg_update(i2c_dev, 0)) return 0;
    uint8_t bat_ins[]={0x0D, 0x00};
    if(i2c_burst_write_dt(i2c_dev, UpdateConfig_Port, bat_ins, 2)!=0) return 0;
    return 1;
}
/*******************************************************************************************************/
/*                                          BQ27220 DRIVER END                                         */
/*******************************************************************************************************/