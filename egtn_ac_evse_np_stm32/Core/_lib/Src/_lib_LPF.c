/*
 * _lib_LPF.c
 *
 *  Created on: Oct 30, 2023
 *      Author: MSI
 */

#include <_lib_LPF.h>

uint8_t _LIB_LPF_init(s_LIB_LPF* p_lpf, uint16_t timeinterval, uint16_t tau)
{
	uint8_t ret_value = 0;

	p_lpf->timeinterval = timeinterval;
	p_lpf->tau = tau;
	p_lpf->value_bak = 0;
	p_lpf->value = 0;

	if(1)
	{
		ret_value = 1;
	}

	return ret_value;
}
uint32_t _LIB_LPF_calc(s_LIB_LPF* p_lpf, uint32_t input_value)
{
	uint32_t output_value = 0;

	if(0 == p_lpf->value_bak)
	{
		p_lpf->value = input_value;
	}
	else
	{
		p_lpf->value = ((p_lpf->tau * p_lpf->value_bak) + (p_lpf->timeinterval * input_value)) / (p_lpf->tau + p_lpf->timeinterval);
	}

	p_lpf->value_bak = p_lpf->value;

	output_value = p_lpf->value;

	return output_value;
}


