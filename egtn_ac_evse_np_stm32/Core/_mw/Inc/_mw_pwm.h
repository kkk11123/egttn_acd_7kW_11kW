/*
 * _mw_pwm.h
 *
 *  Created on: Jan 6, 2023
 *      Author: MSI
 */

#ifndef MW_INC__MW_PWM_H_
#define MW_INC__MW_PWM_H_


#include "main.h"
#include "tim.h"

void _MW_PWM_init();
uint8_t _MW_PWM_set_duty(TIM_HandleTypeDef *htim, uint32_t cp_channel, uint8_t persent);



#endif /* MW_INC__MW_PWM_H_ */
