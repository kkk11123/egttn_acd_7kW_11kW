/*
 * _mw_cp.c
 *
 *  Created on: Jan 6, 2023
 *      Author: MSI
 */


#include <_mw_cp.h>
#include <_mw_pwm.h>
#include <_mw_gpio.h>
#include <_lib_debouncecheck.h>
#include <_lib_logging.h>
#include <_lib_userdelay.h>
#include <_config.h>

static gUserDelay gDelay_mcctl;

static int ctl_value = _OFF;
static int bak_ctl_value = _CONTINUE;

static sCP CP;

/*
 * CP , PD , Board Temp ADC DMA Value
 * index 0 : CP
 * index 1 : PD
 * index 2 : Board Temp
 */
//__IO uint16_t gADCData[MAX_ADC_CH] = {0,};
__IO uint16_t gADCData[MAX_ADC_CH] = {0,};

void _MW_CP_init()
{
	HAL_TIM_OC_Start_IT(&htim1, TIM_CHANNEL_2);
	HAL_TIM_OC_Start_IT(&htim1, TIM_CHANNEL_3);
	HAL_TIM_OC_Start_IT(&htim1, TIM_CHANNEL_4);

	HAL_ADCEx_Calibration_Start(&hadc);
	HAL_ADC_Start_DMA(&hadc, (uint32_t*)gADCData, MAX_ADC_CH);

	_MW_CP_set_pwm_duty(100);
	//CP.PWM_DutyBK = _MW_CP_get_pwm_duty();

	CP.State = default0;
	_LIB_DEBOUNCECHECK_init(CP.DebounceCheckBuf, CP_ADC_DEBOUNCECHECKBUF_LENGTH);

	CP.H.ave_Volt = 0;
	CP.H.sum_Volt = 0;
	CP.H.val_Volt = 0;
	CP.H.val_ADC = 0;
	CP.H.sum_ADC = 0;
	CP.H.ave_ADC = 0;
	CP.H.ave_cnt = 0;
	CP.H.complete_ADC = 0;

	CP.L.ave_Volt = 0;
	CP.L.sum_Volt = 0;
	CP.L.val_Volt = 0;
	CP.L.val_ADC = 0;
	CP.L.sum_ADC = 0;
	CP.L.ave_ADC = 0;
	CP.L.ave_cnt = 0;
	CP.L.complete_ADC = 0;

    CP.State_bk = CP.State;

    _LIB_USERDELAY_set(&gDelay_mcctl, MCCTL_DELAY);
}

uint8_t _MW_CP_change_duty(TIM_HandleTypeDef *htim, uint32_t cp_channel)
{
	double temp_PWM_Duty = _MW_CP_get_pwm_duty();

	if(temp_PWM_Duty != CP.PWM_DutyBK)
	{
		if(0 == _MW_PWM_set_duty(htim, cp_channel, temp_PWM_Duty))
		{
			_LIB_LOGGING_print_file_line("_MW_PWM_ERROR");
		}
		else
		{
			CP.PWM_DutyBK = temp_PWM_Duty;
			return _TRUE;
		}
	}

	return _FALSE;
}

uint8_t _MW_CP_set_pwm_duty(double persent)
{
	if((persent < 0) || (persent > 100))
	{
		return _FALSE;
	}

	CP.PWM_Duty = persent;

	return _TRUE;
}

double _MW_CP_get_pwm_duty()
{
	return CP.PWM_Duty;
}
double _MW_CP_get_h_final_voltage()
{
	return CP.H.ave_Volt;
}
double _MW_CP_get_l_final_voltage()
{
	return CP.L.ave_Volt;
}

uint8_t _MW_CP_cp_relay_ctl(int value)
{
	if(value == _OFF)
	{
		if(_MW_GPIO_get_gpo(CP_RY) == _ON)
		{
			_MW_GPIO_set_gpo(CP_RY, _OFF);
#if ((__CP_DEBUG__)==1)
			_LIB_LOGGING_printf("CP_RELAY : --OFF-- \r\n");
#endif
			return _TRUE;
		}
	}
	else	//value == _ON_
	{
		if(_MW_GPIO_get_gpo(CP_RY) == _OFF)
		{
			_MW_GPIO_set_gpo(CP_RY, _ON);
#if ((__CP_DEBUG__)==1)
			_LIB_LOGGING_printf("CP_RELAY : --ON-- \r\n");
#endif
			return _TRUE;
		}
	}

	return _FALSE;
}

