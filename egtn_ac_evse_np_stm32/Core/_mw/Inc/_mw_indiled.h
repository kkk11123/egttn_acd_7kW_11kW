/*
 * _mw_indiled.h
 *
 *  Created on: Jan 6, 2023
 *      Author: MSI
 */

#ifndef MW_INC__MW_INDILED_H_
#define MW_INC__MW_INDILED_H_


#include "main.h"

typedef enum e__front_indi_led_color{
	BLACK,	//LED ALL OFF
	RED,
	GREEN,
	BLUE,
	YELLOW, // RED + GREEN
	SKY,	// GREEN + BLUE
	PURPLE, // BLUE + RED
	WHITE	// RED + GREEN + BLUE
}e__INDI_LED_COLOR;


void _MW_INDILED_init();
uint8_t _MW_INDILED_sled_ctl(e__INDI_LED_COLOR color);


#endif /* MW_INC__MW_INDILED_H_ */
