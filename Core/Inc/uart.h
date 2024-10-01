/**
 * @file uart.h
 * @brief Handles UART module and UART handler.
 * @author Sonal Tamrakar, Prudhvi Kondapalli
 * @date 12/16/2023
 */

#ifndef INC_UART_H_
#define INC_UART_H_

/**
 * User defined functions
 */
void Uart2Config (void);

void UART2_SendChar (char c);

char UART2_GetChar (void);

void UART2_SendString (char *string);

void usart2_call(void);

#endif /* INC_UART_H_ */
