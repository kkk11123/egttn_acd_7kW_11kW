/*
 * _lib_LPF.h
 *
 *  Created on: Oct 30, 2023
 *      Author: MSI
 */

#ifndef LIB_INC__LIB_LPF_H_
#define LIB_INC__LIB_LPF_H_

//#include <iostream.h>
#include "main.h"

typedef struct s_egtn_lib_dimming
{
	uint16_t timeinterval;
	uint16_t tau;
	uint32_t value_bak;
	uint32_t value;
}s_LIB_LPF;

uint8_t _LIB_LPF_init(s_LIB_LPF* p_lpf, uint16_t timeinterval, uint16_t tau);
uint32_t _LIB_LPF_calc(s_LIB_LPF* p_lpf, uint32_t input_value);



#endif /* LIB_INC__LIB_LPF_H_ */
