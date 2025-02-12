/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.h
  * @brief          : Header for main.c file.
  *                   This file contains the common defines of the application.
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2022 STMicroelectronics.
  * All rights reserved.
  *
  * This software is licensed under terms that can be found in the LICENSE file
  * in the root directory of this software component.
  * If no LICENSE file comes with this software, it is provided AS-IS.
  *
  ******************************************************************************
  */
/* USER CODE END Header */

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __MAIN_H
#define __MAIN_H

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "stm32wlxx_hal.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */

/* USER CODE END Includes */

/* Exported types ------------------------------------------------------------*/
/* USER CODE BEGIN ET */

/* USER CODE END ET */

/* Exported constants --------------------------------------------------------*/
/* USER CODE BEGIN EC */

/* USER CODE END EC */

/* Exported macro ------------------------------------------------------------*/
/* USER CODE BEGIN EM */

/* USER CODE END EM */

/* Exported functions prototypes ---------------------------------------------*/
void Error_Handler(void);

/* USER CODE BEGIN EFP */
int change_SF(void);
int change_power(void);
void blink_led(int blinknumber);
void debug_print(const char* out);
void start_cycle_count();
void stop_cycle_count(char *s);
/* USER CODE END EFP */

/* Private defines -----------------------------------------------------------*/
#define RTC_PREDIV_A ((1<<(15-RTC_N_PREDIV_S))-1)
#define RTC_N_PREDIV_S 10
#define RTC_PREDIV_S ((1<<RTC_N_PREDIV_S)-1)
#define BatExt_CS_Pin GPIO_PIN_8
#define BatExt_CS_GPIO_Port GPIOB
#define BatExt_CD_Pin GPIO_PIN_0
#define BatExt_CD_GPIO_Port GPIOA
#define BatExt_PWR_Pin GPIO_PIN_4
#define BatExt_PWR_GPIO_Port GPIOA
#define LED_Pin GPIO_PIN_5
#define LED_GPIO_Port GPIOA
#define Button1_Pin GPIO_PIN_6
#define Button1_GPIO_Port GPIOA
#define Button1_EXTI_IRQn EXTI9_5_IRQn
#define Button2_Pin GPIO_PIN_7
#define Button2_GPIO_Port GPIOA
#define Button2_EXTI_IRQn EXTI9_5_IRQn
#define BatExt_gain1_Pin GPIO_PIN_10
#define BatExt_gain1_GPIO_Port GPIOA
#define BatExt_gain2_Pin GPIO_PIN_11
#define BatExt_gain2_GPIO_Port GPIOA

/* USER CODE BEGIN Private defines */
#define VERBOSE					1

// RTC must not be at 1 if SEND_SYNC_PACKETS is set (the receiver doesn't sense)
#define RTC_FETCH_SEND_BME		0
#define RTC_DELAY				12000	// must be enough so that there is no packet collision with other senders

// The sync packets are sent by the future receiver to trigger the RTC of the sensors
#define SEND_SYNC_PACKETS		1	// So it is the receiver
#define RECV_SYNC_PACKETS		0	// So it is the sensor/emitter

// Button 1 - Sweep either power or SF
#define CHANGESF 				0		// Sweep between SF7 -> SF12
#define CHANGETXPOWER			1		// Sweep between -5 -> 14 dBm
#define BTN1ACTION				CHANGESF

// Button 2 - Either send or receive mode, with other features (RTC start, sync packets)
#define SENDPACKET				0
#define RECEIVEPACKET			1
#define RXTIMEOUT				180000
#define BTN2ACTION				SENDPACKET    // modify for the board n°2
#define FWI_COMPUTATION			1     // if 0: FWI not computed

#define TEST_MODE				0       // 0 if no test to be performed, otherwise 1
#define TEST_NUMBER				2

// Enable performance measurements
#define PERF_COUNT 1

void print_now(char*);  //__attribute__((noinline))
void print_int(int);
void print_error(char*, int);
void blink(int);
/* USER CODE END Private defines */

#ifdef __cplusplus
}
#endif

#endif /* __MAIN_H */