#if 0
uint8_t _MW_CP_mc_relay_ctl(int value)
{
	uint8_t ret_value = _CONTINUE;

	ctl_value = value;

	if(bak_ctl_value != ctl_value)
	{
		if(_TRUE == _LIB_USERDELAY_start(&gDelay_mcctl, DELAY_RENEW_OFF))
		{
			if(value == _OFF)
			{
				_MW_GPIO_set_gpo(MC_TURN_OFF, _ON);
				_MW_GPIO_set_gpo(MC_TURN_ON, _OFF);
#if ((__CP_DEBUG__)==1)
				_LIB_LOGGING_printf("MC_RELAY : --TURN OFF-- \r\n");
#endif
			}
			else	//value == _ON_
			{
				_MW_GPIO_set_gpo(MC_TURN_ON, _ON);
				_MW_GPIO_set_gpo(MC_TURN_OFF, _OFF);
#if ((__CP_DEBUG__)==1)
				_LIB_LOGGING_printf("MC_RELAY : --TURN ON-- \r\n");
#endif
			}
		}
		if(_TRUE == _LIB_USERDELAY_isfired(&gDelay_mcctl))
		{
			_LIB_USERDELAY_stop(&gDelay_mcctl);
			_MW_GPIO_set_gpo(MC_TURN_ON, _OFF);
			_MW_GPIO_set_gpo(MC_TURN_OFF, _OFF);
			bak_ctl_value = ctl_value;
			ret_value = _TRUE;
		}
	}

	//Todo
	// weld check --> compulsion mc on/off squence

	return ret_value;
#if 0
	if(value == _OFF)
	{
		if((_MW_GPIO_get_gpo(MC_TURN_ON) == _ON) && (_MW_GPIO_get_gpo(MC_TURN_OFF) == _OFF))
		{
			_MW_GPIO_set_gpo(MC_TURN_OFF, _ON);
			_MW_GPIO_set_gpo(MC_TURN_ON, _OFF);
#if ((__CP_DEBUG__)==1)
			_LIB_LOGGING_printf("MC_RELAY : --TURN OFF-- \r\n");
#endif
			return _TRUE;
		}
	}
	else	//value == _ON_
	{
		if((_MW_GPIO_get_gpo(MC_TURN_OFF) == _ON) || (_MW_GPIO_get_gpo(MC_TURN_ON) == _OFF))
		{
			_MW_GPIO_set_gpo(MC_TURN_ON, _ON);
			_MW_GPIO_set_gpo(MC_TURN_OFF, _OFF);
#if ((__CP_DEBUG__)==1)
			_LIB_LOGGING_printf("MC_RELAY : --TURN ON-- \r\n");
#endif
			return _TRUE;
		}
	}

	return _FALSE;
#endif
}
#else
#if 0
uint8_t _MW_CP_mc_relay_ctl(int value)
{
	uint8_t ret_value = _CONTINUE;

	ctl_value = value;

	if(bak_ctl_value != ctl_value)
	{
		if(_TRUE == _LIB_USERDELAY_start(&gDelay_mcctl, DELAY_RENEW_ON))
		{
			if(value == _OFF)
			{
				_MW_GPIO_set_gpo(MC_TURN_OFF, _ON);
				_MW_GPIO_set_gpo(MC_TURN_ON, _OFF);
#if ((__CP_DEBUG__)==1)
				_LIB_LOGGING_printf("MC_RELAY : --TURN OFF-- \r\n");
#endif
			}
			else	//value == _ON_
			{
				_MW_GPIO_set_gpo(MC_TURN_ON, _ON);
				_MW_GPIO_set_gpo(MC_TURN_OFF, _OFF);
#if ((__CP_DEBUG__)==1)
				_LIB_LOGGING_printf("MC_RELAY : --TURN ON-- \r\n");
#endif
			}
			bak_ctl_value = ctl_value;
		}
	}

	if(_TRUE == _LIB_USERDELAY_isfired(&gDelay_mcctl))
	{
		_LIB_USERDELAY_stop(&gDelay_mcctl);
		_MW_GPIO_set_gpo(MC_TURN_ON, _OFF);
		_MW_GPIO_set_gpo(MC_TURN_OFF, _OFF);
		ret_value = _TRUE;
	}

	//Todo
	// weld check --> compulsion mc on/off squence

	return ret_value;
}
#else

