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


//#define TABLE_SIZE 11  // 195V ~ 245V까지 5V 단위
//
//typedef struct {
//    uint32_t vrms_value_15_25;  // 15도 ~ 25도 계산된 VRMS
//	uint32_t vrms_value_50_60; // 50도 ~ 60도 계산된 VRMS
//	float scale_factor_15_25;  // 15~25도 보상 배율
//    float scale_factor_50_60;  // 50~60도 보상 배율
//} LookupEntry;
//
//// 보상값(배율) 테이블 정의
//LookupEntry lookup_table[TABLE_SIZE] = {
//    {20387, 20911, 0.956, 0.933}, //195V 이하
//    {20526, 21162, 0.974, 0.945}, //200V
//    {20747, 21354, 0.988, 0.96}, //205V
//    {20984, 21620, 1.001, 0.971}, //210V
//    {21235, 21937, 1.012, 0.98}, //215V
//    {21471, 22093, 1.025, 0.996}, //220V
//	{21729, 22321, 1.035, 1.008}, //225V
//    {21981, 22587, 1.046, 1.018}, //230V
//	{22187, 22720, 1.059, 1.034}, //235V
//    {22424, 22905, 1.07, 1.048}, //240V
//	{22638, 22918, 1.082, 1.069}, //245V 이상
//};
//
//// 보상값 찾기 함수
//float get_scale_factor(uint32_t vrms, int16_t temperature)
//{
//    for (int i = 0; i < TABLE_SIZE; i++) {
//        if (temperature <= 25) {  // 온도가 25도 이하일경우
//            if (vrms <= lookup_table[i].vrms_value_15_25) {
//                return lookup_table[i].scale_factor_15_25;
//            }
//        } else if (temperature > 25) {  // 26도 이상인 경우에는 50~60도 배율 적용
//            if (vrms <= lookup_table[i].vrms_value_50_60) {
//                return lookup_table[i].scale_factor_50_60;
//            }
//        }
//    }
//    // 테이블 범위를 초과하면 마지막 값 반환
//    return (temperature <= 25) ? lookup_table[TABLE_SIZE - 1].scale_factor_15_25
//                                    : lookup_table[TABLE_SIZE - 1].scale_factor_50_60;
//}


#if 1
void _APP_CHARGSERV_check_Vrms_loop() {
    uint16_t temp = gADCData[ADC_AC_V_INDEX_];
	int16_t temperature = _MW_NTC_get_temp();

    static uint16_t dtemp = 0;
    static uint32_t adc_temp[300] = {0};
    static uint16_t adc_temp_index = 0;
    uint32_t adc_temp_upper = 0;
	uint32_t adc_temp_lpf_value = 0;

    double vrms_adc_value = (double)temp;


#if ((_VRMS_IRMS_CALC_LPF_FILTER_) == 1)
    uint32_t adc_temp_lpf = 0;
#endif

#if 0
    double vrms_adc_input_voltage = ((vrms_adc_value / 4096.0) * 3.3);
#else
    double vrms_adc_input_voltage = (((vrms_adc_value / 4096.0) * 3.3) + 1.65); //2025/02/11 +1.28 -> 1.65로 교체
#endif

    double vrms_voltage = ((vrms_adc_input_voltage * 10000000.0) / 1091.0);

    adc_temp[adc_temp_index++] = (uint32_t)vrms_voltage;
    if (adc_temp_index >= 300) adc_temp_index = 0;

    for (int i = 0; i < 300; i++) {
        if (adc_temp[i] > adc_temp_upper) adc_temp_upper = adc_temp[i];
    }
    uint32_t adc_temp_upper_value = adc_temp_upper;

#if ((_VRMS_IRMS_CALC_LPF_FILTER_) == 0)
    Charger.current_V_rms = adc_temp_upper;
#else
    adc_temp_lpf = _LIB_LPF_calc(&Vrms_calc, adc_temp_upper);

	//float compensation = get_scale_factor(adc_temp_lpf, temperature);// 계산된 lpf를 거친 vrms 값을 바탕으로 보상배율 결정
    
	adc_temp_lpf_value = (uint32_t)(adc_temp_lpf);
	//adc_temp_lpf_value = (uint32_t)(adc_temp_lpf * compensation); //보상값 적용

	 _APP_CHARGSERV_set_voltage_rms_V(adc_temp_lpf);
#endif

#if 1
    dtemp++;

    if (dtemp > 1000) {
        dtemp = 0;


       // printf(" vrms_value : %ld \r\n",  vrms_value);
        printf("before compensation VRMS : %ld \r\n", adc_temp_lpf);
//        printf("compensation : %ld \r\n", compensation);
//		printf("after compensation VRMS : %ld \r\n", adc_temp_lpf_value);
        printf("zct : %d \r\n", gADCData[ADC_ZCT_INDEX_]);
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
