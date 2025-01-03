/*
 * _lib_debouncecheck.h
 *
 *  Created on: Jan 6, 2023
 *      Author: MSI
 */

#ifndef LIB_INC__LIB_DEBOUNCECHECK_H_
#define LIB_INC__LIB_DEBOUNCECHECK_H_


#include "main.h"

void _LIB_DEBOUNCECHECK_init(uint8_t* dbuf, uint16_t dbuflength);
void _LIB_DEBOUNCECHECK_push(uint8_t* dbuf, int pdata);
void _LIB_DEBOUNCECHECK_shift(uint8_t* dbuf, uint16_t dbuflength);
uint8_t _LIB_DEBOUNCECHECK_compare(uint8_t* dbuf, uint16_t dbuflength, uint8_t value);



#endif /* LIB_INC__LIB_DEBOUNCECHECK_H_ */
