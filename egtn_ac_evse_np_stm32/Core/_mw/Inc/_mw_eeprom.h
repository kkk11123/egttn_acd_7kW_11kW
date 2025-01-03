/*
 * _mw_eeprom.h
 *
 *  Created on: Jan 6, 2023
 *      Author: MSI
 */

#ifndef MW_INC__MW_EEPROM_H_
#define MW_INC__MW_EEPROM_H_


#include "main.h"
#include "i2c.h"
#include <stdbool.h>
#include <stdint.h>
#include <stddef.h>

#define		_EEPROM_SIZE_KBIT							4
#define		_EEPROM_I2C   								hi2c1
#define		_EEPROM_USE_FREERTOS          				0
#define		_EEPROM_ADDRESS               				0xA0
#define		_EEPROM_USE_WP_PIN            				1

#if (_EEPROM_USE_WP_PIN==1)
#define		_EEPROM_WP_GPIO								EEPROM_nWP_GPIO_Port
#define		_EEPROM_WP_PIN								EEPROM_nWP_Pin
#endif

//#define I2C_MEMADD_SIZE_8BIT							1
//#define I2C_MEMADD_SIZE_16BIT							2

uint8_t ee24_i2c_mem_write(I2C_TypeDef * base, uint16_t DevAddress, uint16_t MemAddress, uint16_t MemAddSize, uint8_t *pData, uint16_t Size, uint32_t Timeout);
uint8_t ee24_i2c_mem_read(I2C_TypeDef * base, uint16_t DevAddress, uint16_t MemAddress, uint16_t MemAddSize, uint8_t *pData, uint16_t Size, uint32_t Timeout);

bool    ee24_isConnected(void);
bool    ee24_write(uint16_t address, uint8_t *data, size_t lenInBytes, uint32_t timeout);
bool    ee24_read(uint16_t address, uint8_t *data, size_t lenInBytes, uint32_t timeout);
bool    ee24_eraseChip(void);

#endif /* MW_INC__MW_EEPROM_H_ */
