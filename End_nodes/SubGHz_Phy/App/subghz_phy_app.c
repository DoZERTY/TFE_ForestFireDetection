/* USER CODE BEGIN Header */
/**
 ******************************************************************************
 * @file    subghz_phy_app.c
 * @author  MCD Application Team
 * @brief   Application of the SubGHz_Phy Middleware
 ******************************************************************************
 * @attention
 *
 * Copyright (c) 2023 STMicroelectronics.
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
#include "platform.h"
#include "sys_app.h"
#include "subghz_phy_app.h"
#include "radio.h"

/* USER CODE BEGIN Includes */
#include "main.h" // for debug_print
#include "my_packet.h" // for decode_print_packet
/* USER CODE END Includes */

/* External variables ---------------------------------------------------------*/
/* USER CODE BEGIN EV */

/* USER CODE END EV */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */

/* Configurations */
/*Timeout*/
#define RX_TIMEOUT_VALUE              2000
#define TX_TIMEOUT_VALUE              3000
/* Definitions */
#define RX_CONTINUOUS_ON              1
#define RADIO_TX                      0 /* do not change*/
#define RADIO_RX                      1 /* do not change*/
#define PRBS9_INIT                    ( ( uint16_t) 2 )

/* Test Configurations */
/*if TEST_MODE is RADIO_TX, board will send packet indefinitely*/
/*if TEST_MODE is RADIO_RX, board will receive packet indefinitely*/
#define TEST_MODE                     RADIO_TX

#if (( USE_MODEM_LORA == 1 ) && ( USE_MODEM_FSK == 0 ))
/* 0: Tx Long packet shall be disable when USE_MODEM_LORA*/
#define APP_LONG_PACKET              0
/* Application buffer 255 max when USE_MODEM_LORA */
#define MAX_APP_BUFFER_SIZE              255
#elif (( USE_MODEM_LORA == 0 ) && ( USE_MODEM_FSK == 1 ))
/* 0: Tx Long packet disable*/
/* 1: Tx Long packet enable(payload can be greater than 255bytes. Available on stm32wl revision Y)*/
#define APP_LONG_PACKET               0
/* Application buffer, can be increased further*/
#define MAX_APP_BUFFER_SIZE           1000
#else
#error "Please define a modem in the compiler subghz_phy_app.h."
#endif /* USE_MODEM_LORA | USE_MODEM_FSK */

#if (PAYLOAD_LEN>MAX_APP_BUFFER_SIZE)
#error increase MAX_APP_BUFFER_SIZE
#endif /* (PAYLOAD_LEN>MAX_APP_BUFFER_SIZE) */

#if ((APP_LONG_PACKET==0) && PAYLOAD_LEN>255)
#error in case PAYLOAD_LEN>255, APP_LONG_PACKET shall be defined to 1
#endif /* ((APP_LONG_PACKET==0) && PAYLOAD_LEN>255) */

/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/
/* Radio events function pointer */
static RadioEvents_t RadioEvents;

/* USER CODE BEGIN PV */

static __IO uint32_t RadioTxDone_flag = 0;
static __IO uint32_t RadioTxTimeout_flag = 0;
static __IO uint32_t RadioRxDone_flag = 0;
static __IO uint32_t RadioRxTimeout_flag = 0;
static __IO uint32_t RadioError_flag = 0;
static __IO int16_t last_rx_rssi = 0;
static __IO int8_t last_rx_cfo = 0;

uint8_t data_buffer[MAX_APP_BUFFER_SIZE] UTIL_MEM_ALIGN(4);
uint16_t data_offset = 0;

static __IO uint16_t payloadLen = PAYLOAD_LEN;
#if (TEST_MODE == RADIO_TX)
//static uint16_t payloadLenMax = MAX_APP_BUFFER_SIZE;
#endif /* TEST_MODE == RADIO_TX */

#if (( USE_MODEM_LORA == 0 ) && ( USE_MODEM_FSK == 1 ))
static uint8_t syncword[] = { 0xC1, 0x94, 0xC1};
#endif /* USE_MODEM_FSK */

uint32_t count_RxOk = 0;
uint32_t count_RxKo = 0;
uint32_t PER = 0;

//static int32_t packetCnt = 0;

/* TxPayloadMode
 * 0: byte Inc e.g payload=0x00, 0x01, ..,payloadLen-1
 * 1: prbs9  */
static __IO uint8_t TxPayloadMode = 0;


/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
/*!
 * @brief Function to be executed on Radio Tx Done event
 */
static void OnTxDone(void);

/**
  * @brief Function to be executed on Radio Rx Done event
  * @param  payload ptr of buffer received
  * @param  size buffer size
  * @param  rssi
  * @param  LoraSnr_FskCfo
  */
static void OnRxDone(uint8_t *payload, uint16_t size, int16_t rssi, int8_t LoraSnr_FskCfo);

/**
  * @brief Function executed on Radio Tx Timeout event
  */
static void OnTxTimeout(void);

/**
  * @brief Function executed on Radio Rx Timeout event
  */
static void OnRxTimeout(void);

/**
  * @brief Function executed on Radio Rx Error event
  */
static void OnRxError(void);

/* USER CODE BEGIN PFP */

/* USER CODE END PFP */

