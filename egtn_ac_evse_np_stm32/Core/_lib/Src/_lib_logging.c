/*
 * _lib_logging.c
 *
 *  Created on: Jan 6, 2023
 *      Author: MSI
 */




#include <_lib_logging.h>

/**
  * @name  		_write
  * @brief 		Syscall weak function for using printf in Truestudio.
  * 			The function prototype is in syscalls.c
  *
  * @retval     None
  */
int _write(int file, char *ptr, int len)
{
	int DataIdx;

#if ((__DEBUG_PRINTF__) == (_DEBUG__NON_))
	return len;
#endif

	for (DataIdx = 0; DataIdx < len; DataIdx++)
	{
#if ((__DEBUG_PRINTF__) == (_DEBUG__ITM_))
		ITM_SendChar((*ptr++));
#elif ((__DEBUG_PRINTF__) == (_DEBUG__UART_))
		/* Write a function that transmits 1 byte using the corresponding Uart channel. */
		//HAL_UART_Transmit(&huart1, (uint8_t *)(ptr++), 1, 5);
		HAL_UART_Transmit(&huart2, (uint8_t *)(ptr++), 1, 5);
#endif
	}

	return len;
}



