/*
 * _mw_gpio.c
 *
 *  Created on: Jan 6, 2023
 *      Author: MSI
 */




#include <_mw_gpio.h>
#include <_lib_debouncecheck.h>
#include <_lib_logging.h>
#include <_config.h>



static u__GPIO_OUT _GPIO_out;
static u__GPIO_IN _GPIO_in;

static uint8_t WELD_DebounceCheckBuf[GPI_DEBOUNCECHECKBUF_LENGTH] = {0, };
static uint8_t EMG_DebounceCheckBuf[GPI_DEBOUNCECHECKBUF_LENGTH] = {0, };

void _MW_GPIO_init()
{
	GPO_MC_TURN_ON_DISABLE;
	GPO_MC_TURN_OFF_DISABLE;
	GPO_STUS_LED_01_ON;
}

void update_gpo(){
	(_GPIO_out.bit.STUS_LED_01) ? GPO_STUS_LED_01_OFF : GPO_STUS_LED_01_ON;
	(_GPIO_out.bit.STUS_LED_02) ? GPO_STUS_LED_02_OFF : GPO_STUS_LED_02_ON;
	(_GPIO_out.bit.SLED_R) ? GPO_SLED_R_ON : GPO_SLED_R_OFF;
	(_GPIO_out.bit.SLED_G) ? GPO_SLED_G_ON : GPO_SLED_G_OFF;
	(_GPIO_out.bit.SLED_B) ? GPO_SLED_B_ON : GPO_SLED_B_OFF;
	(_GPIO_out.bit.MC_TURN_ON) ? GPO_MC_TURN_ON_ENABLE : GPO_MC_TURN_ON_DISABLE;
	(_GPIO_out.bit.MC_TURN_OFF) ? GPO_MC_TURN_OFF_ENABLE : GPO_MC_TURN_OFF_DISABLE;
	(_GPIO_out.bit.CP_RY) ? GPO_CP_RY_ON : GPO_CP_RY_OFF;
}

void update_gpi(){

	_GPIO_in.bit.SET_SW_01 = ~(GPI_SET_SW_01_IN);
	_GPIO_in.bit.SET_SW_02 = ~(GPI_SET_SW_02_IN);
	_GPIO_in.bit.SET_SW_03 = ~(GPI_SET_SW_03_IN);
	_GPIO_in.bit.SET_SW_04 = ~(GPI_SET_SW_04_IN);

	_LIB_DEBOUNCECHECK_shift(WELD_DebounceCheckBuf, GPI_DEBOUNCECHECKBUF_LENGTH);
	_LIB_DEBOUNCECHECK_push(WELD_DebounceCheckBuf, (GPI_WELD_IN));

	if(_LIB_DEBOUNCECHECK_compare(WELD_DebounceCheckBuf, GPI_DEBOUNCECHECKBUF_LENGTH , 1))
	{
		if(_GPIO_in.bit.WELD == 0)
		{
			_LIB_LOGGING_printf("WELD ON\r\n");
			_GPIO_in.bit.WELD = 1;
		}
	}
	else
	{
		if(_GPIO_in.bit.WELD == 1)
		{
			_LIB_LOGGING_printf("WELD OFF\r\n");
			_GPIO_in.bit.WELD = 0;
		}
	}

#if 1
	_LIB_DEBOUNCECHECK_shift(EMG_DebounceCheckBuf, GPI_DEBOUNCECHECKBUF_LENGTH);
	_LIB_DEBOUNCECHECK_push(EMG_DebounceCheckBuf, (GPI_EMG_IN));

	if(_LIB_DEBOUNCECHECK_compare(EMG_DebounceCheckBuf, GPI_DEBOUNCECHECKBUF_LENGTH , 1))
	{
		if(_GPIO_in.bit.EMG == 0)
		{
			_LIB_LOGGING_printf("EMG OFF\r\n");
			_GPIO_in.bit.EMG = 1;
		}
	}
	else
	{
		if(_GPIO_in.bit.EMG == 1)
		{
			_LIB_LOGGING_printf("EMG ON\r\n");
			_GPIO_in.bit.EMG = 0;
		}
	}
#endif

}

uint8_t _MW_GPIO_set_gpo(uint8_t gpo_num, uint8_t stus){
	switch(gpo_num){
		case STUS_LED_01:
			_GPIO_out.bit.STUS_LED_01 = stus;
			return 1;
		case STUS_LED_02:
			_GPIO_out.bit.STUS_LED_02 = stus;
			return 1;
		case SLED_R:
			_GPIO_out.bit.SLED_R = stus;
			return 1;
		case SLED_G:
			_GPIO_out.bit.SLED_G = stus;
			return 1;
		case SLED_B:
			_GPIO_out.bit.SLED_B = stus;
			return 1;
		case MC_TURN_ON:
			_GPIO_out.bit.MC_TURN_ON = stus;
			return 1;
		case MC_TURN_OFF:
			_GPIO_out.bit.MC_TURN_OFF = stus;
			return 1;
		case CP_RY:
			_GPIO_out.bit.CP_RY = stus;
			return 1;
		default:
			return 0;
	}
}

uint8_t _MW_GPIO_get_gpo(uint8_t gpo_num)
{
	switch(gpo_num){
		case STUS_LED_01:
			return _GPIO_out.bit.STUS_LED_01;
		case STUS_LED_02:
			return _GPIO_out.bit.STUS_LED_02;
		case SLED_R:
			return _GPIO_out.bit.SLED_R;
		case SLED_G:
			return _GPIO_out.bit.SLED_G;
		case SLED_B:
			return _GPIO_out.bit.SLED_B;
		case MC_TURN_ON:
			return _GPIO_out.bit.MC_TURN_ON;
		case MC_TURN_OFF:
			return _GPIO_out.bit.MC_TURN_OFF;
		case CP_RY:
			return _GPIO_out.bit.CP_RY;
		default:
			return 2;
	}
}

uint8_t _MW_GPIO_get_gpi(uint8_t gpi_num){
	switch(gpi_num){
		case SET_SW_01:
			return _GPIO_in.bit.SET_SW_01;
		case SET_SW_02:
			return _GPIO_in.bit.SET_SW_02;
		case SET_SW_03:
			return _GPIO_in.bit.SET_SW_03;
		case SET_SW_04:
			return _GPIO_in.bit.SET_SW_04;
		case WELD:
			return _GPIO_in.bit.WELD;
		case EMG:
			return _GPIO_in.bit.EMG;
		default:
			return 2;
	}
}

void _MW_GPIO_update(){
	update_gpo();
	update_gpi();
}