uint8_t _MW_CP_mc_relay_clear()
{
	uint8_t ret_value = _CONTINUE;

	if(_TRUE == _LIB_USERDELAY_isfired(&gDelay_mcctl))
	{
		_LIB_USERDELAY_stop(&gDelay_mcctl);
		_MW_GPIO_set_gpo(MC_TURN_ON, _OFF);
		_MW_GPIO_set_gpo(MC_TURN_OFF, _OFF);
		ret_value = _TRUE;
	}

	if(_TRUE == ret_value)
	{
#if ((__CP_DEBUG__)==1)
		_LIB_LOGGING_printf("MC_RELAY : --CONTROL_COMPLETE-- \r\n");
#endif
	}

	return ret_value;
}

uint8_t _MW_CP_mc_relay_ctl(int value)
{
	uint8_t ret_value = _CONTINUE;

	ctl_value = value;

	if(bak_ctl_value != ctl_value)
	{
		if(_TRUE == _LIB_USERDELAY_start(&gDelay_mcctl, DELAY_RENEW_ON))
		{
			if(value == _OFF)
			{
				_MW_GPIO_set_gpo(MC_TURN_OFF, _ON);
				_MW_GPIO_set_gpo(MC_TURN_ON, _OFF);
#if ((__CP_DEBUG__)==1)
				_LIB_LOGGING_printf("MC_RELAY : --TURN OFF-- \r\n");
#endif
			}
			else	//value == _ON_
			{
				_MW_GPIO_set_gpo(MC_TURN_ON, _ON);
				_MW_GPIO_set_gpo(MC_TURN_OFF, _OFF);
#if ((__CP_DEBUG__)==1)
				_LIB_LOGGING_printf("MC_RELAY : --TURN ON-- \r\n");
#endif
			}
			bak_ctl_value = ctl_value;
		}
	}

#if 0
	if(_TRUE == _LIB_USERDELAY_isfired(&gDelay_mcctl))
	{
		_LIB_USERDELAY_stop(&gDelay_mcctl);
		_MW_GPIO_set_gpo(MC_TURN_ON, _OFF);
		_MW_GPIO_set_gpo(MC_TURN_OFF, _OFF);
		ret_value = _TRUE;
	}
#endif

	//Todo
	// weld check --> compulsion mc on/off squence

	return ret_value;
}
#endif
#endif
uint8_t _MW_CP_mc_relay_off_fast()
{

	if(_MW_CP_get_mc_relay_state() == _ON)
	{
		GPO_MC_TURN_OFF_ENABLE;
		GPO_MC_TURN_ON_DISABLE;

		_LIB_LOGGING_printf("#### fast mc off _ OK #### \r\n");

		return _TRUE;
	}
	//ctl_value = _OFF;
	//bak_ctl_value = ctl_value;

	//_LIB_LOGGING_printf("#### fast mc off _ No Effect #### \r\n");

	return _FALSE;
}


uint8_t _MW_CP_get_cp_relay_state()
{
	if(_MW_GPIO_get_gpo(CP_RY) == _ON)
	{
		return _ON;
	}
	else	//if(_MW_GPIO_get_gpo(CP_RY) == _OFF)
	{
		return _OFF;
	}
}

uint8_t _MW_CP_get_mc_relay_state()
{
#if 0
	if((_MW_GPIO_get_gpo(MC_TURN_ON) == _ON) && (_MW_GPIO_get_gpo(MC_TURN_OFF) == _OFF))
	{
		return _ON;
	}
	else if((_MW_GPIO_get_gpo(MC_TURN_OFF) == _ON) && (_MW_GPIO_get_gpo(MC_TURN_ON) == _OFF))
	{
		return _OFF;
	}
	else
	{
		return _OFF;
	}
#else
	if(ctl_value == _ON)
	{
		return _ON;
	}
	else
	{
		return _OFF;
	}
#endif
}

CP_StateDef _MW_CP_get_cp_state()
{
	return CP.State;
}

