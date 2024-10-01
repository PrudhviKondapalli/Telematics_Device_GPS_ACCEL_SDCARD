/**
 * @file events.c
 * @brief SysTick utility functions implementation.
 * @author Sonal Tamrakar, Prudhvi Kondapalli
 * @date 12/16/2023
 */

/**
 * Default Libraries allowed to be used
 */
#include "stm32f4xx.h"

/**
 * User-defined libraries
 */
#include "systick.h"

/**
 * User defined variables
 */
volatile uint64_t  ticks = 0; // must be volatile to prevent compiler

/**
 * @brief Increment the SysTick ticks.
 * @param None
 */
void incr_ticks(void)
{
	ticks++;
}

/**
 * @brief Delay for the specified number of milliseconds using SysTick.
 * @param ms: Number of milliseconds to delay.
 */
void delay_ms_systick(int ms)
{
	uint32_t started = ticks;
	while((ticks-started)<=ms); // rollover-safe (within limits)
}

