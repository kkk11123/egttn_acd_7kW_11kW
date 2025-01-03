/*
 * _mw_cp.h
 *
 *  Created on: Jan 6, 2023
 *      Author: MSI
 */

#ifndef MW_INC__MW_CP_H_
#define MW_INC__MW_CP_H_

#include "main.h"
#include "adc.h"
#include "dma.h"
#include "tim.h"

#define __CP_DEBUG__			1

#define CP_ADC_DEBOUNCECHECKBUF_LENGTH		3
#define CP_ADC_VALUE_INDEX					((CP_ADC_DEBOUNCECHECKBUF_LENGTH)-(1))

#if 1
#define ADC_CP_INDEX_		3
#define ADC_TEMP_INDEX_		4
#define ADC_ZCT_INDEX_		1
#define ADC_AC_V_INDEX_		0
#define ADC_AC_A_INDEX_		2
#define MAX_ADC_CH			5
#else
#define ADC_CP_INDEX_		2
#define ADC_TEMP_INDEX_		3
#define ADC_ZCT_INDEX_		0
//#define ADC_AC_V_INDEX_		0
#define ADC_AC_A_INDEX_		1
#define MAX_ADC_CH			4
#endif

/*
 * CP Control ADC Sampling
 *
 *
 * 1.1 EVES Output Voltage level Range      : +12V ~ -10.5V
 * 1.2 EVES Output Voltage level Center     : 0V
 * 1.3 EVES Output Voltage level Real Range : +(12-0)V ~ -(12-0)V = +12V ~ -10.5V
 *
 *
 * 2.1 MCU Input Voltage level Range      : +2.85V ~ +0.45V
 * 2.2 MCU Input Voltage level Center     : +1.65V
 * 2.3 MCU Input Voltage level Real Range : +(2.85-1.65)V ~ (0.45-1.65)V = +1.2V ~ -1.2V
 * 2.4 EVES Output - MCU Input GAIN		  : 10
 *
 *
 * 3.1 ADC Sampling bit : 12bit (2^12 = 4096)
 * 3.2 ADC VREF			: 3.35V
 * 3.3 ADC Resolution   : ADC VREF / ADC Sampling bit = 0.000818V
 * 3.4 MCU Input level Center ADC Value = (MCU Input Voltage level Center) / (ADC VREF) * 2^(ADC Sampling bit)
 * 										= 1.65V / 3.3V * 2^12 = 2017.4(1988)
 *
 *
 * 4.1 Calculate EVES Output Voltage level = (MCU Input ADC Value - MCU Input level Center ADC Value) * (ADC Resolution) * (EVES-MCU GAIN)
 * 										   = (MCU Input ADC Value - 1861) * 0.00818
 *
 */
#define CP_ADC_Resolution					(0.00840F)//(0.00818F)//
#define CP_INPUT_LEVEL_CENTER_ADC_VALUE		(2110)//(2017)//
#define CP_ADC_AVE_BUF_LENGTH				(8)
#define CP_ADC_AVE_BUF_LENGTH_REVERSE		(0.125F)	// (1 / _CP_ADC_AVE_BUF_LENGTH_)

#define CP_P12V_MINIMUM						(10.5F)
#define CP_P9V_MINIMUM						(7.5F)
#define CP_P6V_MINIMUM						(4.5F)
#define CP_N12V_MINIMUM						(-9.5F)

#define CP_SECTION_H						0
#define CP_SECTION_L						1

#define CP_N_VOLTAGE_CHECK					1

#define MCCTL_DELAY							100

typedef enum
{
    default0 = 0,
    DC_12V,
    DC_9V,
    DC_6V,
    PWM_12V,
    PWM_9V,
    PWM_6V,
    DC_N12V,
    Err_PWMH,
    Err_PWML,
    Err_DC
} CP_StateDef;

struct sCalVolt
{
    uint16_t     val_ADC;
    uint16_t     sum_ADC;
    uint16_t     ave_ADC;
    uint16_t 	 ave_cnt;
    uint8_t 	 complete_ADC;
    double       val_Volt;
    double       sum_Volt;
    double       ave_Volt;
};

typedef struct
{
    double          	PWM_Duty;
    double          	PWM_DutyBK;

    CP_StateDef    		State;
    CP_StateDef         DebounceCheckBuf[CP_ADC_DEBOUNCECHECKBUF_LENGTH];

    struct sCalVolt      H;
    struct sCalVolt      L;
    //uint16_t 	 		 ave_cnt;

    CP_StateDef    		State_bk;

}sCP;

//extern sCP CP;

//extern __IO uint16_t gADCData[MAX_ADC_CH];
extern __IO uint16_t gADCData[MAX_ADC_CH];

void _MW_CP_init();
uint8_t _MW_CP_set_pwm_duty(double persent);
double _MW_CP_get_pwm_duty();
double _MW_CP_get_h_final_voltage();
double _MW_CP_get_l_final_voltage();
uint8_t _MW_CP_change_duty(TIM_HandleTypeDef *htim, uint32_t cp_channel);
uint8_t _MW_CP_cp_relay_ctl(int value);
uint8_t _MW_CP_mc_relay_ctl(int value);
uint8_t _MW_CP_mc_relay_clear();
uint8_t _MW_CP_mc_relay_off_fast();
uint8_t _MW_CP_get_cp_relay_state();
uint8_t _MW_CP_get_mc_relay_state();
CP_StateDef _MW_CP_get_cp_state();
uint16_t _MW_CP_cal_ampe_to_duty(uint16_t Ampe);
uint8_t _MW_CP_print_cp_state();
uint8_t _MW_CP_average_adc(uint8_t section, uint16_t adcvalue);
uint8_t _MW_CP_calculate_adc();
uint8_t _MW_CP_calculate_state();
void _MW_CP_main();

#endif /* MW_INC__MW_CP_H_ */