/* KC 61851-1 Table A.5  */
uint16_t _MW_CP_cal_ampe_to_duty(uint16_t Ampe)
{
	double temp_Ampe = (double)Ampe;
	double temp_Duty = 5;

	/* 6A ~ 51A */
	if((temp_Ampe > 6) && (temp_Ampe <= 51))
	{
		temp_Duty = temp_Ampe / 0.6F;
	}
	/* 51A ~ 80A */
	else if((temp_Ampe > 51) && (temp_Ampe <= 80))
	{
		temp_Duty = ( temp_Ampe / 2.5F + 64.0F );
	}

	return (uint16_t)temp_Duty;
}


uint8_t _MW_CP_print_cp_state()
{
	if(CP.State != CP.State_bk)
	{
		switch(CP.State)
		{
			case default0 :
				printf("@@@@ CP_ADC_State : default0 @@@@ \r\n");
			break;

			case DC_12V :
				printf("@@@@ CP_ADC_State : DC_12V @@@@ \r\n");
			break;

			case DC_9V :
				printf("@@@@ CP_ADC_State : DC_9V @@@@ \r\n");
			break;

			case DC_6V :
				printf("@@@@ CP_ADC_State : DC_6V @@@@ \r\n");
			break;

			case PWM_12V :
				printf("@@@@ CP_ADC_State : PWM_12V @@@@ \r\n");
			break;

			case PWM_9V :
				printf("@@@@ CP_ADC_State : PWM_9V @@@@ \r\n");
			break;

			case PWM_6V :
				printf("@@@@ CP_ADC_State : PWM_6V @@@@ \r\n");
			break;

			case DC_N12V :
				printf("@@@@ CP_ADC_State : DC_N12V @@@@ \r\n");
			break;

			case Err_PWMH :
			case Err_DC :
				printf("@@@@ CP_ADC_State : Err_H : %d / 100 @@@@ \r\n", ((int)_MW_CP_get_h_final_voltage() * 100));
			break;

			case Err_PWML :
				printf("@@@@ CP_ADC_State : Err_L : %d / 100 @@@@ \r\n", ((int)_MW_CP_get_l_final_voltage() * 100));
			break;

		}

		CP.State_bk = CP.State;
	}

	return _TRUE;
}

uint8_t _MW_CP_average_adc(uint8_t section, uint16_t adcvalue)
{
	switch(section)
	{
		case CP_SECTION_H:
			CP.H.val_ADC = adcvalue;

			if(CP.H.complete_ADC == 0)
			{
				CP.H.sum_ADC += CP.H.val_ADC;
				CP.H.ave_cnt++;
				if(CP.H.ave_cnt >= CP_ADC_AVE_BUF_LENGTH)
				{
					CP.H.ave_ADC = CP.H.sum_ADC * CP_ADC_AVE_BUF_LENGTH_REVERSE;

					CP.H.sum_ADC = 0;
					CP.H.ave_cnt = 0;

					CP.H.complete_ADC = 1;
					return _TRUE;
				}
				else
				{
					return _CONTINUE;
				}
			}
			else
			{
				return _FALSE;
			}
		break;
		case CP_SECTION_L:
			CP.L.val_ADC = adcvalue;

			if(CP.L.complete_ADC == 0)
			{
				CP.L.sum_ADC += CP.L.val_ADC;
				CP.L.ave_cnt++;
				if(CP.L.ave_cnt >= CP_ADC_AVE_BUF_LENGTH)
				{
					CP.L.ave_ADC = CP.L.sum_ADC * CP_ADC_AVE_BUF_LENGTH_REVERSE;

					CP.L.sum_ADC = 0;
					CP.L.ave_cnt = 0;

					CP.L.complete_ADC = 1;
					return _TRUE;
				}
				else
				{
					return _CONTINUE;
				}
			}
			else
			{
				return _FALSE;
			}
		break;
		default :
			return _FALSE;
		break;
	}
}

uint8_t CP_is_complete_ADC(uint8_t section)
{
	switch(section)
	{
		case CP_SECTION_H:
			if(CP.H.complete_ADC == 1)
			{
				return _TRUE;
			}
			else
			{
				return _CONTINUE;
			}
		break;
		case CP_SECTION_L:
			if(CP.L.complete_ADC == 1)
			{
				return _TRUE;
			}
			else
			{
				return _CONTINUE;
			}
		break;
		default :
			return _FALSE;
		break;
	}
}

