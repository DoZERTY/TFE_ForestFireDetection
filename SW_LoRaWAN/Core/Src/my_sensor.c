/*
 * my_sensor.c
 *
 *  Created on: 11 Feb 2023
 *      Author: brock
 */

#include "my_sensor.h"
#include "bme68x.h"
#include "main.h"



/* Variables */
struct bme68x_dev bme680_dev;
static uint8_t dev_addr;
I2C_HandleTypeDef *my_hi2c;


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
}

/** Initialise the BME680 sensor, after that it is ready to be used **/
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
}
