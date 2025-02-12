/*
 * utils.c
 *
 *  Created on: Feb 12, 2025
 *      Author: Dorian
 */
#include "main.h"
#include <stdlib.h>
#include "string.h"
#include "sys_app.h"


#if (PERF_COUNT == 1)

volatile uint8_t counting_cycles = 0;

void start_cycle_count() {
	uint32_t prim = __get_PRIMASK();
	__disable_irq();
	if (counting_cycles) {
		debug_print("Tried re-entrant cycle counting.\r\n");
		Error_Handler();
	} else {
		counting_cycles = 1;
	}
	if (!prim) {
		__enable_irq();
	}
	DWT->CTRL |= 1 ; // enable the counter
	DWT->CYCCNT = 0; // reset the counter
}
void stop_cycle_count(char *s) {
	uint32_t res = DWT->CYCCNT;
	counting_cycles = 0;
	APP_PRINTF("[PERF] ");
	APP_PRINTF(s);
	APP_PRINTF(" %d cycles.\r\n", (int) res);
}

#else

void start_cycle_count() {}
void stop_cycle_count(char *s) {}

#endif // PERF_COUNT


