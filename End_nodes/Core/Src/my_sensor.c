/*
 * my_sensor.c
 *
 *  Created on: 11 Feb 2023
 *      Author: brock
 */

#include "my_sensor.h"
#include "bme68x.h"
#include "main.h"
#include "sys_app.h"
#include "utils.h"

#include "FWI.h"



/* Variables */
struct bme68x_dev bme680_dev;
I2C_HandleTypeDef *my_hi2c;
static uint8_t dev_addr;
struct bme68x_conf conf;
struct bme68x_heatr_conf heatr_conf;
struct bme68x_data data;
uint8_t n_fields;


/**************************/
/*** INTERFACE FUNCTIONS***/
/**************************/
BME68X_INTF_RET_TYPE bme68x_i2c_read(uint8_t reg_addr, uint8_t *reg_data, uint32_t len, void *intf_ptr)
{
	if(HAL_I2C_Mem_Read(my_hi2c, *((uint16_t *)intf_ptr), reg_addr, 1, (uint8_t *) reg_data, len, HAL_MAX_DELAY) == HAL_OK) {return BME68X_OK;}
	else {return BME68X_E_COM_FAIL;}
}

BME68X_INTF_RET_TYPE bme68x_i2c_write(uint8_t reg_addr, const uint8_t *reg_data, uint32_t len, void *intf_ptr)
{
	if(HAL_I2C_Mem_Write(my_hi2c, *((uint16_t *)intf_ptr), reg_addr, 1, (uint8_t *) reg_data, len, HAL_MAX_DELAY) == HAL_OK) {return BME68X_OK;}
	else {return BME68X_E_COM_FAIL;}
}

void bme68x_delay_us(uint32_t period, void *intf_ptr)
{
	HAL_Delay(period/1000); // je crois que c est ok pcq la fonction est utilisee pour des delais de 5ms a 1s
}

static int8_t bme68x_interface_init(struct bme68x_dev *bme)
{
    int8_t rslt = BME68X_OK;
    if (bme != NULL) {
    	dev_addr = BME68X_I2C_ADDR_LOW<<1;
    	bme->read = &bme68x_i2c_read;
    	bme->write = &bme68x_i2c_write;
    	bme->intf = BME68X_I2C_INTF;
    	bme->delay_us = &bme68x_delay_us;
    	bme->intf_ptr = &dev_addr;
    	bme->amb_temp = 20; /* The ambient temperature in deg C is used for defining the heater temperature */
    }
    else{
        rslt = BME68X_E_NULL_PTR;
    }
    return rslt;
}
/**************************/


/*** Utilitary function to print error code ***/
void bme68x_check_rslt(const char* api_name, int8_t rslt)
{
#if(VERBOSE)
	debug_print(api_name);
    switch (rslt)
    {
        case BME68X_OK:
        	debug_print(" : Success\r\n");
            break;
        case BME68X_E_NULL_PTR:
            debug_print(" : Null pointer\r\n");
            break;
        case BME68X_E_COM_FAIL:
        	debug_print(" : Communication failure\r\n");
            break;
        case BME68X_E_INVALID_LENGTH:
        	debug_print(" : Incorrect length parameter\r\n");
            break;
        case BME68X_E_DEV_NOT_FOUND:
        	debug_print(" : Device not found\r\n");
            break;
        case BME68X_E_SELF_TEST:
        	debug_print(" : Self test error\r\n");
            break;
        case BME68X_W_NO_NEW_DATA:
        	debug_print(" : No new data found\r\n");
            break;
        default:
        	debug_print(" : Unknown error code\r\n");
            break;
    }
#endif
}

