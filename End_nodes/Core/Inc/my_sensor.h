/*
 * my_sensor.h
 *
 *  Created on: 11 Feb 2023
 *      Author: brock
 */

#ifndef INC_MY_SENSOR_H_
#define INC_MY_SENSOR_H_

#include "stm32wlxx_hal.h"

/* Configuration defines */
#define FILTER		BME68X_FILTER_OFF
#define ODR 		BME68X_ODR_NONE
#define OS_HUM 		BME68X_OS_8X     //BME68X_OS_1X, previously BME68X_OS_8X
#define OS_PRES		BME68X_OS_8X     //BME68X_OS_1X, previously BME68X_OS_8X
#define OS_TEMP		BME68X_OS_8X     //BME68X_OS_4X, previously BME68X_OS_8X
#define SENSE_GAS	1 // if 0, no heating and no sensing of the gas
#define HEATR_TEMP	300 // See datasheet, typically between 200 and 400
#define HEATR_DUR	500 // See datasheet, typically takes 20-30ms to reach target temp

/** Initialise the BME680 sensor, after that it is ready to be used **/
void my_sensor_init(I2C_HandleTypeDef *hi2c, uint8_t perform_sefltest);

void my_sensor_init_test_temp(I2C_HandleTypeDef *hi2c, uint8_t perform_sefltest, uint16_t temperature_value);

void my_sensor_init_test_dur(I2C_HandleTypeDef *hi2c, uint8_t perform_selftest, uint16_t duration_value);

/** Fetch new sensor data and returns a pointer to data structure **/
struct bme68x_data *get_BME_data();

double compute_FWI(struct bme68x_data *data);


#endif /* INC_MY_SENSOR_H_ */