void CP_reset_complete_ADC(uint8_t section)
{
	switch(section)
	{
		case CP_SECTION_H:
			CP.H.complete_ADC = 0;
		break;
		case CP_SECTION_L:
			CP.L.complete_ADC = 0;
		break;
		default :

		break;
	}
}

#if 0
uint8_t _MW_CP_calculate_adc()
{
	if(_TRUE == _MW_CP_get_cp_relay_state())
	{
		CP.H.val_Volt = CP_ADC_Resolution * (CP.H.val_ADC - CP_INPUT_LEVEL_CENTER_ADC_VALUE);
		CP.L.val_Volt = CP_ADC_Resolution * (CP.L.val_ADC - CP_INPUT_LEVEL_CENTER_ADC_VALUE);

		CP.H.sum_Volt += CP.H.val_Volt;
		CP.L.sum_Volt += CP.L.val_Volt;
		CP.ave_cnt++;

		if(CP.ave_cnt >= CP_ADC_AVE_BUF_LENGTH)
		{
			CP.H.ave_Volt = CP.H.sum_Volt * CP_ADC_AVE_BUF_LENGTH_REVERSE;
			CP.L.ave_Volt = CP.L.sum_Volt * CP_ADC_AVE_BUF_LENGTH_REVERSE;

			CP.H.sum_Volt = 0;
			CP.L.sum_Volt = 0;
			CP.ave_cnt = 0;

			return _TRUE;
		}
	}
	else
	{
		_LIB_DEBOUNCECHECK_init(CP.DebounceCheckBuf, CP_ADC_DEBOUNCECHECKBUF_LENGTH);

		CP.State = default0;

		CP.H.val_Volt = 0;
		CP.L.val_Volt = 0;
		CP.H.sum_Volt = 0;
		CP.L.sum_Volt = 0;
		CP.H.ave_Volt = 0;
		CP.L.ave_Volt = 0;
		CP.ave_cnt = 0;
	}
	return _CONTINUE;
}
#else
#if 0
uint8_t _MW_CP_calculate_adc()
{
	uint8_t cp_section_h_complete_adc = CP_is_complete_ADC(CP_SECTION_H);
	uint8_t cp_section_l_complete_adc = CP_is_complete_ADC(CP_SECTION_L);

	if(_TRUE == _MW_CP_get_cp_relay_state())
	{
		if((_TRUE == cp_section_h_complete_adc) && (_TRUE == cp_section_l_complete_adc))
		{
			CP.H.val_Volt = CP_ADC_Resolution * (CP.H.ave_ADC - CP_INPUT_LEVEL_CENTER_ADC_VALUE);
			CP.H.ave_Volt = CP.H.val_Volt;

			//_LIB_LOGGING_printf("H : %d\r\n", (uint16_t)(CP.H.ave_Volt * 100));

			CP.L.val_Volt = CP_ADC_Resolution * (CP.L.ave_ADC - CP_INPUT_LEVEL_CENTER_ADC_VALUE);
			CP.L.ave_Volt = CP.L.val_Volt;

			CP_reset_complete_ADC(CP_SECTION_H);
			CP_reset_complete_ADC(CP_SECTION_L);

			return _TRUE;
		}
	}
	else
	{
		_LIB_DEBOUNCECHECK_init(CP.DebounceCheckBuf, CP_ADC_DEBOUNCECHECKBUF_LENGTH);

		CP.State = default0;

		CP.H.val_Volt = 0;
		CP.L.val_Volt = 0;
		CP.H.sum_Volt = 0;
		CP.L.sum_Volt = 0;
		CP.H.ave_Volt = 0;
		CP.L.ave_Volt = 0;
	}
	return _CONTINUE;
}
#else
uint8_t _MW_CP_calculate_adc()
{
	uint8_t cp_section_h_complete_adc = CP_is_complete_ADC(CP_SECTION_H);
	uint8_t cp_section_l_complete_adc = CP_is_complete_ADC(CP_SECTION_L);

	int32_t temp_CP_H_ave = 0;
	int32_t temp_CP_L_ave = 0;
	int32_t temp_CP_H_cal = 0;
	int32_t temp_CP_L_cal = 0;

	if(_TRUE == _MW_CP_get_cp_relay_state())
	{
		if((_TRUE == cp_section_h_complete_adc) && (_TRUE == cp_section_l_complete_adc))
		{
			temp_CP_H_ave = (int32_t)CP.H.ave_ADC;
			temp_CP_H_cal = temp_CP_H_ave - CP_INPUT_LEVEL_CENTER_ADC_VALUE;

			CP.H.val_Volt = CP_ADC_Resolution * ((double)temp_CP_H_cal);
			CP.H.ave_Volt = CP.H.val_Volt;

			//_LIB_LOGGING_printf("H : %d\r\n", (uint16_t)(CP.H.ave_Volt * 100));

			temp_CP_L_ave = (int32_t)CP.L.ave_ADC;
			temp_CP_L_cal = temp_CP_L_ave - CP_INPUT_LEVEL_CENTER_ADC_VALUE;

			CP.L.val_Volt = CP_ADC_Resolution * ((double)temp_CP_L_cal);
			CP.L.ave_Volt = CP.L.val_Volt;

			CP_reset_complete_ADC(CP_SECTION_H);
			CP_reset_complete_ADC(CP_SECTION_L);

			return _TRUE;
		}
	}
	else
	{
		_LIB_DEBOUNCECHECK_init(CP.DebounceCheckBuf, CP_ADC_DEBOUNCECHECKBUF_LENGTH);

		CP.State = default0;

		CP.H.val_Volt = 0;
		CP.L.val_Volt = 0;
		CP.H.sum_Volt = 0;
		CP.L.sum_Volt = 0;
		CP.H.ave_Volt = 0;
		CP.L.ave_Volt = 0;
	}
	return _CONTINUE;
}
#endif
#endif

