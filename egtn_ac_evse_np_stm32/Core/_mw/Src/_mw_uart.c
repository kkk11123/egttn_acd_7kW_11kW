/*
 * _mw_uart.c
 *
 *  Created on: Jan 6, 2023
 *      Author: MSI
 */


#include <_mw_uart.h>

void _MW_UART_tx01(uint8_t* tx_b, size_t tx_cnt){
	HAL_UART_Transmit(&huart1, tx_b, tx_cnt, 3000);
}

void _MW_UART_tx02(uint8_t* tx_b, size_t tx_cnt){
	HAL_UART_Transmit(&huart2, tx_b, tx_cnt, 3000);
}
