/*
 * _mw_ntc.h
 *
 *  Created on: Jan 6, 2023
 *      Author: MSI
 */

#ifndef MW_INC__MW_NTC_H_
#define MW_INC__MW_NTC_H_

#include "main.h"
#include "adc.h"
#include "dma.h"
#include "tim.h"

#define __NTC_DEBUG__			1

#define NTC_TEMP_ADC_LENGTH		10

int _MW_NTC_cal_ntc(uint16_t i_temp);				//TSM1A103F34D1RZ
uint8_t _MW_NTC_store_value(uint16_t ADC_value);
int16_t _MW_NTC_get_temp();
uint8_t _MW_NTC_cal_temp();
uint8_t _MW_NTC_loop();



#endif /* MW_INC__MW_NTC_H_ */
