/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : Main program body
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
/* Includes ------------------------------------------------------------------*/
#include "main.h"
#include "adc.h"
#include "dma.h"
#include "i2c.h"
#include "usart.h"
#include "spi.h"
#include "app_subghz_phy.h"
#include "tim.h"
#include "gpio.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include <stdlib.h>
#include "string.h"
#include "my_sensor.h"
#include "radio.h"
#include "sys_app.h"
#include "subghz_phy_app.h"
#include "rtc.h"
#include "my_packet.h"

#include <stdio.h>

/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */
/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/

/* USER CODE BEGIN PV */

struct bme68x_data *bme_data;
int currentTxPower = TX_OUTPUT_POWER;
int currentSF = LORA_SPREADING_FACTOR;

int timertogglenumber;

/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
/* USER CODE BEGIN PFP */
void blink(int);
/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */
/* USER CODE END 0 */

/**
  * @brief  The application entry point.
  * @retval int
  */
int main(void)
{

  /* USER CODE BEGIN 1 */

  /* USER CODE END 1 */

  /* MCU Configuration--------------------------------------------------------*/

  /* Reset of all peripherals, Initializes the Flash interface and the Systick. */
  HAL_Init();

  /* USER CODE BEGIN Init */

  /* USER CODE END Init */

  /* Configure the system clock */
  SystemClock_Config();

  /* USER CODE BEGIN SysInit */

  /* USER CODE END SysInit */

  /* Initialize all configured peripherals */
  MX_GPIO_Init();
  MX_DMA_Init();
  MX_I2C1_Init();
  MX_LPUART1_UART_Init();
  MX_SubGHz_Phy_Init();
  MX_ADC_Init();
  MX_TIM1_Init();
  MX_SPI1_Init();
  /* USER CODE BEGIN 2 */
  //blink(1000);
  // Print banner and blink LED //
  debug_print("\nBonjour Brock !\r\n");

  // Init BME680 sensor with or without selftest //
  my_sensor_init(&hi2c1, 0);
  debug_print("My sensor is well initiated !\r\n\n");

  // Low power mode enable
//  HAL_PWREx_EnableLowPowerRunMode();

  /*** Suspend the systick before going into the STOP mode***/
//  HAL_SuspendTick();
//
//  /*** Enter the STOP mode ***/
//  HAL_PWR_EnterSTOPMode(PWR_LOWPOWERREGULATOR_ON, PWR_STOPENTRY_WFI);



  //
  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1)
  {
    /* USER CODE END WHILE */
    MX_SubGHz_Phy_Process();

    /* USER CODE BEGIN 3 */
  }
  /* USER CODE END 3 */
}

/**
  * @brief System Clock Configuration
  * @retval None
  */
void SystemClock_Config(void)
{
  RCC_OscInitTypeDef RCC_OscInitStruct = {0};
  RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};

  /** Configure LSE Drive Capability
  */
  HAL_PWR_EnableBkUpAccess();
  __HAL_RCC_LSEDRIVE_CONFIG(RCC_LSEDRIVE_LOW);

  /** Configure the main internal regulator output voltage
  */
  __HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE2);

  /** Initializes the CPU, AHB and APB buses clocks
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSI|RCC_OSCILLATORTYPE_LSE
                              |RCC_OSCILLATORTYPE_MSI;
  RCC_OscInitStruct.LSEState = RCC_LSE_ON;
  RCC_OscInitStruct.HSIState = RCC_HSI_ON;
  RCC_OscInitStruct.MSIState = RCC_MSI_ON;
  RCC_OscInitStruct.HSICalibrationValue = RCC_HSICALIBRATION_DEFAULT;
  RCC_OscInitStruct.MSICalibrationValue = RCC_MSICALIBRATION_DEFAULT;
  RCC_OscInitStruct.MSIClockRange = RCC_MSIRANGE_4;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_NONE;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }

  /** Configure the SYSCLKSource, HCLK, PCLK1 and PCLK2 clocks dividers
  */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK3|RCC_CLOCKTYPE_HCLK
                              |RCC_CLOCKTYPE_SYSCLK|RCC_CLOCKTYPE_PCLK1
                              |RCC_CLOCKTYPE_PCLK2;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_MSI;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV8;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV8;
  RCC_ClkInitStruct.AHBCLK3Divider = RCC_SYSCLK_DIV1;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_0) != HAL_OK)
  {
    Error_Handler();
  }
}

/* USER CODE BEGIN 4 */
void print_now(char* string){
	HAL_UART_Transmit(&hlpuart1, (uint8_t *)string, strlen(string), 0xFFFF);
}

int intbuffer[20]; //Considere we won't have more than 20 digits
char charbuffer[2] = {'c', '\0'};
void print_int(int my_int){
	int k = 0;
	while(my_int>0){
		char temp = my_int%10+48;
		intbuffer[k]=temp;
		k++;
		my_int /= 10;
	}
	print_now("\r\n");
	for(int j = k-1; j >=0; j-=1 ){
		charbuffer[0]= intbuffer[j];
		//print_now(charbuffer);
		debug_print(charbuffer);
	}
	//print_now("\r\n");
	debug_print("\r\n");
}
void print_error(char* string, int my_int){
	print_now(string);
	print_int(my_int);
}

