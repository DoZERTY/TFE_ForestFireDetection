/*
 * my_sensor.h
 *
 *  Created on: 11 Feb 2023
 *      Author: brock
 */

#ifndef INC_MY_SENSOR_H_
#define INC_MY_SENSOR_H_

#include "stm32wlxx_hal.h"

void my_sensor_init(I2C_HandleTypeDef *hi2c, uint8_t perform_sefltest);

#endif /* INC_MY_SENSOR_H_ */
