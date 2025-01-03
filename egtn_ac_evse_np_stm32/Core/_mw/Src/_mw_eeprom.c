/*
 * _mw_eeprom.c
 *
 *  Created on: Jan 6, 2023
 *      Author: MSI
 */

#include <_mw_eeprom.h>

#if (_EEPROM_USE_FREERTOS == 1)
#include "cmsis_os.h"
#define ee24_delay(x)   osDelay(x)
#else
#define ee24_delay(x)   HAL_Delay(x)
#endif

#if (_EEPROM_SIZE_KBIT == 1) || (_EEPROM_SIZE_KBIT == 2)
#define _EEPROM_PSIZE     8
#elif (_EEPROM_SIZE_KBIT == 4) || (_EEPROM_SIZE_KBIT == 8) || (_EEPROM_SIZE_KBIT == 16)
#define _EEPROM_PSIZE     16
#else
#define _EEPROM_PSIZE     32
#endif

static uint8_t ee24_lock = 0;

#if 0
#if 0
uint8_t ee24_i2c_mem_write(I2C_TypeDef * base, uint16_t DevAddress, uint16_t MemAddress, uint16_t MemAddSize, uint8_t *pData, uint16_t Size, uint32_t Timeout)
{
	//while(LL_I2C_IsActiveFlag_BUSY(base));

	LL_I2C_AcknowledgeNextData(base, LL_I2C_ACK);
	LL_I2C_SetSlaveAddr(base, DevAddress);

	LL_I2C_GenerateStartCondition(base);

	//while(!LL_I2C_IsActiveFlag_SB(base));
	//LL_I2C_TransmitData8(base, DevAddress);
	//while(!LL_I2C_IsActiveFlag_TXE(base));
	//while(!LL_I2C_IsActiveFlag_ADDR(base));
	//LL_I2C_ClearFlag_ADDR(base);

	//while(!LL_I2C_IsActiveFlag_TXE(base));
	//LL_I2C_TransmitData8(base, ((MemAddress >> 8) & 0xFF));
	//while(!LL_I2C_IsActiveFlag_TXE(base));
	LL_I2C_TransmitData8(base, (MemAddress & 0xFF));
	while(!LL_I2C_IsActiveFlag_TXE(base));
	for(int i = 0; i< Size; i++)
	{
		LL_I2C_TransmitData8(base, *pData);
		pData++;
		while(!LL_I2C_IsActiveFlag_TXE(base));
	}
	while(!LL_I2C_IsActiveFlag_TC(base));

	LL_I2C_GenerateStopCondition(base);

	return EVON_TRUE;
}
#else

uint8_t ee24_i2c_mem_write(I2C_TypeDef * base, uint16_t DevAddress, uint16_t MemAddress, uint16_t MemAddSize, uint8_t *pData, uint16_t Size, uint32_t Timeout)
{
	while(LL_I2C_IsActiveFlag_BUSY(base));

	LL_I2C_HandleTransfer(base, DevAddress, LL_I2C_ADDRSLAVE_7BIT, Size+1, LL_I2C_MODE_AUTOEND, LL_I2C_GENERATE_START_WRITE);

	while(!LL_I2C_IsActiveFlag_TXE(base));
	LL_I2C_TransmitData8(base,(MemAddress & 0xFF));

	for(int i = 0; i< Size; i++)
	{
		while(!LL_I2C_IsActiveFlag_TXE(base));
		LL_I2C_TransmitData8(base, *pData);
		pData++;
	}

	//while(!LL_I2C_IsActiveFlag_TC(base));

	//LL_I2C_GenerateStopCondition(base);

	return EVON_TRUE;
}
#endif

