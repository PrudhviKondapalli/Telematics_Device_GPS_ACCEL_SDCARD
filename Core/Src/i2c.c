/**
 * @file i2c.c
 * @brief I2C communication implementation file.
 * @author Sonal Tamrakar, Prudhvi Kondapalli
 * @date 12/16/2023
 * @credit Thanks to ControllersTech for the informative tutorial on I2C using STM32 family.
 * @leveraged code The I2C functions were adapted off the below link.
 * @link: https://www.youtube.com/watch?v=xxphp9wDnHA&ab_channel=ControllersTech
 */

/**
 * Default Libraries allowed to be used and user defined libraries
 */
#include "main.h"
#include "i2c.h"

/**
 * @brief Configures the I2C peripheral and associated GPIO pins.
 * @details Enables the I2C CLOCK and GPIO CLOCK, configures I2C3-SDA and I2C3-SCL,
 * resets I2C3, and sets the peripheral frequency and other configurations.
 * @param None
 * @return None
 */
void I2C_Config (void)
{
	// Enable the I2C CLOCK and GPIO CLOCK
	//I2C3-SDA is PC9 and I2C3-SCL is PA8
	RCC->APB1ENR |= APB1_I2C3_EN;  // enable I2C3 CLOCK
	RCC->AHB1ENR |= AHB1_GPIOA_EN | AHB1_GPIOC_EN;  // Enable GPIO A + C Clocks

	// Configure the I2C3 PINs for ALternate Functions (MODER)
	GPIOA->MODER |= GPIOA_PA8_ALT;
	GPIOC->MODER |= GPIOC_PC9_ALT;

	//Open Drain Configuration in port output type register
	GPIOA->OTYPER |= GPIOA_PA8_OD;
	GPIOC->OTYPER |= GPIOC_PC9_OD;

	//High Speed Select in Output Speed register
	GPIOA->OSPEEDR |= GPIOA_PA8_HIGH_SPEED;
	GPIOC->OSPEEDR |= GPIOC_PC9_HIGH_SPEED;

	//Pull-Up select in PUPDR register
	GPIOA->PUPDR |= GPIOA_PA8_PULL_UP;
	GPIOC->PUPDR |= GPIOC_PC9_PULL_UP;

	//Alternate function select AF4 for I2C3
	GPIOA->AFR[1] = GPIOA_PA8_AFR_I2C3;
	GPIOC->AFR[1] = GPIOC_PC9_AFR_I2C3;


	// Reset the I2C3 using SWRESET
	I2C3->CR1 |= I2C3_SWRESET_SET;
	I2C3->CR1 &= ~(I2C3_SWRESET_SET);

	///Because APB1 is @ 24Mhz, set peripheral also to 24 Mhz
	I2C3->CR2 |= I2C3_PERIPH_FREQ;

	//Calculation needed here
	/* Configure the clock control registers:
	 * This value is determined by taking the (SCL high time
	 * + SDA/SCL rise time) / APB1 Bus Frequency
	 * APB1 Bus Frequency: 24 Mhz
	*/
	I2C3->CCR = I2C3_CCR;

	// Configure the rise time register
	// (Rise Time SCL / Period of APB1 clock) + 1
	I2C3->TRISE = I2C3_TRISE;

	// Program the I2C3_CR1 register to enable the peripheral
	I2C3->CR1 |= I2C3_ENABLE;  // Enable I2C
}

/**
 * @brief Generates the I2C start condition.
 * @details Enables ACK, generates START, and ensures the start bit is generated.
 * @param None
 * @return None
 */
void I2C_Start (void)
{
	I2C3->CR1 |= ACK_ENABLE; //Acknowledge returned after a byte is received
	I2C3->CR1 |= START_GEN;  // Generate START
	while (!(I2C3->SR1 & START_BIT));  //Ensure start bit (condition) generated, read only
}

/**
 * @brief Writes a byte of data on the I2C bus.
 * @details Waits for TXE and BTF bits, then transmits data.
 * @param data: Data to be transmitted.
 * @return None
 */
void I2C_Write (uint8_t data)
{
	while (!(I2C3->SR1 & TXE_BIT));  // while TXE bit is not set, stay here
	I2C3->DR = data;
	while (!(I2C3->SR1 & BTF_BIT));  // while BTF bit is not set, stay here
}

