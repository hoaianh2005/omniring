#include "hwDefine.h"
/*
*
*
*
*
*/
const struct device* gpio0;
const struct device* gpio1;
const struct device* uart0 = DEVICE_DT_GET(DT_NODELABEL(uart0));
const struct device* uart1 = DEVICE_DT_GET(DT_NODELABEL(uart1));

#define BMI270_NODE DT_NODELABEL(bmi270)
const struct i2c_dt_spec bmi270 = I2C_DT_SPEC_GET(BMI270_NODE);
//#define ICM_NODE DT_NODELABEL(icm20948)
//const struct i2c_dt_spec icm20948 = I2C_DT_SPEC_GET(ICM_NODE);
#define AS_NODE DT_NODELABEL(as7057)
const struct i2c_dt_spec as7057 = I2C_DT_SPEC_GET(AS_NODE);
#define BQ_NODE DT_NODELABEL(bq27220)
const struct i2c_dt_spec bq27220 = I2C_DT_SPEC_GET(BQ_NODE);