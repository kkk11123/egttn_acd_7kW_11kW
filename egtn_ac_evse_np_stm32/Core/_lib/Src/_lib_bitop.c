/*
 * _lib_bitop.c
 *
 *  Created on: Jan 6, 2023
 *      Author: MSI
 */




#include <_lib_bitop.h>

uint8_t _LIB_BITOP_ext_u16_to_u8one(uint16_t var, uint16_t offset)
{
	uint16_t data;
	data = ((var >> (offset * 8)) & 0xff);
	return (uint8_t)data;
}

uint16_t _LIB_BITOP_combi_u8two_to_u16(uint8_t msb, uint8_t lsb)
{
	uint16_t temp;
	uint16_t b1 = (uint16_t)msb;
	uint16_t b2 = (uint16_t)lsb;

	temp = ( (b1 << 8) | b2 );

	return temp;
}

short _LIB_BITOP_bcd_to_short(short bcd)
{
	return bcd ? (_LIB_BITOP_bcd_to_short(bcd >> 4) * 10) + (bcd & 0x0f) : 0;
}

unsigned int _LIB_BITOP_bcd_to_uint(unsigned int bcd)
{
	return bcd ? (_LIB_BITOP_bcd_to_uint(bcd >> 4) * 10) + (bcd & 0x0f) : 0;
}




