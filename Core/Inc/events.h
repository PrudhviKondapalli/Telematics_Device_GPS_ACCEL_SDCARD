/**
 * @file events.h
 * @brief Handles event analysis and file operations for storing data.
 * @author Sonal Tamrakar, Prudhvi Kondapalli
 * @date 12/16/2023
 */

#ifndef INC_EVENTS_H_
#define INC_EVENTS_H_

/**
 * Default Libraries allowed to be used
 */
#include <stdint.h>

/**
 * User defined functions
 */
void event_analysis(int16_t *xbuffer, int16_t *ybuffer, int16_t *zbuffer);

void buf_analysis(int16_t input_buffer[], int index);

void goToAscii(int16_t number, char* resBuf);

#endif /* INC_EVENTS_H_ */