void blink(int time_ms){

	HAL_GPIO_TogglePin(GPIOA, GPIO_PIN_5);
	HAL_Delay(time_ms);
	HAL_GPIO_TogglePin(GPIOA, GPIO_PIN_5);
}

/* GPIO button press callback */
void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin)
{
	/*** Wake-up from STOP mode ***/
//	  SystemClock_Config();
//	  HAL_ResumeTick();
//
//	  blink(200);

	//debug_print("GPIO EXTI callback - ");
	if(GPIO_Pin == Button1_Pin) {
		print_now("Hi Button 1\r\n");
		//batext_power_on();
		blink(500);
	} else if (GPIO_Pin == Button2_Pin) {
		#if((BTN2ACTION == SENDPACKET) & (!TEST_MODE))
				//send_dummy_packet();
//				blink(100);
				bme_data = get_BME_data();
//				debug_print("Retrieve BME 680 data ... \r\n");
				if (FWI_COMPUTATION){
					double fwi_value = compute_FWI(bme_data);
					APP_PRINTF("FWI value is %d...\r\n", (int) fwi_value);
				}
				make_packet(bme_data);
//				blink(50);
				send_packet();

				/*** Suspend the systick before going into the STOP mode***/
//			    HAL_SuspendTick();
//
//			    /*** Enter the STOP mode ***/
//			    HAL_PWR_EnterSTOPMode(PWR_LOWPOWERREGULATOR_ON, PWR_STOPENTRY_WFI);


//				HAL_Delay(100);
//			while(1){
//				bme_data = get_BME_data();
////				double fwi_value = compute_FWI(bme_data);
////				printf("FWI value is: %f", fwi_value);
//
//				make_packet(bme_data);
//				send_packet();
//				HAL_Delay(200);
//			}
		#elif((BTN2ACTION == SENDPACKET) & (TEST_MODE))
			#if (TEST_NUMBER == 0)
				bme_data = get_BME_data();
				make_packet(bme_data);
				send_packet();
			#elif (TEST_NUMBER == 1)  // TO do multiple times to test the reproducibility   => Can also be used for Exp3
				int temp;
				for(temp=200; temp<401; temp+=20){
					my_sensor_init_test_temp(&hi2c1, 0, temp);
					bme_data = get_BME_data();
					make_packet(bme_data);
//					send_packet();
					HAL_Delay(30000);    //wait 30s
				}
			#elif (TEST_NUMBER == 2)
				int dur, delta_t;
				dur = 25;
				my_sensor_init_test_dur(&hi2c1, 0, dur);
				for(delta_t=200; delta_t<2510; delta_t+=200){
					bme_data = get_BME_data();
					HAL_Delay(delta_t);
				}

			#endif

		#elif(BTN2ACTION == RECEIVEPACKET)
				APP_PRINTF("Radio in RX mode for %ds...\r\n", RXTIMEOUT/1000);
				Radio.Rx(RXTIMEOUT);
				HAL_GPIO_WritePin(GPIOA, GPIO_PIN_5, GPIO_PIN_SET);
		#endif


	} else {
		debug_print("other?\r\n");
	}
}


void HAL_RTCEx_WakeUpTimerEventCallback(RTC_HandleTypeDef *hrtc)
{
#if(RTC_FETCH_SEND_BME && !SEND_SYNC_PACKETS)
	debug_print("RTC Wake up !\r\n");
	change_SF(); // ONLY FOR SNR EXPERIENCE !!!!!!!!!!
	bme_data = get_BME_data();
	make_packet(bme_data);
	send_packet();
	blink_led(1);
#elif(SEND_SYNC_PACKETS && !RTC_FETCH_SEND_BME)
	send_sync_packet();
#endif
}


int change_SF()
{
	if(++currentSF > 12) {
		currentSF = 7;
	}
	SubghzApp_SetLoRaConfig(currentTxPower, currentSF);
#if(SEND_SYNC_PACKETS)
	APP_PRINTF("Radio in RX mode for %ds...\r\n", RXTIMEOUT/1000);
	Radio.Rx(RXTIMEOUT);
#endif
	return currentSF;
}

int change_power()
{
	if(++currentTxPower > 14) {
		currentTxPower = -5;
	}
	SubghzApp_SetLoRaConfig(currentTxPower, currentSF);
	return currentTxPower;
}


void debug_print(const char* out)
{
#if(VERBOSE)
	APP_PRINTF(out);
#endif
}

/* USER CODE END 4 */

/**
  * @brief  This function is executed in case of error occurrence.
  * @retval None
  */
void Error_Handler(void)
{
  /* USER CODE BEGIN Error_Handler_Debug */
  /* User can add his own implementation to report the HAL error return state */
  __disable_irq();
  while (1)
  {
  }
  /* USER CODE END Error_Handler_Debug */
}

#ifdef  USE_FULL_ASSERT
/**
  * @brief  Reports the name of the source file and the source line number
  *         where the assert_param error has occurred.
  * @param  file: pointer to the source file name
  * @param  line: assert_param error line source number
  * @retval None
  */
void assert_failed(uint8_t *file, uint32_t line)
{
  /* USER CODE BEGIN 6 */
  /* User can add his own implementation to report the file name and line number,
     ex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */
  /* USER CODE END 6 */
}
#endif /* USE_FULL_ASSERT */