void my_sensor_init(I2C_HandleTypeDef *hi2c, uint8_t perform_selftest)
{
	my_hi2c = hi2c;

	int8_t rslt;

	rslt = bme68x_interface_init(&bme680_dev);
	bme68x_check_rslt("BME680 interface init", rslt);

	rslt = bme68x_init(&bme680_dev);
	bme68x_check_rslt("BME680 init", rslt);

	if(perform_selftest) {
		debug_print("BME680 selftest check begin...\r\n");
		rslt = bme68x_selftest_check(&bme680_dev);
		bme68x_check_rslt("BME680 selftest check", rslt);
	}

	conf.filter = FILTER;
	conf.odr = ODR;
	conf.os_hum  = OS_HUM;
	conf.os_pres = OS_PRES;
	conf.os_temp = OS_TEMP;
	rslt = bme68x_set_conf(&conf, &bme680_dev);
	bme68x_check_rslt("bme68x_set_conf", rslt);

#if(SENSE_GAS)
	heatr_conf.enable = BME68X_ENABLE;
	heatr_conf.heatr_temp = HEATR_TEMP;   // heater T° set to 300 °C
	heatr_conf.heatr_dur = HEATR_DUR;     // heating duration set to 50 ms
	rslt = bme68x_set_heatr_conf(BME68X_FORCED_MODE, &heatr_conf, &bme680_dev);
	bme68x_check_rslt("bme68x_set_heatr_conf", rslt);
#else
	heatr_conf.enable = BME68X_DISABLE;
	rslt = bme68x_set_heatr_conf(BME68X_FORCED_MODE, &heatr_conf, &bme680_dev);
	bme68x_check_rslt("bme68x_set_heatr_conf", rslt);
#endif

	rslt = bme68x_set_op_mode(BME68X_SLEEP_MODE, &bme680_dev);
	bme68x_check_rslt("bme68x_set_op_mode", rslt);
}

void print_data_bme(uint8_t n_fields){
	APP_PRINTF("T [C*100]\t P [Pa]\t\t H [%%*1000]\t Gas R [ohm]\t Status\n");
	if (n_fields) {
#ifdef BME68X_USE_FPU
		APP_PRINTF("%.2f\t %.2f\t %.2f\t %.2f\t 0x%x\r\n",
				data.temperature,
				data.pressure,
				data.humidity,
				data.gas_resistance,
				data.status);
#else
		APP_PRINTF("%d\t\t %u\t\t %u\t\t %u\t\t 0x%x\r\n",
				data.temperature,
				data.pressure,
				data.humidity,
				data.gas_resistance,
				data.status);
#endif
	}
}

void my_sensor_init_test_temp(I2C_HandleTypeDef *hi2c, uint8_t perform_selftest, uint16_t temperature_value)
{
	my_hi2c = hi2c;

	int8_t rslt;

	rslt = bme68x_interface_init(&bme680_dev);
	bme68x_check_rslt("BME680 interface init", rslt);

	rslt = bme68x_init(&bme680_dev);
	bme68x_check_rslt("BME680 init", rslt);

	if(perform_selftest) {
		debug_print("BME680 selftest check begin...\r\n");
		rslt = bme68x_selftest_check(&bme680_dev);
		bme68x_check_rslt("BME680 selftest check", rslt);
	}

	conf.filter = FILTER;
	conf.odr = ODR;
	conf.os_hum  = OS_HUM;
	conf.os_pres = OS_PRES;
	conf.os_temp = OS_TEMP;
	rslt = bme68x_set_conf(&conf, &bme680_dev);
	bme68x_check_rslt("bme68x_set_conf", rslt);

#if(SENSE_GAS)
	heatr_conf.enable = BME68X_ENABLE;
	heatr_conf.heatr_temp = temperature_value;
	heatr_conf.heatr_dur = HEATR_DUR;     // heating duration set to 50 ms
	rslt = bme68x_set_heatr_conf(BME68X_FORCED_MODE, &heatr_conf, &bme680_dev);
	bme68x_check_rslt("bme68x_set_heatr_conf", rslt);
#else
	heatr_conf.enable = BME68X_DISABLE;
	rslt = bme68x_set_heatr_conf(BME68X_FORCED_MODE, &heatr_conf, &bme680_dev);
	bme68x_check_rslt("bme68x_set_heatr_conf", rslt);
#endif

	rslt = bme68x_set_op_mode(BME68X_SLEEP_MODE, &bme680_dev);
	bme68x_check_rslt("bme68x_set_op_mode", rslt);
}