#if 0
uint8_t ee24_i2c_mem_read(I2C_TypeDef * base, uint16_t DevAddress, uint16_t MemAddress, uint16_t MemAddSize, uint8_t *pData, uint16_t Size, uint32_t Timeout)
{
	while(LL_I2C_IsActiveFlag_BUSY(base));

	LL_I2C_GenerateStartCondition(base);

	LL_I2C_TransmitData8(base, (DevAddress | 0x00));
	while(!LL_I2C_IsActiveFlag_TXE(base));
	while(!LL_I2C_IsActiveFlag_ADDR(base));
	LL_I2C_ClearFlag_ADDR(base);

	//LL_I2C_TransmitData8(base, ((MemAddress >> 8) & 0xFF));
	//while(!LL_I2C_IsActiveFlag_TXE(base));
	LL_I2C_TransmitData8(base, (MemAddress & 0xFF));
	while(!LL_I2C_IsActiveFlag_TXE(base));
	while(!LL_I2C_IsActiveFlag_TC(base));

	LL_I2C_GenerateStartCondition(base);
	LL_I2C_TransmitData8(base, (DevAddress | 0x01));
	while(!LL_I2C_IsActiveFlag_ADDR(base));
	LL_I2C_ClearFlag_ADDR(base);

	for(int i = 0; i< Size; i++)
	{
#if 0
		if(i < Size - 1)
		{
			LL_I2C_AcknowledgeNextData(base, LL_I2C_ACK);
		}
		else
		{
			LL_I2C_AcknowledgeNextData(base, LL_I2C_NACK);
		}
#else
		if(i == (Size - 1))
		{
			LL_I2C_AcknowledgeNextData(base, LL_I2C_NACK);
		}
#endif
		while(!LL_I2C_IsActiveFlag_RXNE(base));
		pData[i] = LL_I2C_ReceiveData8(base);
	}

	LL_I2C_GenerateStopCondition(base);
}
#else
uint8_t ee24_i2c_mem_read(I2C_TypeDef * base, uint16_t DevAddress, uint16_t MemAddress, uint16_t MemAddSize, uint8_t *pData, uint16_t Size, uint32_t Timeout)
{
	while(LL_I2C_IsActiveFlag_BUSY(base));

	LL_I2C_HandleTransfer(base, DevAddress, LL_I2C_ADDRSLAVE_7BIT, 1, LL_I2C_MODE_SOFTEND, LL_I2C_GENERATE_START_WRITE);

	while(!LL_I2C_IsActiveFlag_TXE(base));
	LL_I2C_TransmitData8(base,(MemAddress & 0xFF));

	while(!LL_I2C_IsActiveFlag_TC(base));
	LL_I2C_GenerateStopCondition(base);

	LL_I2C_HandleTransfer(base, DevAddress, LL_I2C_ADDRSLAVE_7BIT, Size, LL_I2C_MODE_AUTOEND, LL_I2C_GENERATE_RESTART_7BIT_READ);

	//while(!LL_I2C_IsActiveFlag_TXE(base));
	//LL_I2C_TransmitData8(base,(MemAddress & 0xFF));

	for(int i = 0; i< Size; i++)
	{
		while(!LL_I2C_IsActiveFlag_RXNE(base));
		pData[i] = LL_I2C_ReceiveData8(base);
	}

	return EVON_TRUE;
}
#endif
#endif
//################################################################################################################
#if 0
bool ee24_isConnected(void)
{
#if (_EEPROM_USE_WP_PIN==1)
  HAL_GPIO_WritePin(_EEPROM_WP_GPIO,_EEPROM_WP_PIN,GPIO_PIN_SET);
#endif
  if (HAL_I2C_IsDeviceReady(&_EEPROM_I2C, _EEPROM_ADDRESS, 2, 100)==HAL_OK)
    return true;
  else
    return false;
}
#endif
//################################################################################################################
bool ee24_write(uint16_t address, uint8_t *data, size_t len, uint32_t timeout)
{
  if (ee24_lock == 1)
    return false;
  ee24_lock = 1;
  uint16_t w;
  //uint32_t startTime = HAL_GetTick();
#if	(_EEPROM_USE_WP_PIN==1)
  HAL_GPIO_WritePin(_EEPROM_WP_GPIO, _EEPROM_WP_PIN, GPIO_PIN_RESET);
#endif
  while (1)
  {
    w = _EEPROM_PSIZE - (address  % _EEPROM_PSIZE);
    if (w > len)
      w = len;
#if ((_EEPROM_SIZE_KBIT==1) || (_EEPROM_SIZE_KBIT==2))
    if (HAL_I2C_Mem_Write(&_EEPROM_I2C, _EEPROM_ADDRESS, address, I2C_MEMADD_SIZE_8BIT, data, w, 100) == HAL_OK)
#elif (_EEPROM_SIZE_KBIT==4)
    if (HAL_I2C_Mem_Write(&_EEPROM_I2C, _EEPROM_ADDRESS | ((address & 0x0100) >> 7), (address & 0xff), I2C_MEMADD_SIZE_8BIT, data, w, 100) == HAL_OK)
#elif (_EEPROM_SIZE_KBIT==8)
    if (HAL_I2C_Mem_Write(&_EEPROM_I2C, _EEPROM_ADDRESS | ((address & 0x0300) >> 7), (address & 0xff), I2C_MEMADD_SIZE_8BIT, data, w, 100) == HAL_OK)
#elif (_EEPROM_SIZE_KBIT==16)
    if (HAL_I2C_Mem_Write(&_EEPROM_I2C, _EEPROM_ADDRESS | ((address & 0x0700) >> 7), (address & 0xff), I2C_MEMADD_SIZE_8BIT, data, w, 100) == HAL_OK)
#else
    if (HAL_I2C_Mem_Write(&_EEPROM_I2C, _EEPROM_ADDRESS, address, I2C_MEMADD_SIZE_16BIT, data, w, 100) == EVON_TRUE)
#endif
    {
      //ee24_delay(10);
    	ee24_delay(6);
      len -= w;
      data += w;
      address += w;
      if (len == 0)
      {
        #if (_EEPROM_USE_WP_PIN==1)
    	  HAL_GPIO_WritePin(_EEPROM_WP_GPIO, _EEPROM_WP_PIN, GPIO_PIN_SET);
        #endif
        ee24_lock = 0;
        return true;
      }
      //if (HAL_GetTick() - startTime >= timeout)
      //{
      //  ee24_lock = 0;
        //return false;
      //}
    }
    else
    {
#if (_EEPROM_USE_WP_PIN==1)
    	HAL_GPIO_WritePin(_EEPROM_WP_GPIO, _EEPROM_WP_PIN, GPIO_PIN_SET);
#endif
      ee24_lock = 0;
      return false;
    }
  }
}
//################################################################################################################
bool ee24_read(uint16_t address, uint8_t *data, size_t len, uint32_t timeout)
{
  if (ee24_lock == 1)
    return false;
  ee24_lock = 1;
#if (_EEPROM_USE_WP_PIN==1)
  HAL_GPIO_WritePin(_EEPROM_WP_GPIO, _EEPROM_WP_PIN, GPIO_PIN_RESET);
#endif
#if ((_EEPROM_SIZE_KBIT==1) || (_EEPROM_SIZE_KBIT==2))
  if (HAL_I2C_Mem_Read(&_EEPROM_I2C, _EEPROM_ADDRESS, address, I2C_MEMADD_SIZE_8BIT, data, len, 100) == HAL_OK)
#elif (_EEPROM_SIZE_KBIT == 4)
  if (HAL_I2C_Mem_Read(&_EEPROM_I2C, _EEPROM_ADDRESS | ((address & 0x0100) >> 7), (address & 0xff), I2C_MEMADD_SIZE_8BIT, data, len, 100) == HAL_OK)
#elif (_EEPROM_SIZE_KBIT == 8)
  if (HAL_I2C_Mem_Read(&_EEPROM_I2C, _EEPROM_ADDRESS | ((address & 0x0300) >> 7), (address & 0xff), I2C_MEMADD_SIZE_8BIT, data, len, 100) == HAL_OK)
#elif (_EEPROM_SIZE_KBIT==16)
  if (HAL_I2C_Mem_Read(&_EEPROM_I2C, _EEPROM_ADDRESS | ((address & 0x0700) >> 7), (address & 0xff), I2C_MEMADD_SIZE_8BIT, data, len, 100) == HAL_OK)
#else
  if (HAL_I2C_Mem_Read(&_EEPROM_I2C, _EEPROM_ADDRESS, address, I2C_MEMADD_SIZE_16BIT, data, len, timeout) == HAL_OK)
#endif
  {
    ee24_lock = 0;
    return true;
  }
  else
  {
    ee24_lock = 0;
    return false;
  }
}
//################################################################################################################
bool ee24_eraseChip(void)
{
  const uint8_t eraseData[32] = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF\
    , 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF};
  uint32_t bytes = 0;
  while ( bytes < (_EEPROM_SIZE_KBIT * 256))
  {
    if (ee24_write(bytes, (uint8_t*)eraseData, sizeof(eraseData), 100) == false)
      return false;
    bytes += sizeof(eraseData);
  }
  return true;
}
//################################################################################################################


