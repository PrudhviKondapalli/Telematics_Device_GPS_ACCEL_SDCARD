/**
 * @file i2c.h
 * @brief I2C communication implementation file.
 * @author Sonal Tamrakar, Prudhvi Kondapalli
 * @date 12/16/2023
 */

#ifndef INC_I2C_H_
#define INC_I2C_H_

/**
 * User defined Macros
 */
#define APB1_I2C3_EN			(1<<23)
#define AHB1_GPIOA_EN			(1<<0)
#define AHB1_GPIOC_EN			(1<<2)
#define GPIOA_PA8_ALT			(2<<16)
#define GPIOC_PC9_ALT			(2<<18)
#define GPIOA_PA8_OD			(1<<8)
#define GPIOC_PC9_OD			(1<<9)
#define GPIOA_PA8_HIGH_SPEED	(3<<16)
#define GPIOC_PC9_HIGH_SPEED	(3<<18)
#define GPIOA_PA8_PULL_UP		(1<<16)
#define GPIOC_PC9_PULL_UP		(1<<18)
#define GPIOA_PA8_AFR_I2C3		(4<<0)
#define GPIOC_PC9_AFR_I2C3		(4<<4)
#define I2C3_SWRESET_SET		(1<<15)
#define I2C3_PERIPH_FREQ		(24<<0)
#define I2C3_CCR				(120<<0)
#define I2C3_TRISE				(25)
#define I2C3_ENABLE				(1<<0)
#define ACK_ENABLE				(1<<10)
#define START_GEN				(1<<8)
#define START_BIT				(1<<0)
#define TXE_BIT					(1<<7)
#define BTF_BIT					(1<<2)
#define ADDR_BIT				(1<<1)
#define STOP_BIT				(1<<9)
#define RXNE_BIT				(1<<6)

/**
 * User defined functions
 */
void I2C_Read (uint8_t Address, uint8_t *buffer, uint8_t size);
void I2C_Stop (void);
void I2C_Address (uint8_t Address);
void I2C_Write (uint8_t data);
void I2C_Start (void);
void I2C_Config (void);
void MPU_Write (uint8_t Address, uint8_t Reg, uint8_t Data);
void MPU_Read (uint8_t Address, uint8_t Reg, uint8_t *buffer, uint8_t size);

#endif /* INC_I2C_H_ */
