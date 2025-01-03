/*
 * _mw_pwm.c
 *
 *  Created on: Jan 6, 2023
 *      Author: MSI
 */





#include <_mw_pwm.h>
#include <_config.h>

void _MW_PWM_init()
{
	HAL_TIM_PWM_Start_IT(&htim1,TIM_CHANNEL_1);
}

uint8_t _MW_PWM_set_duty(TIM_HandleTypeDef *htim, uint32_t cp_channel, uint8_t persent)
{
	TIM_OC_InitTypeDef sConfigOC = {0};
	uint16_t value = 0;
	uint16_t maxvalue = 23999;
	double unitvalue = 239.99;

	if((persent < 0) || (persent > 100))
	{
		return 0;
	}

	value = (uint16_t)(maxvalue - (uint16_t)(unitvalue * persent));
	//value = (uint16_t)(unitvalue * persent);

	//HAL_TIM_PWM_Stop(htim, cp_channel);
	HAL_TIM_PWM_Stop_IT(htim, cp_channel);
    sConfigOC.OCMode = TIM_OCMODE_PWM1;
    sConfigOC.Pulse = value;
    if (HAL_TIM_PWM_ConfigChannel(htim, &sConfigOC, cp_channel) != HAL_OK)
    {
	    Error_Handler();
    }
    //if(HAL_TIM_PWM_Start(htim, cp_channel) != HAL_OK)
    if(HAL_TIM_PWM_Start_IT(htim, cp_channel) != HAL_OK)
    {
  	    Error_Handler();
    }

    return 1;
}