uint8_t _MW_CP_calculate_state()
{
	double temp_pwm_duty = _MW_CP_get_pwm_duty();
	double temp_h_final_volt = _MW_CP_get_h_final_voltage();
#if ((CP_N_VOLTAGE_CHECK)==1)
#if 0
	double temp_l_final_volt = _MW_CP_get_l_final_voltage();
#endif
#endif

	_LIB_DEBOUNCECHECK_shift(CP.DebounceCheckBuf, CP_ADC_DEBOUNCECHECKBUF_LENGTH);

	if(temp_pwm_duty > 0)
	{
		if(temp_h_final_volt >= CP_P12V_MINIMUM)
		{
			if(temp_pwm_duty != 100)	_LIB_DEBOUNCECHECK_push(CP.DebounceCheckBuf, PWM_12V);
			else						_LIB_DEBOUNCECHECK_push(CP.DebounceCheckBuf, DC_12V);
		}
		else if(temp_h_final_volt >= CP_P9V_MINIMUM)
		{
			if(temp_pwm_duty != 100)	_LIB_DEBOUNCECHECK_push(CP.DebounceCheckBuf, PWM_9V);
			else						_LIB_DEBOUNCECHECK_push(CP.DebounceCheckBuf, DC_9V);
		}
		else if(temp_h_final_volt >= CP_P6V_MINIMUM)
		{
			if(temp_pwm_duty != 100)	_LIB_DEBOUNCECHECK_push(CP.DebounceCheckBuf, PWM_6V);
			else						_LIB_DEBOUNCECHECK_push(CP.DebounceCheckBuf, DC_6V);
		}
		else
		{
			if(temp_pwm_duty != 100)	_LIB_DEBOUNCECHECK_push(CP.DebounceCheckBuf, Err_PWMH);
			else						_LIB_DEBOUNCECHECK_push(CP.DebounceCheckBuf, Err_DC);
		}

#if ((CP_N_VOLTAGE_CHECK)==1)
#if 0
		if(temp_l_final_volt > CP_N12V_MINIMUM)
		{
			if(temp_pwm_duty != 100)	_LIB_DEBOUNCECHECK_push(CP.DebounceCheckBuf, Err_PWML);
		}
#endif
#endif

	}

	if(_TRUE == _LIB_DEBOUNCECHECK_compare(CP.DebounceCheckBuf, CP_ADC_DEBOUNCECHECKBUF_LENGTH, 0))
	{
		CP.State = CP.DebounceCheckBuf[CP_ADC_VALUE_INDEX];
#if ((__CP_DEBUG__)==1)
		_MW_CP_print_cp_state();
#endif
		return _TRUE;
	}

	return _CONTINUE;
}

void _MW_CP_main()
{
	if(_TRUE == _MW_CP_calculate_adc())
	{
		_MW_CP_calculate_state();
	}
}