/* Exported functions ---------------------------------------------------------*/
void SubghzApp_Init(void)
{
  /* USER CODE BEGIN SubghzApp_Init_1 */

  /* USER CODE END SubghzApp_Init_1 */

  /* Radio initialization */
  RadioEvents.TxDone = OnTxDone;
  RadioEvents.RxDone = OnRxDone;
  RadioEvents.TxTimeout = OnTxTimeout;
  RadioEvents.RxTimeout = OnRxTimeout;
  RadioEvents.RxError = OnRxError;

  Radio.Init(&RadioEvents);

  /* USER CODE BEGIN SubghzApp_Init_2 */

  /* Radio Set frequency */
  Radio.SetChannel(RF_FREQUENCY);

  /* Radio configuration */
#if ((USE_MODEM_LORA == 1) && (USE_MODEM_FSK == 0))
//  APP_LOG(TS_OFF, VLEVEL_M, "---------------\n\r");
//  APP_LOG(TS_OFF, VLEVEL_M, "LORA_MODULATION\n\r");
//  APP_LOG(TS_OFF, VLEVEL_M, "LORA_BW=%d kHz\n\r", (1 << LORA_BANDWIDTH) * 125);
//  APP_LOG(TS_OFF, VLEVEL_M, "LORA_SF=%d\n\r", LORA_SPREADING_FACTOR);

  Radio.SetTxConfig(MODEM_LORA, TX_OUTPUT_POWER, 0, LORA_BANDWIDTH,
		  LORA_SPREADING_FACTOR, LORA_CODINGRATE,
		  LORA_PREAMBLE_LENGTH, LORA_FIX_LENGTH_PAYLOAD_ON,
		  true, 0, 0, LORA_IQ_INVERSION_ON, TX_TIMEOUT_VALUE);

  Radio.SetRxConfig(MODEM_LORA, LORA_BANDWIDTH, LORA_SPREADING_FACTOR,
		  LORA_CODINGRATE, 0, LORA_PREAMBLE_LENGTH,
		  LORA_SYMBOL_TIMEOUT, LORA_FIX_LENGTH_PAYLOAD_ON,
		  0, true, 0, 0, LORA_IQ_INVERSION_ON, true);

  Radio.SetMaxPayloadLength(MODEM_LORA, MAX_APP_BUFFER_SIZE);

#elif ((USE_MODEM_LORA == 0) && (USE_MODEM_FSK == 1))
  APP_LOG(TS_OFF, VLEVEL_M, "---------------\n\r");
  APP_LOG(TS_OFF, VLEVEL_M, "FSK_MODULATION\n\r");
  APP_LOG(TS_OFF, VLEVEL_M, "FSK_BW=%d Hz\n\r", FSK_BANDWIDTH);
  APP_LOG(TS_OFF, VLEVEL_M, "FSK_DR=%d bits/s\n\r", FSK_DATARATE);

  Radio.SetTxConfig(MODEM_FSK, TX_OUTPUT_POWER, FSK_FDEV, 0,
		  FSK_DATARATE, 0,
		  FSK_PREAMBLE_LENGTH, FSK_FIX_LENGTH_PAYLOAD_ON,
		  true, 0, 0, 0, TX_TIMEOUT_VALUE);

  Radio.SetRxConfig(MODEM_FSK, FSK_BANDWIDTH, FSK_DATARATE,
		  0, FSK_AFC_BANDWIDTH, FSK_PREAMBLE_LENGTH,
		  0, FSK_FIX_LENGTH_PAYLOAD_ON, 0, true,
		  0, 0, false, true);

  Radio.SetMaxPayloadLength(MODEM_FSK, MAX_APP_BUFFER_SIZE);

#else
#error "Please define a modulation in the subghz_phy_app.h file."
#endif /* USE_MODEM_LORA | USE_MODEM_FSK */

  /* USER CODE END SubghzApp_Init_2 */
}

/* USER CODE BEGIN EF */

void SubghzApp_SetLoRaConfig(int tx_output_power, int spreading_factor)
{
	Radio.SetTxConfig(MODEM_LORA, tx_output_power, 0, LORA_BANDWIDTH,
			spreading_factor, LORA_CODINGRATE,
			LORA_PREAMBLE_LENGTH, LORA_FIX_LENGTH_PAYLOAD_ON,
			true, 0, 0, LORA_IQ_INVERSION_ON, TX_TIMEOUT_VALUE);

	Radio.SetRxConfig(MODEM_LORA, LORA_BANDWIDTH, spreading_factor,
			LORA_CODINGRATE, 0, LORA_PREAMBLE_LENGTH,
			LORA_SYMBOL_TIMEOUT, LORA_FIX_LENGTH_PAYLOAD_ON,
			0, true, 0, 0, LORA_IQ_INVERSION_ON, true);

	Radio.SetMaxPayloadLength(MODEM_LORA, MAX_APP_BUFFER_SIZE);
}

/* USER CODE END EF */

/* Private functions ---------------------------------------------------------*/
static void OnTxDone(void)
{
  /* USER CODE BEGIN OnTxDone */
	debug_print("Tx done callback\r\n");
  /* USER CODE END OnTxDone */
}

static void OnRxDone(uint8_t *payload, uint16_t size, int16_t rssi, int8_t LoraSnr_FskCfo)
{
  /* USER CODE BEGIN OnRxDone */
	//debug_print("Rx done callback\r\n");
	decode_print_packet(payload, size, rssi, LoraSnr_FskCfo);
  /* USER CODE END OnRxDone */
}

static void OnTxTimeout(void)
{
  /* USER CODE BEGIN OnTxTimeout */
	debug_print("Tx timeout callback\r\n");
  /* USER CODE END OnTxTimeout */
}

static void OnRxTimeout(void)
{
  /* USER CODE BEGIN OnRxTimeout */
	debug_print("Rx timeout callback\r\n");
  /* USER CODE END OnRxTimeout */
}

static void OnRxError(void)
{
  /* USER CODE BEGIN OnRxError */
	debug_print("Rx error callback\r\n");
  /* USER CODE END OnRxError */
}

/* USER CODE BEGIN PrFD */

/* USER CODE END PrFD */
