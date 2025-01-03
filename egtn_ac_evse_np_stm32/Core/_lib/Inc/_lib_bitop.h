/*
 * _lib_bitop.h
 *
 *  Created on: Jan 6, 2023
 *      Author: MSI
 */

#ifndef LIB_INC__LIB_BITOP_H_
#define LIB_INC__LIB_BITOP_H_

#include "main.h"

#define _FILED_LSB_		0
#define _FILED_MSB_		1

uint8_t _LIB_BITOP_ext_u16_to_u8one(uint16_t var, uint16_t offset);
uint16_t _LIB_BITOP_combi_u8two_to_u16(uint8_t msb, uint8_t lsb);
short _LIB_BITOP_bcd_to_short(short bcd);
unsigned int _LIB_BITOP_bcd_to_uint(unsigned int bcd);

#endif /* LIB_INC__LIB_BITOP_H_ */
