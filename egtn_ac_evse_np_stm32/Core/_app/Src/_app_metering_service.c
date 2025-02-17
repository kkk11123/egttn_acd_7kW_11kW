/*
 * _app_metering_service.c
 *
 *  Created on: 2024. 5. 14.
 *      Author: USER
 */

#include <_app_metering_service.h>
#include <_config.h>
#include <_mw_ntc.h>

#if ((_VRMS_IRMS_CALC_LPF_FILTER_) == 1)
static s_LIB_LPF Vrms_calc;
static s_LIB_LPF Irms_calc;
#endif
#if	((_IRMS_INPUT_LPF_FILTER_) == 1)
static s_LIB_LPF Irms_input;
#endif

static gUserDelay gDelay_metering_periodic_loop_time;


uint32_t calc_delta_t_100us(uint32_t prev, uint32_t t)
{

	if(t >= prev)
	{
		return (t - prev);
	}
	else
	{
		return (0xFFFFFFFF - prev + 1 + t);
	}
}

/*
 * CT ratio : 2500:1  (50A:20mA)
 *
 * CT_burden_r = 10R
 *
 * CT_input_v_max = 0.2V
 *
 * absolute value circuit ratio : 5.0x
 *
 * CT_mcu_input_v_max = 1V
 *
 * RC_filter_Voltage_Compensation = 1.21
 *
 * Instantaneous Current = 50A/1 * (3.3V/4096) * input_adc_value * RC_filter_Voltage_Compensation
 *
 */

#define INSTANT_CURRENT_CONSTANT  (0.0484F)

#define _Calc_IRMS_FUNC_VER_	2

#if ((_Calc_IRMS_FUNC_VER_) == 2)

uint32_t delta_t_total = 0;

void _APP_CHARGSERV_check_Irms_loop()
{
	static uint8_t step = 0;

	uint16_t temp = gADCData[ADC_AC_A_INDEX_];

	static uint16_t dtemp = 0;

	double irms_adc_value = 0;
	double irms_adc_value_final = 0;
	double instant_current = 0;
	double instant_current_final = 0;
	static double integral_instant_current = 0;
	uint32_t delta_t = 0;
	uint32_t t_curr = 0;
	static uint32_t t_bak = 0;
	//static uint32_t delta_t_total = 0;
	static uint8_t delta_t_count = 0;
	double per_t_total_intergral_current = 0;
	double I_rms = 0;
	uint32_t I_rms_U32 = 0;
	uint32_t I_rms_U32_LPF = 0;

	irms_adc_value = (double)temp;

	
	//irms_adc_value_final = irms_adc_value * sqrt(2);
	irms_adc_value_final = irms_adc_value;

	instant_current = INSTANT_CURRENT_CONSTANT * irms_adc_value_final;
	

	instant_current_final = instant_current * instant_current;



	switch(step)
	{
		case 0 :
			//integral_instant_current = 0;
			//delta_t_total = 0;
			//delta_t_count = 0;


			step = 1;
		case 1:
			t_curr = _LIB_USERDLEAY_gettick();
			delta_t = calc_delta_t_100us(t_bak, t_curr);

			integral_instant_current = integral_instant_current + (instant_current_final * (double)delta_t);
			delta_t_total = delta_t_total + delta_t;
			delta_t_count++;

			if(16 <= delta_t_count)
			{
				delta_t_count = 0;

				per_t_total_intergral_current = integral_instant_current / ((double)delta_t_total);

				I_rms = sqrt(per_t_total_intergral_current);

				I_rms_U32 = (uint32_t)(I_rms * 1000);//mA 단위로 변환

				I_rms_U32_LPF = _LIB_LPF_calc(&Irms_calc, I_rms_U32);

				_APP_CHARGSERV_set_current_rms_A(I_rms_U32_LPF);

				if(dtemp > 1000)
				{
					dtemp = 0;
					//printf("ADC:%ld\r\n",(uint32_t)(temp));
					//printf("inA:%ld\r\n",(uint32_t)(instant_current*100));
					//printf("instant_current_final:%ld\r\n",(uint32_t)(instant_current_final*100));
					//printf("dt:%ld\r\n",(uint32_t)delta_t);
					printf("p:%ld\r\n",(uint32_t)integral_instant_current);
					printf("t:%ld\r\n",(uint32_t)delta_t_total);
					printf("I_RMS(x1000):%ld\r\n",I_rms_U32_LPF);
				}
				else
				{
					dtemp++;
				}

				integral_instant_current = 0;
				delta_t_total = 0;

				step = 0;
			}

			t_bak = t_curr;
			break;
		default:

			break;
	}

}

