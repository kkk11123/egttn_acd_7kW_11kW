/*
 * _app_metering_service.c
 *
 *  Created on: 2024. 5. 14.
 *      Author: USER
 */

#include <_app_metering_service.h>
#include <_config.h>

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

				I_rms_U32 = (uint32_t)(I_rms * 1000);

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

typedef struct {
    uint32_t vrms_value;  // 계산된 VRMS 최대값
    uint32_t compensation;  // 보상값
} VLOOKUP;

#define VLOOKUP_SIZE 65

VLOOKUP vrms_lookup_table[VLOOKUP_SIZE] = {
	{13236, -8516}, // 실제 전압 ~50V
	{13435, -8216}, // 실제 전압 51V ~ 55V
	{13635, -7915}, // 실제 전압 56V ~ 60V
	{13863, -7644}, // 실제 전압 61V ~ 64V
	{14092, -7373}, // 실제 전압 65V ~ 69V
	{14336, -7116}, // 실제 전압 70V ~ 74V
	{14580, -6859}, // 실제 전압 75V ~ 79V
	{14831, -6611}, // 실제 전압 80V ~ 84V
	{15082, -6362}, // 실제 전압 85V ~ 89V
	{15351, -6132}, // 실제 전압 90V ~ 94V
	{15621, -5901}, // 실제 전압 95V ~ 99V
	{15894, -5675}, // 실제 전압 100V ~ 104V
	{16167, -5448}, // 실제 전압 105V ~ 109V
	{16450, -5217}, // 실제 전압 110V ~ 114V
	{16734, -4986}, // 실제 전압 115V ~ 119V
	{16979, -4745}, // 실제 전압 120V ~ 124V
	{17224, -4504}, // 실제 전압 125V ~ 129V
	{17478, -4274}, // 실제 전압 130V ~ 134V
	{17732, -4043}, // 실제 전압 135V ~ 139V
	{18009, -3805}, // 실제 전압 140V ~ 144V
	{18287, -3567}, // 실제 전압 145V ~ 149V
	{18549, -3330}, // 실제 전압 150V ~ 154V
	{18812, -3092}, // 실제 전압 155V ~ 159V
	{19077, -2858}, // 실제 전압 160V ~ 164V
	{19343, -2623}, // 실제 전압 165V ~ 169V
	{19609, -2389}, // 실제 전압 170V ~ 174V
	{19875, -2155}, // 실제 전압 175V ~ 179V
	{20137, -1917}, // 실제 전압 180V ~ 184V
	{20399, -1679}, // 실제 전압 185V ~ 189V
	{20639, -1419}, // 실제 전압 190V ~ 194V
	{20879, -1159}, // 실제 전압 195V ~ 199V
	{21156, -936},  // 실제 전압 200V ~ 204V
	{21350, -672},  //실제 전압 205V ~ 210V
	{21480, -300}, //실제 전압 211V
	{21550, -250},//212V ~ 213V
	{21650, -200},//214V ~ 216V
	{21700, 0},//217V
    {21800, 0},//218V
	{21860, 100},//219V
	{21950, 100}, //220V~221V
	{22050, 250}, //222V ~ 223V
	{22150, 350}, //224V ~ 225V
    {22260, 450}, //226V ~ 227V
	{22350, 550}, //228V ~ 229V
	{22450, 650}, //230V ~ 231V
	{22550, 750}, //232V ~ 233V
	{22650, 850}, //234V ~ 235V
	{22750, 950}, //236V ~ 237V
	{22850, 1050}, //238V ~ 239V
	{22950, 1250}, //240V ~ 242V
	{23050, 1350}, //243V ~ 244V
	{23150, 1500}, //245V ~ 246V
    {23250, 1600}, //247V ~ 248V
	{23300, 1650}, //249V ~ 250V
	{23400, 1800}, //251V ~ 252V
	{23450, 1900}, //253V
	{23480, 1950}, //254V
	{23500, 2000}, //255V
	{23550, 2050}, //256V
	{23600, 2200}, //257V ~ 258V
	{23650, 2300}, //259V
    {23700, 2250}, //260V
	{23950, 2400}, //265V
	{24050, 2800}, //270V
};

// look up 테이블로부터 보상값을 찾는 함수
uint32_t get_compensation(uint32_t vrms) {
    for (int i = 0; i < VLOOKUP_SIZE; i++) {
        if (vrms <= vrms_lookup_table[i].vrms_value) {//계산된 vrms의 최대값과 vrms 기준값과 비교
            return vrms_lookup_table[i].compensation;
        }
    }
    // 270V 이상 -> 보상값 28V
    return vrms_lookup_table[VLOOKUP_SIZE - 1].compensation;
}

//uint32_t get_vrms_value(uint32_t vrms) {
//    for (int i = 0; i < VLOOKUP_SIZE; i++) {
//        if (vrms <= vrms_lookup_table[i].vrms_value) {
//            return vrms_lookup_table[i].vrms_value;
//        }
//    }
//}

#if 1
void _APP_CHARGSERV_check_Vrms_loop() {
    uint16_t temp = gADCData[ADC_AC_V_INDEX_];

    static uint16_t dtemp = 0;
    static uint32_t adc_temp[300] = {0};
    static uint16_t adc_temp_index = 0;
    uint32_t adc_temp_upper = 0;

    double vrms_adc_value = (double)temp;


#if ((_VRMS_IRMS_CALC_LPF_FILTER_) == 1)
    uint32_t adc_temp_lpf = 0;
#endif

#if 0
    double vrms_adc_input_voltage = ((vrms_adc_value / 4096.0) * 3.3);
#else
    double vrms_adc_input_voltage = (((vrms_adc_value / 4096.0) * 3.3) + 1.28);
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
    //adc_temp_lpf = _LIB_LPF_calc(&Vrms_calc, adc_temp_upper);

    uint32_t compensation = get_compensation(adc_temp_upper);//vrms 최대값을 바탕으로 보상값 계산
    //uint32_t vrms_value = get_vrms_value(adc_temp_upper);
	adc_temp_upper += compensation;
	//lpf를 거친 신호에 보상을 해줄 경우 lpf 값이 계속 달라져서 정확한 보상값을 찾기 힘듬

    _APP_CHARGSERV_set_voltage_rms_V(adc_temp_upper);
#endif

#if 1
    dtemp++;

    if (dtemp > 1000) {
        dtemp = 0;


       // printf(" vrms_value : %ld \r\n",  vrms_value);
        printf("before compensation VRMS : %ld \r\n", adc_temp_upper_value);
        printf("compensation : %ld \r\n", compensation);
        printf("after compensation VRMS : %ld \r\n", adc_temp_upper);
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
