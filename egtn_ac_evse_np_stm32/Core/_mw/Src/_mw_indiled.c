/*
 * _mw_indiled.c
 *
 *  Created on: Jan 6, 2023
 *      Author: MSI
 */



#include <_mw_indiled.h>
#include <_mw_gpio.h>
#include <_config.h>

void _MW_INDILED_init()
{
	//_MW_INDILED_bdled_ctl(BLACK);
	_MW_INDILED_sled_ctl(BLACK);
}

uint8_t _MW_INDILED_sled_ctl(e__INDI_LED_COLOR color)
{
	switch(color)
	{
		case RED :
			_MW_GPIO_set_gpo(SLED_R, _ON);
			_MW_GPIO_set_gpo(SLED_G, _OFF);
			_MW_GPIO_set_gpo(SLED_B, _OFF);
		break;

		case GREEN :
			_MW_GPIO_set_gpo(SLED_R, _OFF);
			_MW_GPIO_set_gpo(SLED_G, _ON);
			_MW_GPIO_set_gpo(SLED_B, _OFF);
		break;

		case BLUE :
			_MW_GPIO_set_gpo(SLED_R, _OFF);
			_MW_GPIO_set_gpo(SLED_G, _OFF);
			_MW_GPIO_set_gpo(SLED_B, _ON);
		break;

		case YELLOW :
			_MW_GPIO_set_gpo(SLED_R, _ON);
			_MW_GPIO_set_gpo(SLED_G, _ON);
			_MW_GPIO_set_gpo(SLED_B, _OFF);
		break;

		case SKY :
			_MW_GPIO_set_gpo(SLED_R, _OFF);
			_MW_GPIO_set_gpo(SLED_G, _ON);
			_MW_GPIO_set_gpo(SLED_B, _ON);
		break;

		case PURPLE :
			_MW_GPIO_set_gpo(SLED_R, _ON);
			_MW_GPIO_set_gpo(SLED_G, _OFF);
			_MW_GPIO_set_gpo(SLED_B, _ON);
		break;

		case WHITE :
			_MW_GPIO_set_gpo(SLED_R, _ON);
			_MW_GPIO_set_gpo(SLED_G, _ON);
			_MW_GPIO_set_gpo(SLED_B, _ON);
		break;

		case BLACK :
		default :	// Black
			_MW_GPIO_set_gpo(SLED_R, _OFF);
			_MW_GPIO_set_gpo(SLED_G, _OFF);
			_MW_GPIO_set_gpo(SLED_B, _OFF);
		break;
	}

	return _TRUE;
}

