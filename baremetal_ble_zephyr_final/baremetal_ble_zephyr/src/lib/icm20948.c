#include "icm20948.h"
/*
*
*
*
*
*/
/*******************************************************************************************************/
/*                                        ICM-20948 DRIVER START                                       */
/*******************************************************************************************************/
int icm20948_init(const struct device_t* dev)
{
    const struct i2c_dt_spec* i2c_dev=(const struct i2c_dt_spec*)dev->data;
    uint8_t tx_buf[]={BANK_SEL_REG, BANK0};
    if(i2c_write_dt(i2c_dev, tx_buf, 2)<0) return 0;
    uint8_t addr=WHO_AM_I; uint8_t ret;
    if(i2c_write_read_dt(i2c_dev, &addr, 1, &ret, 1)<0) return 0;
    if(ret==0xEA) return 1;
    return 0;
}
int icm20948_config(const struct device_t* dev)
{
    const struct i2c_dt_spec* i2c_dev = (const struct i2c_dt_spec*)dev->data;
    uint8_t tx_buf[2];
    uint8_t addr, ret;
    tx_buf[0]=BANK_SEL_REG; tx_buf[1]=BANK0;
    if(i2c_write_dt(i2c_dev, tx_buf, 2)<0) return 0;
    tx_buf[0]=PWR_MGMT_1; tx_buf[1]=0x80;
    if(i2c_write_dt(i2c_dev, tx_buf, 2)<0) return 0;
    k_msleep(100);
    tx_buf[0]=PWR_MGMT_1; tx_buf[1]=0x09; 
    if(i2c_write_dt(i2c_dev, tx_buf, 2)<0) return 0;
    tx_buf[0]=PWR_MGMT_2; tx_buf[1]=0x07; 
    if(i2c_write_dt(i2c_dev, tx_buf, 2)<0) return 0;
    tx_buf[0]=FIFO_EN_2; tx_buf[1]=0x10; 
    if(i2c_write_dt(i2c_dev, tx_buf, 2)<0) return 0;
    tx_buf[0]=USER_CTRL; tx_buf[1]=0x40; 
    if(i2c_write_dt(i2c_dev, tx_buf, 2)<0) return 0;
    tx_buf[0]=LP_CONFIG; tx_buf[1]=0x20; 
    if(i2c_write_dt(i2c_dev, tx_buf, 2)<0) return 0;
    tx_buf[0]=INT_PIN_CFG; tx_buf[1]=0x00; 
    if(i2c_write_dt(i2c_dev, tx_buf, 2)<0) return 0;
    tx_buf[0]=BANK_SEL_REG; tx_buf[1]=BANK2;
    if(i2c_write_dt(i2c_dev, tx_buf, 2)<0) return 0;
    tx_buf[0]=ACCEL_CONFIG; tx_buf[1]=0x3D; 
    if(i2c_write_dt(i2c_dev, tx_buf, 2)<0) return 0;
    tx_buf[0]=ACCEL_CONFIG_2; tx_buf[1]=0x02; 
    if(i2c_write_dt(i2c_dev, tx_buf, 2)<0) return 0;
    tx_buf[0]=ACCEL_SMPLRT_DIV_1; tx_buf[1]=0x00;
    if(i2c_write_dt(i2c_dev, tx_buf, 2)<0) return 0;
    tx_buf[0]=ACCEL_SMPLRT_DIV_2; tx_buf[1]=0x16;
    if(i2c_write_dt(i2c_dev, tx_buf, 2)<0) return 0;
    tx_buf[0]=ACCEL_WOM_THR; tx_buf[1]=0x1B; 
    if(i2c_write_dt(i2c_dev, tx_buf, 2)<0) return 0;
    tx_buf[0]=ACCEL_INTEL_CTRL; tx_buf[1]=0x03; 
    if(i2c_write_dt(i2c_dev, tx_buf, 2)<0) return 0;
    tx_buf[0]=BANK_SEL_REG; tx_buf[1]=BANK0;
    if(i2c_write_dt(i2c_dev, tx_buf, 2)<0) return 0;
    k_msleep(30);
    addr=INT_STATUS;
    if(i2c_write_read_dt(i2c_dev, &addr, 1, &ret, 1)<0) return 0;
    return 1;
}
int icm20948_sleep(const struct device_t* dev)
{
    const struct i2c_dt_spec* i2c_dev = (const struct i2c_dt_spec*)dev->data;
    uint8_t tx_buf[2];
    tx_buf[0]=BANK_SEL_REG; tx_buf[1]=BANK0;
    if(i2c_write_dt(i2c_dev, tx_buf, 2)<0) return 0;
    tx_buf[0]=PWR_MGMT_1; tx_buf[1]=0x41;
    if(i2c_write_dt(i2c_dev, tx_buf, 2)<0) return 0;

    return 1;
}
int icm20948_fifo_burst_read(const struct device_t* dev, uint8_t* data, uint32_t* len_of_raw)
{
    const struct i2c_dt_spec* i2c_dev=(const struct i2c_dt_spec*)dev->data;
    uint8_t fifo_count_l_h[2];
    int ret=i2c_burst_read_dt(i2c_dev, FIFO_COUNT_H, fifo_count_l_h, 2); 
    printk("\n    (...imu fifo level read ret: %d", ret);
    if(ret!=0) return 0;
    
    (*len_of_raw)=(uint32_t)((fifo_count_l_h[0]<<8) | (fifo_count_l_h[1]));
    printk("\n    (...imu raw len read = %d)", (*len_of_raw));
    ret=i2c_burst_read_dt(i2c_dev, FIFO_R_W, data, (*len_of_raw));
    printk("\n    (...imu fifo burst read ret: %d)", ret); 
    if(ret!=0) return 0;

    return 1;
}
int icm20948_reg_read(const struct device_t* dev, uint8_t reg_addr, uint8_t* data)
{
    const struct i2c_dt_spec* i2c_dev=(const struct i2c_dt_spec*)dev->data;
    int ret=i2c_write_read_dt(i2c_dev, &reg_addr, 1, data, 1);
    printk("\n    (...imu reg read ret: %d)", ret); 
    if(ret<0) return 0;
    
    return 1;
}
void icm20948_data_proc(uint8_t raw[], uint32_t len_of_raw, icm_data_format result[], uint32_t* len_of_result)
{
    uint32_t result_idx=0;
    for(int i=0; i<len_of_raw; i+=6)
    {
        result[result_idx].x=(int16_t)((raw[i]<<8)|raw[i+1]);    
        result[result_idx].y=(int16_t)((raw[i+2]<<8)|raw[i+3]);
        result[result_idx].z=(int16_t)((raw[i+4]<<8)|raw[i+5]);
        result_idx+=1;
    }
    (*len_of_result)=result_idx;
}
/*******************************************************************************************************/
/*                                        ICM-20948 DRIVER END                                         */
/*******************************************************************************************************/