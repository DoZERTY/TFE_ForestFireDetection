/*
 * my_packet.c
 *
 *  Created on: 19 Mar 2023
 *      Author: brock
 */

#include "my_packet.h"
#include "bme68x_defs.h"
#include "radio.h"
#include "main.h"
#include "sys_app.h"
#include "rtc.h"
//#include "radio_driver.c"

uint8_t packet_buffer[PAYLOAD_MAX_LENGTH];
uint8_t packet_length;
uint32_t mask = 0x000000FF;

#if(SEND_SYNC_PACKETS)
uint8_t sync_num = 1;
#endif

void make_packet(struct bme68x_data *bme_data)
{
	packet_length = 0;
	packet_buffer[packet_length++] = (uint8_t) SENDER_ID;

	packet_buffer[packet_length++] = (uint8_t) ((bme_data->temperature >> 8) & mask);
	packet_buffer[packet_length++] = (uint8_t) (bme_data->temperature & mask);

	packet_buffer[packet_length++] = (uint8_t) ((bme_data->pressure >> 24) & mask);
	packet_buffer[packet_length++] = (uint8_t) ((bme_data->pressure >> 16) & mask);
	packet_buffer[packet_length++] = (uint8_t) ((bme_data->pressure >> 8) & mask);
	packet_buffer[packet_length++] = (uint8_t) (bme_data->pressure & mask);

	packet_buffer[packet_length++] = (uint8_t) ((bme_data->humidity >> 24) & mask);
	packet_buffer[packet_length++] = (uint8_t) ((bme_data->humidity >> 16) & mask);
	packet_buffer[packet_length++] = (uint8_t) ((bme_data->humidity >> 8) & mask);
	packet_buffer[packet_length++] = (uint8_t) (bme_data->humidity & mask);

#if(SENSE_GAS)
	packet_buffer[packet_length++] = (uint8_t) ((bme_data->gas_resistance >> 24) & mask);
	packet_buffer[packet_length++] = (uint8_t) ((bme_data->gas_resistance >> 16) & mask);
	packet_buffer[packet_length++] = (uint8_t) ((bme_data->gas_resistance >> 8) & mask);
	packet_buffer[packet_length++] = (uint8_t) (bme_data->gas_resistance & mask);
#endif

#if(USE_MAX_LENGTH)
	for(; packet_length < PAYLOAD_MAX_LENGTH;) {
		packet_buffer[packet_length++] = 0;
	}
#endif
}

void send_packet()
{
//	APP_PRINTF("Status Radio: %d \r\n", Radio.GetStatus());
//	APP_PRINTF("Sending %d bytes...\r\n", packet_length);
	if(Radio.Send(packet_buffer, packet_length)) {
		debug_print("Send packet failed.\r\n");
	}
//	APP_PRINTF("Status Radio: %d \r\n", Radio.GetStatus());

//	HAL_Delay(100);
//	Radio.Standby();    // Packet not received correctly (just "HDR KO"). With Sleep, I have an CRC error
//	HAL_Delay(100);
//	APP_PRINTF("Status Radio is now: %d \r\n", Radio.GetStatus());
//
//	Radio.Sleep();   //or        Radio.Sleep();
}

void send_dummy_packet()
{
	for(packet_length = 0; packet_length < PAYLOAD_MAX_LENGTH; ++packet_length) {
		packet_buffer[packet_length] = packet_length;
	}
	send_packet();
}

#if(SEND_SYNC_PACKETS)
void send_sync_packet()
{
	if(sync_num == 1) {
		myRTC_start(SYNC_PACKET_DELAY);
	}
	if(sync_num <= SYNC_PACKET_NUMBER) {
		packet_length = 1;
		packet_buffer[0] = sync_num;
		send_packet();
		sync_num++;
	} else {
		sync_num = 1;
		myRTC_stop();
		APP_PRINTF("Radio in RX mode for %ds...\r\n", RXTIMEOUT/1000);
		Radio.Rx(RXTIMEOUT);
	}
}
#endif

void decode_print_packet(uint8_t *payload, uint16_t size, int16_t rssi, int8_t LoraSnr_FskCfo)
{
	float rssi_f = -((float)rssi)/2;
	float snr_f = ((float)LoraSnr_FskCfo)/4;
	APP_PRINTF("PACKET RECEIVED --- RSSI = %d (%d.%d dBm)     SNR = %d (%d.%d dB)     SIZE=%d bytes\n                    PAYLOAD=0x",
			rssi,
			(int)(rssi_f),
			((int)(rssi_f*10))%10,
			LoraSnr_FskCfo,
			(int)(snr_f),
			((int)(snr_f*100))%100,
			size);
	for(uint16_t i = 0; i < size; ++i) {
		APP_PRINTF("%02x", payload[i]);
	}
	APP_PRINTF("\r\n");
#if(PRINT_PACKET_DECODED)
	if(size == PACKET_LENGTH) {
		uint8_t id = payload[0];
		uint16_t temp = (uint16_t) ((payload[1] << 8) | payload[2]);
		uint32_t press = (uint32_t) ((payload[3] << 24) | (payload[4] << 16) | (payload[5] << 8) | payload[6]);
		uint32_t hum = (uint32_t) ((payload[7] << 24) | (payload[8] << 16) | (payload[9] << 8) | payload[10]);
		if(SENSE_GAS){
			uint32_t gas = (uint32_t) ((payload[11] << 24) | (payload[12] << 16) | (payload[13] << 8) | payload[14]);
			APP_PRINTF("ID\t\tT [C*100]\t P [Pa]\t H [%%*1000]\t Gas R [ohm]\r\n")
			APP_PRINTF("%u\t\t%d\t\t %u\t\t %u\t\t %u\r\n", id, (int) temp, press, hum, gas);
		} else {
			APP_PRINTF("ID\t\tT [C*100]\t P [Pa]\t H [%%*1000]\r\n");
			APP_PRINTF("%u\t\t%d\t\t %u\t\t %u\r\n", id, (int) temp, press, hum);
		}
	}
#endif
#if(RECV_SYNC_PACKETS)
	if(size == 1 && payload[0] == SENDER_ID) {
		myRTC_start(RTC_DELAY);
		HAL_GPIO_WritePin(GPIOA, GPIO_PIN_5, GPIO_PIN_RESET);
	}
#endif
#if(SEND_SYNC_PACKETS)
	if(payload[0] == 0x06) {
		int sf = change_SF();
		APP_PRINTF("Changed LoRa config : SF=%d\r\n", sf);
	}
#endif
}