void my_sensor_init_test_dur(I2C_HandleTypeDef *hi2c, uint8_t perform_selftest, uint16_t duration_value)
{
	my_hi2c = hi2c;

	int8_t rslt;

	rslt = bme68x_interface_init(&bme680_dev);
	bme68x_check_rslt("BME680 interface init", rslt);

	rslt = bme68x_init(&bme680_dev);
	bme68x_check_rslt("BME680 init", rslt);

	if(perform_selftest) {
		debug_print("BME680 selftest check begin...\r\n");
		rslt = bme68x_selftest_check(&bme680_dev);
		bme68x_check_rslt("BME680 selftest check", rslt);
	}

	conf.filter = FILTER;
	conf.odr = ODR;
	conf.os_hum  = OS_HUM;
	conf.os_pres = OS_PRES;
	conf.os_temp = OS_TEMP;
	rslt = bme68x_set_conf(&conf, &bme680_dev);
	bme68x_check_rslt("bme68x_set_conf", rslt);

#if(SENSE_GAS)
	heatr_conf.enable = BME68X_ENABLE;
	heatr_conf.heatr_temp = HEATR_TEMP;
	heatr_conf.heatr_dur = duration_value;     // heating duration set to 50 ms
	rslt = bme68x_set_heatr_conf(BME68X_FORCED_MODE, &heatr_conf, &bme680_dev);
	bme68x_check_rslt("bme68x_set_heatr_conf", rslt);
#else
	heatr_conf.enable = BME68X_DISABLE;
	rslt = bme68x_set_heatr_conf(BME68X_FORCED_MODE, &heatr_conf, &bme680_dev);
	bme68x_check_rslt("bme68x_set_heatr_conf", rslt);
#endif

	rslt = bme68x_set_op_mode(BME68X_SLEEP_MODE, &bme680_dev);
	bme68x_check_rslt("bme68x_set_op_mode", rslt);
}

struct bme68x_data *get_BME_data()
{
	int8_t rslt;
	uint32_t del_period;
	rslt = bme68x_set_op_mode(BME68X_FORCED_MODE, &bme680_dev);
	bme68x_check_rslt("bme68x_set_op_mode", rslt);
	/* Calculate delay period in microseconds */
#if(SENSE_GAS)
	del_period = bme68x_get_meas_dur(BME68X_FORCED_MODE, &conf, &bme680_dev) + (heatr_conf.heatr_dur * 1000);
#else
	del_period = bme68x_get_meas_dur(BME68X_FORCED_MODE, &conf, &bme680_dev);
#endif
//	debug_print("delay period in microseconds is \r\n");    // add to evaluate the measurement duration
//	print_int(del_period);
//	debug_print("\r\n");

	bme680_dev.delay_us(del_period, bme680_dev.intf_ptr);
	// blink not functioning
	rslt = bme68x_get_data(BME68X_FORCED_MODE, &data, &n_fields, &bme680_dev);
	bme68x_check_rslt("bme68x_get_data", rslt);

#if(VERBOSE)
	print_data_bme(n_fields);
#endif

	return &data;
}

double compute_FWI(struct bme68x_data *data)
{
	start_cycle_count();
	double temp_value = (double) data->temperature/100.0;
	double humidity_value = (double) data->humidity/1000.0;
	double wind = 10;
	double rain = 0;
	int I_value = 2;  // because February
	double ffmc = FFMCcalc(temp_value, humidity_value, wind, rain, ffmc0);
	double dmc = DMCcalc(temp_value, humidity_value, rain, dmc0, I_value);
	double dc = DCcalc(temp_value, rain, dc0, I_value);
	double isi = ISIcalc(wind, ffmc);
	double bui = BUIcalc(dmc, dc);
	double fwi = FWIcalc(isi, bui);

	// update of previous value
	ffmc0 = ffmc;
	dmc0 = dmc;
	dc0 = dc;
	stop_cycle_count("FWI cycle counts");

	return fwi;


}
