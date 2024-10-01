/**
 * @file uart.c
 * @brief Handles UART module and UART handler.
 * @author Sonal Tamrakar, Prudhvi Kondapalli
 * @date 12/16/2023
 */

/**
 * Default Libraries allowed to be used
 */
#include "main.h"
#include "stdint.h"
#include "stdlib.h"
#include "string.h"
#include "math.h"

/**
 * User-defined libraries
 */
#include <parse_NMEA.h>
#include "uart.h"

/**
 * User defined variables
 */
volatile char GPRMC[100] = {0};
volatile char GPVTG[100] = {0};
volatile char GPGGA[100] = {0};
volatile char GPGSA[100] = {0};
volatile char GPGSV1[100] = {0};
volatile char GPGSV2[100] = {0};
volatile char GPGSV3[100] = {0};
volatile char GPGLL[100] = {0};
volatile char* NMEA[8] = {GPRMC, GPVTG, GPGGA, GPGSA, GPGSV1, GPGSV2, GPGSV3, GPGLL};
volatile int32_t buf_count = 0;
volatile uint32_t nmea_count = 0;
volatile uint32_t uart_incr_ticks = 0;
volatile uint8_t even_counter = 0;
extern int16_t systick_count;

/**
 * GPS struct and instance
 */
GPSSTRUCT gnssTransfer;

/**
  * @brief  This function is executed to initialize UART module, sets BAUD rate and enables the receiver and transmitter bit
  * @retval None
  */
void Uart2Config (void)
{

	RCC->APB1ENR |= (1<<17);  // Enable UART2 CLOCK
	RCC->AHB1ENR |= (1<<0); // Enable GPIOA CLOCK

	GPIOA->MODER |= (2<<4);  // Bits (5:4)= 1:0 --> Alternate Function for Pin PA2
	GPIOA->MODER |= (2<<6);  // Bits (7:6)= 1:0 --> Alternate Function for Pin PA3

	GPIOA->OSPEEDR |= (3<<4) | (3<<6);  // Bits (5:4)= 1:1 and Bits (7:6)= 1:1 --> High Speed for PIN PA2 and PA3

	GPIOA->AFR[0] |= (7<<8);  //  AF7 Alternate function for USART2 at Pin PA2
	GPIOA->AFR[0] |= (7<<12); //  AF7 Alternate function for USART2 at Pin PA3

	USART2->CR1 = 0x00;  // clear all
	USART2->CR1 |= (1<<5) | (1<<7);


	// Program the M bit in USART_CR1 to define the word length.
	USART2->CR1 &= ~(1<<12);  // M =0; 8 bit word length

	// Select the desired baud rate using the USART_BRR register.
	USART2->BRR = 0x0341;   // Baud rate of 115200 AT 8MHz

	//Enable the Transmitter/Receiver by Setting the TE and RE bits in USART_CR1 Register
	USART2->CR1 |= (1<<2); // RE=1,Enable the Receiver
	USART2->CR1 |= (1<<3);  // TE=1,Enable Transmitter

	//USART2->CR1 |= (1<<13); //UART ENABLE

	NVIC_SetPriority(USART2_IRQn, 2);
	//NVIC_ClearPending(USART2_IRQn);
	//NVIC_EnableIRQ(USART2_IRQn);
	//NVIC_SetPriority(UART0_IRQn, 2); // could be 0, 1, 2, or 3 priority
	//NVIC_ClearPendingIRQ(UART0_IRQn);
	//NVIC_EnableIRQ(UART0_IRQn);
}

/**
 * @brief Sends a character via UART.
 * @param c The character to be sent.
 * @retval None
 */
void UART2_SendChar (char c)
{
	USART2->DR = c; // load the data into DR register
	while (!(USART2->SR & (1<<6))); //
}

/**
  * @brief  This function is executed to read the data and store it in a variable temp and return
  * @retval Character send via UART
  */
char UART2_GetChar (void)
{
	uint8_t temp;

	while (!(USART2->SR & (1<<5)));  // wait for RXNE bit to set
	temp = USART2->DR;  // Read the data clearing RXNE as well.
	return temp;
}


/**
 * @brief Sends a string to the terminal emulator via UART.
 * @param string Pointer to the string to be sent.
 * @retval None
 */
void UART2_SendString (char *string)
{
	while (*string) UART2_SendChar (*string++);
}

/**
 * @brief Handles the USART2 interrupt, processes received NMEA sentences, and triggers analysis functions.
 * @note This function is invoked by the USART2 interrupt.
 * @retval None
 */
void usart2_call(void)
{
    // Check if we are here because of RXNE interrupt
    if (USART2->SR & USART_SR_RXNE) // If RX is not empty
    {
        // Store received character in the NMEA buffer
        NMEA[nmea_count][buf_count] = (char)USART2->DR;

        // Check if the end of the NMEA sentence is reached
        if (USART2->DR == '\n')
        {
            NMEA[nmea_count][buf_count] = '\0';          // Null-terminate the NMEA sentence
            NMEA[nmea_count][buf_count - 1] = '\0';      // Remove the '\r' character
            buf_count = 0;                               // Reset buffer count for the next sentence
            nmea_count++;                                // Increment NMEA sentence count

            // Check if all expected NMEA sentences are received
            if (nmea_count == 8)
            {
                nmea_count = 0;                       // Reset NMEA sentence count
                uart_incr_ticks++;                    // Increment UART ticks count
                USART2->CR1 &= ~(1 << 13);           // Disable USART2 to stop receiving data
                NVIC_DisableIRQ(USART2_IRQn);        // Disable USART2 interrupt

                // Analyze and process the received sentences based on even/odd counters
                if (even_counter % 2 == 0)
                {
                    GGA_analysis(NMEA[2], &gnssTransfer.GGA);
                    even_counter = 1;
                    systick_count = 0;                // Reset systick count
                }
                else
                {
                    RMC_analysis(NMEA[0], &gnssTransfer.RMC);
                    even_counter = 0;
                    systick_count = 0;                // Reset systick count
                }
                return;
            }
        }
        buf_count++;
    }

    // Check if we are here because of TXEIE interrupt
    if (USART1->SR & USART_SR_TXE) // If TX buffer is empty
    {
        // Handle transmit completion here
    }
}