#elif ((_Calc_IRMS_FUNC_VER_) == 1)
void _APP_CHARGSERV_check_Irms_loop()
{
	uint16_t temp = gADCData[ADC_AC_A_INDEX_];

	static uint16_t dtemp = 0;
	static uint32_t adc_temp[500] = {0,};
	static uint16_t adc_temp_index = 0;
	uint32_t adc_temp_upper = 0;

	double Irms_adc_value = (double)temp;
#if ((CP_N_VOLTAGE_CHECK)==1)
	double temp_l_final_volt = _MW_CP_get_l_final_voltage();
	double temp_H_final_volt = _MW_CP_get_h_final_voltage();
#endif


#if	((_IRMS_INPUT_LPF_FILTER_) == 1)
	uint32_t Irms_adc_int32_value = (uint32_t)(Irms_adc_value * 28.30);
	uint32_t Irms_adc_int32_lpf = _LIB_LPF_calc(&Irms_input, Irms_adc_int32_value);

	adc_temp[adc_temp_index++] = Irms_adc_int32_lpf;
#else
	//adc_temp[adc_temp_index++] = (uint32_t)(Irms_adc_value * 30.22);
	adc_temp[adc_temp_index++] = (uint32_t)(Irms_adc_value * 28.30);
	//adc_temp[adc_temp_index++] = (uint32_t)(Irms_adc_value * 29.22);
	//adc_temp[adc_temp_index++] = (uint32_t)(Irms_adc_value * 24.18);
	//adc_temp[adc_temp_index++] = (uint32_t)(Irms_adc_value * 23.4);
#endif

	if(adc_temp_index >= 500)	adc_temp_index = 0;


	for(int i = 0; i<500; i++)
	{
		if(adc_temp[i] > adc_temp_upper)	adc_temp_upper = adc_temp[i];
	}

#if	((_VRMS_IRMS_CALC_LPF_FILTER_) == 1)
	uint32_t adc_temp_lpf = 0;
#endif

#if	((_VRMS_IRMS_CALC_LPF_FILTER_) == 0)
	Charger.current_I_rms = adc_temp_upper;
#else

	adc_temp_lpf = _LIB_LPF_calc(&Irms_calc, adc_temp_upper);

	Charger.current_I_rms = adc_temp_lpf;
#endif

#if 1
	dtemp++;

	if(dtemp > 200)
	{
		dtemp = 0;
		printf("IRMS : %ld \r\n", Charger.current_I_rms);
		printf("CP_H : %d \r\n", (int)(temp_H_final_volt * 100));
		printf("CP_L : %d \r\n", (int)(temp_l_final_volt * 100));

	}
#endif
}
#else
void _APP_CHARGSERV_check_Irms_loop()
{
	uint16_t temp = gADCData[ADC_AC_A_INDEX_];

	static uint16_t dtemp = 0;
	static uint16_t adc_temp[500] = {0,};
	static uint16_t adc_temp_index = 0;
	uint16_t adc_temp_max = 0;
	uint16_t adc_temp_min = 0;
	uint32_t adc_temp_avr = 0;

	double Irms_adc_value = (double)temp;

	//adc_temp[adc_temp_index++] = (uint32_t)(Irms_adc_value * 30.22);
	adc_temp[adc_temp_index++] = (uint16_t)(Irms_adc_value * 24.18);
	if(adc_temp_index >= 500)
	{
		adc_temp_index = 0;

		int i = 0;

		adc_temp_max = 0;
		adc_temp_min = adc_temp[0];
		adc_temp_avr = 0;

		for(int i = 0; i<500; i++)
		{
			if(adc_temp[i] > adc_temp_max)	adc_temp_max = adc_temp[i];
			if(adc_temp[i] < adc_temp_min)	adc_temp_min = adc_temp[i];

		}

		adc_temp_avr = ((((uint32_t)adc_temp_max) + ((uint32_t)adc_temp_min)) / 2);

		Charger.current_I_rms = adc_temp_avr;

#if 1
		//dtemp++;

		//if(dtemp > 200)
		//{
		//	dtemp = 0;
			printf("IRMS : %ld \r\n", Charger.current_I_rms);
		//}
#endif
	}
}
#endif

