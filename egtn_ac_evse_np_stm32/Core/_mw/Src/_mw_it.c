/*
 * _mw_it.c
 *
 *  Created on: Jan 6, 2023
 *      Author: MSI
 */



#include <_mw_it.h>
#include <_app_charging_service.h>
#include <_app_rfid_comm.h>
#include <_app_system_control.h>

static uint8_t _it_rx01 = 0, _it_rx02 = 0;

#if ((_LEAKAGE_CALC_LPF_FILTER_) == 1)
static s_LIB_LPF leakage_calc;
#define _LEAKAGE_LPF_TIMEINTERVAL_ (2)
#define _LEAKAGE_LPF_TAU_ (190)
#endif

void _MW_IT_init()
{
	HAL_UART_Receive_IT(&huart1, &_it_rx01, 1);
	HAL_UART_Receive_IT(&huart2, &_it_rx02, 1);

	HAL_TIM_Base_Start_IT(&htim3);

#if ((_LEAKAGE_CALC_LPF_FILTER_) == 1)
	_LIB_LPF_init(&leakage_calc, _LEAKAGE_LPF_TIMEINTERVAL_, _LEAKAGE_LPF_TAU_);
#endif
}

void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart)
{
	if(huart->Instance == USART1){
		HAL_UART_Receive_IT(&huart1, &_it_rx01, 1);
		//printf("0x%02x\r\n",_it_rx04);
		//HAL_UART_Transmit(&huart2, &_it_rx01, 1, 3000);
		_APP_RFID_push_ringbuffer(_it_rx01);
	}

	if(huart->Instance == USART2){
		HAL_UART_Receive_IT(&huart2, &_it_rx02, 1);
		_APP_SYSTEMCTL_push_ringbuffer(_it_rx02);
	}
}

void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim)
{
	static uint16_t temp_count = 0;

	if(htim == &htim3)
	{
		//Set_Task_Tick();
		_LIB_USERDELAY_tickcount();

		temp_count++;

		if(temp_count > 10000)
		{
			temp_count = 0;
			if(_MW_GPIO_get_gpo(STUS_LED_02) == 1)
			{
				_MW_GPIO_set_gpo(STUS_LED_02, 0);
			}
			else
			{
				_MW_GPIO_set_gpo(STUS_LED_02, 1);
			}
		}

	}
}

void HAL_TIM_PWM_PulseFinishedCallback(TIM_HandleTypeDef *htim)
{
	uint32_t bCh;

	if(htim == &htim1)
	{
		//printf("pwm cb\r\n");
		if(htim->Channel == HAL_TIM_ACTIVE_CHANNEL_1)
		{
			bCh = TIM_CHANNEL_1;

			_MW_CP_change_duty(htim, bCh);
		}
	}
}

void HAL_TIM_OC_DelayElapsedCallback(TIM_HandleTypeDef *htim)
{

	//220803 PES
	// delay leakage current check
	static uint16_t delay_leakage_current_count = 0;
	static uint8_t delay_leakage_current_flag = 0;

	static uint8_t leakage_detect_count = 0;

#if ((_LEAKAGE_CALC_LPF_FILTER_) == 1)
	static uint16_t leakage_temp[16] = {0, };
	static uint8_t leakage_temp_index = 0;
	uint16_t leakage_temp_upper = 0;
	uint32_t leakage_lpf = 0;

	static uint8_t print_flag = 0;

#endif
	if(htim == &htim1)
	{
		if(htim->Channel == HAL_TIM_ACTIVE_CHANNEL_2)
		{

			_MW_CP_average_adc(CP_SECTION_L, gADCData[ADC_CP_INDEX_]);

		}
		else if(htim->Channel == HAL_TIM_ACTIVE_CHANNEL_3)
		{

			_MW_CP_average_adc(CP_SECTION_H, gADCData[ADC_CP_INDEX_]);
			//printf("%d\r\n",gADCData[ADC_ZCT_INDEX_]);

		}
		else if(htim->Channel == HAL_TIM_ACTIVE_CHANNEL_4)
		{

			uint16_t temp = gADCData[ADC_ZCT_INDEX_];
			//if ??? >  trip fault bit set
			//printf("%d\r\n",gADCData[ADC_ZCT_INDEX_]);
			//zctadc = temp;

			if(_ON == _MW_CP_get_mc_relay_state())
			{
				if((CHARGSERV_LEAKAGE_DETECT_DELAY <= delay_leakage_current_count) && (0 == delay_leakage_current_flag))
				{
					delay_leakage_current_count = 0;
					delay_leakage_current_flag = 1;

#if ((_LEAKAGE_CALC_LPF_FILTER_) == 1)
					_LIB_LPF_init(&leakage_calc, _LEAKAGE_LPF_TIMEINTERVAL_, _LEAKAGE_LPF_TAU_);
					memset(leakage_temp,0,sizeof(leakage_temp));
					leakage_temp_index = 0;
					_LIB_LOGGING_printf("#### CHARGSERV : leakage_LPF_init #### \r\n");
#endif
				}
				else
				{
					delay_leakage_current_count++;
				}
			}
			else
			{
				delay_leakage_current_count = 0;
				delay_leakage_current_flag = 0;
			}

			if(1 == delay_leakage_current_flag)
			{
#if ((_LEAKAGE_CALC_LPF_FILTER_) == 0)
				if((temp > CHARGSERV_LEAKAGE_AMPE_TO_ADC) && (_OFF == _APP_CHARGSERV_is_leakage_fault_set()))
				{

#if 0
					_APP_CHARGSERV_leakage_fault_set(temp);
#else
					if(leakage_detect_count >= 3)
					{
						leakage_detect_count = 0;
						_APP_CHARGSERV_leakage_fault_set(temp);
					}
					else
					{
						leakage_detect_count++;
					}
#endif
				}
				else
				{
					leakage_detect_count = 0;
				}
#else
				leakage_temp[leakage_temp_index++] = temp;
				if(leakage_temp_index >= 16)	leakage_temp_index = 0;

				for(int i = 0; i<16; i++)
				{
					if(leakage_temp[i] > leakage_temp_upper)	leakage_temp_upper = leakage_temp[i];
				}

				if((0 == print_flag) && (leakage_temp_upper > CHARGSERV_LEAKAGE_AMPE_TO_ADC))
				{
					print_flag = 1;
					_LIB_LOGGING_printf("#### CHARGSERV : leakage_temp_upper  time: %ld #### \r\n",_LIB_USERDLEAY_gettick());

				}
				else if((1 == print_flag) && (leakage_temp_upper <= CHARGSERV_LEAKAGE_AMPE_TO_ADC))
				{
					print_flag = 0;
				}

				leakage_lpf = _LIB_LPF_calc(&leakage_calc, (uint32_t)leakage_temp_upper);

				if((leakage_lpf > CHARGSERV_LEAKAGE_AMPE_TO_ADC) && (_OFF == _APP_CHARGSERV_is_leakage_fault_set()))
				{
					_LIB_LOGGING_printf("#### CHARGSERV : leakage_fault_set  time: %ld #### \r\n",_LIB_USERDLEAY_gettick());
					_APP_CHARGSERV_leakage_fault_set(leakage_lpf);
				}
#endif
			}

		}
	}
}
