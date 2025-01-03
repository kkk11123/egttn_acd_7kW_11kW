/*
 * _mw_uart.h
 *
 *  Created on: Jan 6, 2023
 *      Author: MSI
 */

#ifndef MW_INC__MW_UART_H_
#define MW_INC__MW_UART_H_

#include "main.h"
#include "usart.h"

void _MW_UART_tx01(uint8_t* tx_b, size_t tx_cnt);
void _MW_UART_tx02(uint8_t* tx_b, size_t tx_cnt);

#endif /* MW_INC__MW_UART_H_ */