/*
 *
 * PT+Peak Detecter (15.2K)
 *
 * 242VAC = 2.6V
 * 220VAC = 2.36V
 * 198VAC = 2.12V
 * dvac = 0.01091
 * dvac_100x = 0.0001091
 * Peak_Vf = 0.65V
 * adc_max_value = 2^12 = 4096
 * Vadc_ref = 3.3V
 *
 * VRMS = ((input adc value / adc_max_value) * Vadc_ref) / dvac
 *
 * VRMS(+Peak Vf) = ((input adc value / adc_max_value) * Vadc_ref + Peak_Vf) / dvac
 *
 */

/*
 *
 * PT+Peak Detecter (15.2K)
 *
 * 242VAC = 2.6V
 * 220VAC = 2.36V
 * 198VAC = 2.12V
 * dvac = 0.01091
 * dvac_100x = 0.0001091
 * Peak_Vf = 0.65V
 * adc_max_value = 2^12 = 4096
 * Vadc_ref = 3.3V
 *
 * VRMS = ((input adc value / adc_max_value) * Vadc_ref) / dvac
 *
 * VRMS(+Peak Vf) = ((input adc value / adc_max_value) * Vadc_ref + Peak_Vf) / dvac
 *
 */


#define ADC_MAX 4095.0       // 12비트 ADC 최대값
uint32_t delta_t_total_v = 0; //전압 delta_t 총합

#if 1
// AC 전압값 리턴 함수
uint32_t _APP_METERING_calc_vrms(double Rinput, double Routput, float Vref, double ADC_value) {

	static uint8_t step_v = 0;

	double vrms_adc_value = 0;
	double vrms_adc_value_final = 0;
	double instant_voltage = 0;
	double instant_voltage_final = 0;
	static double integral_instant_voltage = 0;

	uint32_t delta_t = 0;
	uint32_t t_curr = 0;
	static uint32_t t_bak_v = 0;

	static uint8_t delta_t_count_v = 0;
	double per_t_total_intergral_voltage = 0;
	double V_rms = 0;
	uint32_t V_rms_U32 = 0;
	uint32_t V_rms_U32_LPF = 0;

	//입력 전압 계산
	double V_in = (((ADC_value/ADC_MAX) * 3.3) - 1.65) * (Rinput/Routput);

	instant_voltage_final = V_in * V_in;

	switch(step_v)
	{
		case 0 :
			step_v = 1;
		case 1:
			t_curr = _LIB_USERDLEAY_gettick();
			delta_t = calc_delta_t_100us(t_bak_v, t_curr);

			//vrms 계산
			integral_instant_voltage = integral_instant_voltage + (instant_voltage_final * (double)delta_t);
			delta_t_total_v = delta_t_total_v + delta_t;
			delta_t_count_v++;

			if(28 <= delta_t_count_v)
			{
				delta_t_count_v = 0;

				per_t_total_intergral_voltage = integral_instant_voltage / ((double)delta_t_total_v);

				V_rms = sqrt(per_t_total_intergral_voltage);

				V_rms_U32 = (uint32_t)(V_rms * 100);//소수 2자리까지 표시

				
				 //printf("V_in : %u \r\n", (uint32_t)(V_in));
				 printf("delta_total: %u\r\n", delta_t_total_v); //주기 총합
				
				integral_instant_voltage = 0;
				delta_t_total_v = 0;

				step_v = 0;
			}

			t_bak_v = t_curr;
			break;
		default:
			break;
	}
	return V_rms_U32;

}
#endif

