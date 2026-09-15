#include "as7057.h"
/*
*
*
*
*
*/
/*******************************************************************************************************/
/*                                          AS7057 DRIVER START                                        */
/*******************************************************************************************************/
int as7057_init(const struct device_t* dev)
{
    const struct i2c_dt_spec* i2c_dev=(const struct i2c_dt_spec*)dev->data;
    uint8_t addr=SILICON_ID; uint8_t ret;
    if(i2c_write_read_dt(i2c_dev, &addr, 1, &ret, 1)<0) return 0;
	if(ret) return 1;
    return 0;
}
int as7057_config(const struct device_t* dev)
{
    const struct i2c_dt_spec* i2c_dev=(const struct i2c_dt_spec*)dev->data;
    uint8_t tx_buf[2];

    // uint8_t reset_cmd[]={CHIP_CTRL, 0x01};
    // if(i2c_write_dt(i2c_dev, reset_cmd, 2)<0) return 0;
    // k_msleep(5); 
    // uint8_t sys_cfg[]={0x00, 0x07, 0x61, 0x60, 0x60, 0xFC, 0x06};
    // if(i2c_burst_write_dt(i2c_dev, CONTROL, sys_cfg, 7)!=0) return 0;
    // uint8_t mod_cfg[]={0x14, 0x07, 0x27, 0x04}; //, 0x27, 0x07};
    // if(i2c_burst_write_dt(i2c_dev, MOD1_CFGA, mod_cfg, 4)!=0) return 0;
    // tx_buf[0]=SEQ_CONFIG; tx_buf[1]=0x09;
    // if(i2c_write_dt(i2c_dev, tx_buf, sizeof(tx_buf))<0) return 0;
    // uint8_t freq_cfg[]={0x7F, 0x02, 0x00, 0x00};
    // if(i2c_burst_write_dt(i2c_dev, SEQ_FREQL, freq_cfg, 4)!=0) return 0;
    // tx_buf[0]=MOD1_SEQ1_SUB_EN; tx_buf[1]=0x03;
    // if(i2c_write_dt(i2c_dev, tx_buf, sizeof(tx_buf))<0) return 0;
    // tx_buf[0]=SEQ_MODCONF; tx_buf[1]=0x00;
    // if(i2c_write_dt(i2c_dev, tx_buf, sizeof(tx_buf))<0) return 0;
    // tx_buf[0]=LED_SEQ1_SUB12; tx_buf[1]=0x01;
    // if(i2c_write_dt(i2c_dev, tx_buf, sizeof(tx_buf))<0) return 0;
    // tx_buf[0]=SEQ1_LED1_CURR; tx_buf[1]=0x0D;
    // if(i2c_write_dt(i2c_dev, tx_buf, sizeof(tx_buf))<0) return 0;
    // tx_buf[0]=SEQ_LED_INIT; tx_buf[1]=0x14;
    // if(i2c_write_dt(i2c_dev, tx_buf, sizeof(tx_buf))<0) return 0;
    // tx_buf[0]=SEQ_SUB_WAIT; tx_buf[1]=0x1E;
    // if(i2c_write_dt(i2c_dev, tx_buf, sizeof(tx_buf))<0) return 0;
    // uint8_t pd_cfg[]={0x50, 0x50};
    // if(i2c_burst_write_dt(i2c_dev, PD_SEQ1_SUB1, pd_cfg, 2)!=0) return 0;
    // tx_buf[0]=PDSEL_CFG; tx_buf[1]=0x00;
    // if(i2c_write_dt(i2c_dev, tx_buf, sizeof(tx_buf))<0) return 0;
    // uint8_t sinc_cfg[]={0x2C, 0x01};
    // if(i2c_burst_write_dt(i2c_dev, SEQ1_SINC_CFGA, sinc_cfg, 2)!=0) return 0;
    // uint8_t aoc_cfg[]={0x10, 0xAB, 0x61, 0x80, 0x03};
    // if(i2c_burst_write_dt(i2c_dev, AOC_CFG, aoc_cfg, 5)!=0) return 0;
    // uint8_t standby_cfg_1[]={0x7F, 0x04, 0x02, 0x04};
    // if(i2c_burst_write_dt(i2c_dev, STANDBY_ON, standby_cfg_1, 4)!=0) return 0;
    // uint8_t standby_cfg_2[]={0x03, 0x10, 0x10};
    // if(i2c_burst_write_dt(i2c_dev, STANDBY_EN5, standby_cfg_2, 3)!=0) return 0;
    // tx_buf[0]=IRQ_ENABLE; tx_buf[1]=0x0F;
    // if(i2c_write_dt(i2c_dev, tx_buf, sizeof(tx_buf))<0) return 0;
    // uint8_t fifo_cfg[]={0x3C, 0x80};
    // if(i2c_burst_write_dt(i2c_dev, FIFO_THRESHOLD, fifo_cfg, 2)!=0) return 0;
    // tx_buf[0]=SEQ_START; tx_buf[1]=0x01;
    // if(i2c_write_dt(i2c_dev, tx_buf, sizeof(tx_buf))<0) return 0;
    // return 1;

    // --- 1. RESET HỆ THỐNG ---
    tx_buf[0]=CHIP_CTRL; tx_buf[1]=0x01;
    int ret[19];
    ret[0]=i2c_write_dt(i2c_dev, tx_buf, sizeof(tx_buf)); // CHIP_CTRL: Soft Reset 
    k_msleep(5);
    // --- 2. CẤU HÌNH HỆ THỐNG & CLOCK ---  
    tx_buf[0]=CONTROL; tx_buf[1]=0x00;
    ret[1]=i2c_write_dt(i2c_dev, tx_buf, sizeof(tx_buf)); // CONTROL: I2C Standard
    tx_buf[0]=CGB_CFG; tx_buf[1]=0x07;
    ret[2]=i2c_write_dt(i2c_dev, tx_buf, sizeof(tx_buf)); // CGB_CFG: LF, HF, PLL ON
    tx_buf[0]=INT_CFG; tx_buf[1]=0x61;
    ret[3]=i2c_write_dt(i2c_dev, tx_buf, sizeof(tx_buf)); // INT_CFG: Falling Edge (tối ưu cho ngắt), Increase driver strength for INT pin
    tx_buf[0]=CSXN_CFG; tx_buf[1]=0x60;
    ret[4]=i2c_write_dt(i2c_dev, tx_buf, sizeof(tx_buf)); // CSXN_CFG: Driver strength
    tx_buf[0]=IO_CFG; tx_buf[1]=0x60;
    ret[5]=i2c_write_dt(i2c_dev, tx_buf, sizeof(tx_buf)); // IO_CFG: SDA driver strength
    // --- 3. ĐIỆN ÁP THAM CHIẾU ---
    // Sử dụng bộ lọc LP cho Bandgap để tín hiệu sạch nhất 
    tx_buf[0]=REF_CFGA; tx_buf[1]=0xFC;
    ret[6]=i2c_write_dt(i2c_dev, tx_buf, sizeof(tx_buf)); // REF_CFGA: No bypass LP, Old startup
    tx_buf[0]=REF_CFGB; tx_buf[1]=0x06;
    ret[7]=i2c_write_dt(i2c_dev, tx_buf, sizeof(tx_buf)); // REF_CFGB: Low noise reference
    // --- 4. CẤU HÌNH BỘ ĐIỀU CHẾ ---
    // Cấu hình Modulator 1 cho dải đo 16uA (AGAIN 4)
    tx_buf[0]=MOD1_CFGA; tx_buf[1]=0x17;
    ret[8]=i2c_write_dt(i2c_dev, tx_buf, sizeof(tx_buf)); // MOD1_CFGA: MOD1 Enable, finger: FS 128uA (17), wrist: FS 16uA (14) 
    tx_buf[0]=MOD1_CFGB; tx_buf[1]=0x04;
    ret[9]=i2c_write_dt(i2c_dev, tx_buf, sizeof(tx_buf)); // MOD1_CFGB: Scale factor, finger: 0.625x (04), wrist: 1x (07)
    tx_buf[0]=MOD1_CFGC; tx_buf[1]=0x2F;
    ret[10]=i2c_write_dt(i2c_dev, tx_buf, sizeof(tx_buf)); // MOD1_CFGC: DSM Amplitude, finger: 2F, wrist: 27
    tx_buf[0]=MOD1_CFGD; tx_buf[1]=0x0F;
    ret[11]=i2c_write_dt(i2c_dev, tx_buf, sizeof(tx_buf)); // MOD1_CFGD: IREF, finger: AGAIN 4 -> 16uA (0F), wrist: AGAIN 3 -> 8uA (07)
    // tx_buf[0]=MOD1_CFGE; tx_buf[1]=0x27;
    // if(i2c_write_dt(i2c_dev, tx_buf, sizeof(tx_buf))<0); // MOD1_CFGE: DSM Seq2 (giữ nguyên)
    // tx_buf[0]=MOD1_CFGF; tx_buf[1]=0x07;
    // if(i2c_write_dt(i2c_dev, tx_buf, sizeof(tx_buf))<0); // MOD1_CFGF: IREF Seq2 (giữ nguyên)
    // --- 5. TẦN SỐ LẤY MẪU & BỘ TUẦN TỰ (50Hz - 2 Sub-samples) ---      
    tx_buf[0]=SEQ_CONFIG; tx_buf[1]=0x09;
    ret[12]=i2c_write_dt(i2c_dev, tx_buf, sizeof(tx_buf)); // SEQ_CONFIG: Seq1 En, 2 subs (n+1=2)
    tx_buf[0]=SEQ_FREQL; tx_buf[1]=0x7F;
    ret[13]=i2c_write_dt(i2c_dev, tx_buf, sizeof(tx_buf)); // SEQ_FREQL: n=639 -> 50Hz
    tx_buf[0]=SEQ_FREQH; tx_buf[1]=0x02;
    ret[14]=i2c_write_dt(i2c_dev, tx_buf, sizeof(tx_buf)); // SEQ_FREQH: n=639 -> 50Hz
    tx_buf[0]=SEQ1_FREQDIVL; tx_buf[1]=0x00;
    ret[15]=i2c_write_dt(i2c_dev, tx_buf, sizeof(tx_buf)); // SEQ1_FREQDIVL: Divider=1 
    tx_buf[0]=SEQ1_FREQDIVH; tx_buf[1]=0x00;
    ret[16]=i2c_write_dt(i2c_dev, tx_buf, sizeof(tx_buf)); // SEQ1_FREQDIVH
    tx_buf[0]=MOD1_SEQ1_SUB_EN; tx_buf[1]=0x03;
    ret[17]=i2c_write_dt(i2c_dev, tx_buf, sizeof(tx_buf)); // MOD1_SEQ1_SUB_EN: Kích hoạt Sub1 & Sub2 cho MOD1 
    tx_buf[0]=SEQ_MODCONF; tx_buf[1]=0x01;
    ret[18]=i2c_write_dt(i2c_dev, tx_buf, sizeof(tx_buf)); // SEQ_MODCONF: Modulator clock 5MHz (SNR tốt hơn 10MHz), 01: finger, 00: wrist
    // --- 6. CẤU HÌNH LED (LED 1 XANH: Sub1 OFF, Sub2 ON) ---  
    tx_buf[0]=LED_SEQ1_SUB12; tx_buf[1]=0x01;
    if(i2c_write_dt(i2c_dev, tx_buf, sizeof(tx_buf))<0); // LED_SEQ1_SUB12: Sub1=OFF, Sub2=LED1 ON
    tx_buf[0]=SEQ1_LED1_CURR; tx_buf[1]=0x0F;
    if(i2c_write_dt(i2c_dev, tx_buf, sizeof(tx_buf))<0); // SEQ1_LED1_CURR: ~9.4mA (1.5625mA * 6), 06: finger, 0D: wrist
    tx_buf[0]=SEQ_LED_INIT; tx_buf[1]=0x14;
    if(i2c_write_dt(i2c_dev, tx_buf, sizeof(tx_buf))<0); // SEQ_LED_INIT: 20us setup time (theo SpO2 sample)
    tx_buf[0]=SEQ_SUB_WAIT; tx_buf[1]=0x1E;
    if(i2c_write_dt(i2c_dev, tx_buf, sizeof(tx_buf))<0); // SEQ_SUB_WAIT: 30us giữa các sub
    // --- 7. CẤU HÌNH PHOTODIODE (PD1 & PD3 cho MOD1) ---  
    tx_buf[0]=PD_SEQ1_SUB1; tx_buf[1]=0x50;
    if(i2c_write_dt(i2c_dev, tx_buf, sizeof(tx_buf))<0); // PD_SEQ1_SUB1: PD1&PD3 -> MOD1 
    tx_buf[0]=PD_SEQ1_SUB2; tx_buf[1]=0x50;
    if(i2c_write_dt(i2c_dev, tx_buf, sizeof(tx_buf))<0); // PD_SEQ1_SUB2: PD1&PD3 -> MOD1
    tx_buf[0]=PDSEL_CFG; tx_buf[1]=0x00;
    if(i2c_write_dt(i2c_dev, tx_buf, sizeof(tx_buf))<0); // PDSEL_CFG: PDREF to VCM
    // --- 8. BỘ LỌC KỸ THUẬT SỐ ---
    tx_buf[0]=SEQ1_SINC_CFGA; tx_buf[1]=0x64;
    if(i2c_write_dt(i2c_dev, tx_buf, sizeof(tx_buf))<0); // SEQ1_SINC_CFGA: finger: 64, wrist: 2C
    tx_buf[0]=SEQ1_SINC_CFGB; tx_buf[1]=0x01;
    if(i2c_write_dt(i2c_dev, tx_buf, sizeof(tx_buf))<0); // SEQ1_SINC_CFGB: Order 5, CIC mode
    // --- 9. TỰ ĐỘNG BÙ DÒNG (AOC - Giữ nguyên SpO2 style) --- 
    tx_buf[0]=AOC_CFG; tx_buf[1]=0x10;
    if(i2c_write_dt(i2c_dev, tx_buf, sizeof(tx_buf))<0); // AOC_CFG: OVS 1
    tx_buf[0]=AOC_MOD1_THH; tx_buf[1]=0xAB;
    if(i2c_write_dt(i2c_dev, tx_buf, sizeof(tx_buf))<0); // AOC_MOD1_THH: Ngưỡng cao bão hòa
    tx_buf[0]=AOC_MOD1_THL; tx_buf[1]=0x61;
    if(i2c_write_dt(i2c_dev, tx_buf, sizeof(tx_buf))<0); // AOC_MOD1_THL: Ngưỡng thấp
    tx_buf[0]=MOD1_SEQ1_AOC_EN; tx_buf[1]=0x03;
    if(i2c_write_dt(i2c_dev, tx_buf, sizeof(tx_buf))<0); // MOD1_SEQ1_AOC_EN: Bật AOC cho cả Sub1 và Sub2
    tx_buf[0]=AOC_SAR_THRES; tx_buf[1]=0x80;
    if(i2c_write_dt(i2c_dev, tx_buf, sizeof(tx_buf))<0); // AOC_SAR_THRES
    // --- 10. CHẾ ĐỘ CHỜ TIẾT KIỆM NĂNG LƯỢNG (DYNAMIC STANDBY) ---
    // Tự động tắt các khối analog giữa các lần đo để tiết kiệm pin
    tx_buf[0]=STANDBY_ON; tx_buf[1]=0x7F;
    if(i2c_write_dt(i2c_dev, tx_buf, sizeof(tx_buf))<0); // STANDBY_ON: Bật Dynamic Standby
    tx_buf[0]=STANDBY_EN1; tx_buf[1]=0x04;
    if(i2c_write_dt(i2c_dev, tx_buf, sizeof(tx_buf))<0); // STANDBY_EN1: CGB wake-up
    tx_buf[0]=STANDBY_EN2; tx_buf[1]=0x02;
    if(i2c_write_dt(i2c_dev, tx_buf, sizeof(tx_buf))<0); // STANDBY_EN2: REF wake-up
    tx_buf[0]=STANDBY_EN3; tx_buf[1]=0x04;
    if(i2c_write_dt(i2c_dev, tx_buf, sizeof(tx_buf))<0); // STANDBY_EN3: Bandgap
    tx_buf[0]=STANDBY_EN5; tx_buf[1]=0x03;
    if(i2c_write_dt(i2c_dev, tx_buf, sizeof(tx_buf))<0); // STANDBY_EN5: MOD Current
    tx_buf[0]=STANDBY_EN6; tx_buf[1]=0x10;
    if(i2c_write_dt(i2c_dev, tx_buf, sizeof(tx_buf))<0); // STANDBY_EN6: VCM Buffers
    tx_buf[0]=STANDBY_EN7; tx_buf[1]=0x10;
    if(i2c_write_dt(i2c_dev, tx_buf, sizeof(tx_buf))<0); // STANDBY_EN7: MOD1 Enable
    // --- 11. FIFO & NGẮT ---      
    tx_buf[0]=IRQ_ENABLE; tx_buf[1]=0x0F;
    if(i2c_write_dt(i2c_dev, tx_buf, sizeof(tx_buf))<0); // IRQ_ENABLE: asat, led_lowvds, fifo_threshold (default=64), sequencer
    tx_buf[0]=FIFO_THRESHOLD; tx_buf[1]=0x3C;
    if(i2c_write_dt(i2c_dev, tx_buf, sizeof(tx_buf))<0); // 60x3byte
    tx_buf[0]=FIFO_CTRL; tx_buf[1]=0x80;
    if(i2c_write_dt(i2c_dev, tx_buf, sizeof(tx_buf))<0); // FIFO_CTRL: Xóa FIFO trước khi bắt đầu
    // --- 12. BẮT ĐẦU ĐO ---
    tx_buf[0]=SEQ_START; tx_buf[1]=0x01;
    if(i2c_write_dt(i2c_dev, tx_buf, sizeof(tx_buf))<0); // SEQ_START: Start measurement
    printk("\n    (...ppg each config ret checking: ) -> ret: ");
    for(int i=0; i<19; i++)
    {
        printk("%d ", ret[i]);
    }
    return 1;
}
int as7057_fifo_burst_read(const struct device_t* dev, uint8_t* data, uint32_t* len_of_raw)
{
    const struct i2c_dt_spec* i2c_dev=(const struct i2c_dt_spec*)dev->data;
    uint8_t addr=FIFO_LEVEL0;
    int ret=i2c_write_read_dt(i2c_dev, &addr, 1, len_of_raw, 1);
    printk("\n    (...ppg fifo level read = %d)", (*len_of_raw));
    if(ret<0) return 0;

    (*len_of_raw)*=3;
    ret=i2c_burst_read_dt(i2c_dev, FIFOL, data, (*len_of_raw));
    printk("\n    (...ppg fifo burst read ret: %d)", ret);
    if(ret!=0) return 0;

    return 1;
}
int as7057_reg_read(const struct device_t* dev, uint8_t reg_addr, uint8_t* data)
{
    const struct i2c_dt_spec* i2c_dev=(const struct i2c_dt_spec*)dev->data;
    int ret=i2c_write_read_dt(i2c_dev, &reg_addr, 1, data, 1);
    printk("\n    (...ppg reg read = %d)", ret);
    if(ret<0) return 0;
    
	return 1;
}
void as7057_data_proc(uint8_t raw[], uint32_t len_of_raw, int32_t result[], uint8_t* len_of_result)
{
    uint32_t result_idx=0;
    uint32_t raw_idx=0;
    while(raw_idx<=len_of_raw-6)
    {
        uint8_t marker_sub1=raw[raw_idx]&0x07;
        if (marker_sub1==0x00)
        {
            uint8_t marker_sub2=raw[raw_idx+3]&0x07;
            if (marker_sub2==0x02)
            {
                uint32_t raw_val0=(((uint32_t)raw[raw_idx+2]<<12)|((uint32_t)raw[raw_idx+1]<<4)|((raw[raw_idx]>>4)&0x0F));
                uint32_t raw_val1=(((uint32_t)raw[raw_idx+5]<<12)|((uint32_t)raw[raw_idx+4]<<4)|((raw[raw_idx+3]>>4)&0x0F));
                int32_t sub1_val=((int32_t)(raw_val0<<12)>>12);
                int32_t sub2_val=((int32_t)(raw_val1<<12)>>12);
                result[result_idx++]=sub2_val-sub1_val;
                raw_idx+=6;
                continue;
            }
        }
        raw_idx+=3;
    }
    (*len_of_result)=result_idx;
}
/*******************************************************************************************************/
/*                                           AS7057 DRIVER END                                         */
/*******************************************************************************************************/