/**
 * @brief Sends the slave address on the I2C bus.
 * @details Sends the address byte and waits for ADDR bit to be set.
 * @param Address: 7-bit slave address.
 * @return None
 */
void I2C_Address (uint8_t Address)
{
	I2C3->DR = Address;  // Send address byte
	while (!(I2C3->SR1 & ADDR_BIT));  // while ADDR bit not set
	uint8_t temp = I2C3->SR1 | I2C3->SR2;  // read SR1 and SR2 to clear the ADDR bit
}

/**
 * @brief Generates the I2C stop condition.
 * @details Generates the I2C stop condition.
 * @param None
 * @return None
 */
void I2C_Stop (void)
{
	I2C3->CR1 |= STOP_BIT;  // Stop I2C
}

/**
 * @brief Reads data from the I2C bus.
 * @details Reads data from the I2C bus based on the specified parameters.
 * @param Address: 7-bit slave address.
 * @param buffer: Pointer to the buffer to store the read data.
 * @param size: Number of bytes to read.
 * @return None
 */
void I2C_Read (uint8_t Address, uint8_t *buffer, uint8_t size)
{
	//If only 1 BYTE needs to be Read
	int remaining = size;
	if (size == 1)
	{
		I2C3->DR = Address;  // Send Address
		while (!(I2C3->SR1 & ADDR_BIT));  // wait for ADDR bit to set

		I2C3->CR1 &= ~ACK_ENABLE;  // clear the ACK bit
		uint8_t temp = I2C3->SR1 | I2C3->SR2;  // Clear ADDR by reading SR1 and SR2
		I2C3->CR1 |= STOP_BIT;  // Stop I2C
		while (!(I2C3->SR1 & RXNE_BIT));  // Ensure RxNE is set
		buffer[size-remaining] = I2C3->DR;  // Read data and store into buffer
	}
	//If Multiple BYTES needs to be read
	else
	{
		I2C3->DR = Address;  // Send Address
		while (!(I2C3->SR1 & ADDR_BIT));  // wait for ADDR bit to set
		uint8_t temp = I2C3->SR1 | I2C3->SR2;  // read SR1 and SR2 to clear the ADDR bit

		while (remaining > 2)
		{
			while (!(I2C3->SR1 & RXNE_BIT));  // wait for RxNE to set
			buffer[size-remaining] = I2C3->DR;  // Store data from DR into buffer
			I2C3->CR1 |= ACK_ENABLE;  // Send ACK
			remaining--;
		}
		while (!(I2C3->SR1 & RXNE_BIT));  // wait for RxNE to set
		buffer[size-remaining] = I2C3->DR;
		I2C3->CR1 &= ~ACK_ENABLE;  // Clear ACK
		I2C3->CR1 |= STOP_BIT;  // Stop I2C
		remaining--;
		while (!(I2C3->SR1 & RXNE_BIT));  // wait for RxNE to set
		buffer[size-remaining] = I2C3->DR;  // copy the data into the buffer
	}
}

/**
 * @brief Writes data to the MPU6050 sensor.
 * @details Initiates communication with the MPU6050 sensor and writes data to the specified register.
 * @param Address: 7-bit slave address of the MPU6050 sensor.
 * @param Reg: Register address to write data to.
 * @param Data: Data byte to be written to the register.
 * @return None
 */
void MPU_Write (uint8_t Address, uint8_t Reg, uint8_t Data)
{
	I2C_Start();
	I2C_Address (Address);
	I2C_Write (Reg);
	I2C_Write (Data);
	I2C_Stop ();
}

/**
 * @brief Reads data from the MPU6050 sensor.
 * @details Initiates communication with the MPU6050 sensor, writes the register address,
 * generates a repeated start condition, and reads the specified number of bytes.
 * @param Address: 7-bit slave address of the MPU6050 sensor.
 * @param Reg: Register address to read data from.
 * @param buffer: Pointer to the buffer to store the read data.
 * @param size: Number of bytes to read.
 * @return None
 */
void MPU_Read (uint8_t Address, uint8_t Reg, uint8_t *buffer, uint8_t size)
{
	I2C_Start ();
	I2C_Address (Address);
	I2C_Write (Reg);
	I2C_Start ();
	I2C_Read (Address+0x01, buffer, size);
	I2C_Stop ();
}
