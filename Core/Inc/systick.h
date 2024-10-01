/**
 * @file systick.h
 * @brief SysTick utility functions implementation.
 * @author Sonal Tamrakar, Prudhvi Kondapalli
 * @date 12/16/2023
 */

#ifndef INC_SYSTICK_H_
#define INC_SYSTICK_H_

/**
 * User-defined libraries
 */
#include <stdint.h>

/**
 * User defined functions
 */
void delay_ms_systick(int ms);

void incr_ticks(void);

void reset_ticks(void);

#endif /* INC_SYSTICK_H_ */
