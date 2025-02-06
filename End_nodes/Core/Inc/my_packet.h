/*
 * my_packet.h
 *
 *  Created on: 19 Mar 2023
 *      Author: brock
 *
 *
 * ***** PACKET FORMAT -- 15 BYTES or 11 BYTES if no gas res *****
 *
 *  -------------------------------------------------
 * | 1 |   2  |      4     |      4     |     4      |
 *  -------------------------------------------------
 * |ID | TEMP |  PRESSURE  |  HUMIDITY  |  GAS RES   |
 *  -------------------------------------------------
 *
 */

#ifndef INC_MY_PACKET_H_
#define INC_MY_PACKET_H_


#include "bme68x_defs.h"
#include "my_sensor.h"

#define SENDER_ID					1	// on one bytes : 0 to 255

#define PAYLOAD_MAX_LENGTH			64  // used when USE_MAX_LENGTH is 1 or when send_dummy_packet called
#define USE_MAX_LENGTH				0 	// if 1, fills the whole buffer with dummy payload
#define PACKET_LENGTH				SENSE_GAS ? 15 : 11 	// in bytes, for decoding

#define PRINT_PACKET_DECODED		1	// if 1 print the numbers in addition than in hex format

#define SYNC_PACKET_NUMBER			6		// from 1 to that number
#define SYNC_PACKET_DELAY			RTC_DELAY/SYNC_PACKET_NUMBER	// in ms, between two sync packets

#if(SEND_SYNC_PACKETS)
void send_sync_packet(void);
#endif

void make_packet(struct bme68x_data *bme_data);
void send_packet(void);
void send_dummy_packet(void);
void decode_print_packet(uint8_t *payload, uint16_t size, int16_t rssi, int8_t LoraSnr_FskCfo);

#endif /* INC_MY_PACKET_H_ */