#if 1
void _APP_CHARGSERV_check_Vrms_loop() {
    uint16_t temp = gADCData[ADC_AC_V_INDEX_];

    static uint16_t dtemp_v = 0;
    static uint32_t adc_temp[300] = {0};
    static uint16_t adc_temp_index = 0;
    uint32_t adc_temp_upper = 0;
	uint32_t adc_temp_lpf_value = 0;
	uint32_t vrms_voltage = 0;

    double vrms_adc_value = (double)temp;


#if ((_VRMS_IRMS_CALC_LPF_FILTER_) == 1)
    uint32_t adc_temp_lpf = 0;
#endif

	//AC 전압값 리턴 함수 적용(R_output: 250K, R_input : 1K, Vref: 1.65V)
	vrms_voltage = _APP_METERING_calc_vrms(250000.0, 1000.0, 1.65, vrms_adc_value);

	//vrms중에 최대값 탐색
    adc_temp[adc_temp_index++] = vrms_voltage;
    if (adc_temp_index >= 300) adc_temp_index = 0;

    for (int i = 0; i < 300; i++) {
        if (adc_temp[i] > adc_temp_upper) adc_temp_upper = adc_temp[i];
    }
    uint32_t adc_temp_upper_value = adc_temp_upper;

#if ((_VRMS_IRMS_CALC_LPF_FILTER_) == 0)
    Charger.current_V_rms = adc_temp_upper;
#else
	//LPF 적용(TimeInterval: 2, Tau: 800)
	 adc_temp_lpf = _LIB_LPF_calc(&Vrms_calc, adc_temp_upper_value);

	 _APP_CHARGSERV_set_voltage_rms_V(adc_temp_lpf);
#endif


//출력
#if 1
if(dtemp_v > 1000)
	{
		dtemp_v = 0;
		printf("VRMS : %ld \r\n", adc_temp_lpf);
		printf("zct : %d \r\n", gADCData[ADC_ZCT_INDEX_]);
	}
	else
	{
		dtemp_v++;
	}

#endif
}
#endif

void _APP_METERING_startup()
{
#if ((_VRMS_IRMS_CALC_LPF_FILTER_) == 1)
	_LIB_LPF_init(&Vrms_calc, _LPF_TIMEINTERVAL_VOL_, _LPF_TAU_VOL_);
	_LIB_LPF_init(&Irms_calc, _LPF_TIMEINTERVAL_CUR_, _LPF_TAU_CUR_);
#endif
#if	((_IRMS_INPUT_LPF_FILTER_) == 1)
	_LIB_LPF_init(&Irms_input, _IRMS_INPUT_LPF_TIMEINTERVAL_, _IRMS_INPUT_LPF_TAU_);
#endif

	_LIB_USERDELAY_set(&gDelay_metering_periodic_loop_time,1);
}

void _APP_METERING_process()
{
	static uint8_t vrms_step = 0;

	_LIB_USERDELAY_start(&gDelay_metering_periodic_loop_time, DELAY_RENEW_OFF);

    if(_LIB_USERDELAY_isfired(&gDelay_metering_periodic_loop_time))
    {
    	_APP_CHARGSERV_check_Irms_loop();

    	if(vrms_step >= 5)
    	{
    		vrms_step = 0;
    		_APP_CHARGSERV_check_Vrms_loop();
    	}
    	else
    	{
    		vrms_step++;
    	}

	  	_LIB_USERDELAY_start(&gDelay_metering_periodic_loop_time, DELAY_RENEW_ON);
    }
}
