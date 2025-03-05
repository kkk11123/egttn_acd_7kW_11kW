/*
 * _mw_ntc.c
 *
 *  Created on: Jan 6, 2023
 *      Author: MSI
 */


#include <_mw_ntc.h>
#include <_mw_cp.h>
#include <_lib_logging.h>
#include <_config.h>

static uint16_t NTC_Temp_ADC[NTC_TEMP_ADC_LENGTH] = { 0, };
static int16_t NTC_Temp = 0;//1000;

// TSM1A103F34D1RZ R - T Table(Rnor)
// NTC VDD : 3.3V
// NTC Reference Resister : 10      (unit : Kohm)
// Temp_array Voltage = Rnor / (NTC Reference Resister + Rnor) * NTC VDD     (unit : V)

// -40 ~ +125
#if 0	// NTC VDD == 3.0V
const uint16_t Temp_array[170] = { 2860, 2852, 2844, 2835, 2826, 2817, 2807,
		2798, 2787, 2776, 2765, 2753, 2741, 2729, 2716, 2702, 2688, 2673, 2658,
		2643, 2626, 2610, 2593, 2575, 2557, 2538, 2519, 2499, 2479, 2459, 2438,
		2416, 2394, 2372, 2349, 2325, 2302, 2278, 2253, 2228, 2203, 2177, 2151,
		2125, 2098, 2071, 2044, 2016, 1988, 1960, 1932, 1904, 1875, 1847, 1818,
		1789, 1760, 1731, 1702, 1673, 1644, 1615, 1586, 1557, 1528, 1500, 1472,
		1444, 1416, 1388, 1361, 1333, 1306, 1280, 1253, 1227, 1202, 1176, 1151,
		1127, 1103, 1079, 1055, 1032, 1009, 987, 965, 943, 922, 901, 880, 860,
		840, 821, 802, 783, 765, 747, 730, 713, 696, 679, 663, 648, 632, 617,
		603, 588, 574, 560, 547, 534, 521, 509, 496, 484, 473, 461, 450, 440,
		429, 419, 409, 399, 390, 380, 371, 362, 354, 345, 337, 329, 322, 314,
		307, 300, 293, 286, 279, 273, 266, 260, 254, 249, 243, 238, 232, 227,
		222, 217, 212, 207, 203, 198, 194, 190, 186, 182, 178, 174, 170, 167,
		163, 160, 156, 153 };	// unit : mV
#else	// NTC VDD == 3.35V
const uint16_t Temp_array[] =
{
		3193,
		3185,
		3175,
		3166,
		3156,
		3146,
		3135,
		3124,
		3112,
		3100,
		3088,
		3075,
		3061,
		3047,
		3032,
		3017,
		3002,
		2985,
		2968,
		2951,
		2933,
		2914,
		2895,
		2875,
		2855,
		2834,
		2813,
		2791,
		2768,
		2745,
		2722,
		2698,
		2673,
		2648,
		2623,
		2597,
		2570,
		2543,
		2516,
		2488,
		2460,
		2431,
		2402,
		2373,
		2343,
		2313,
		2282,
		2251,
		2220,
		2189,
		2158,
		2126,
		2094,
		2062,
		2030,
		1998,
		1965,
		1933,
		1900,
		1868,
		1835,
		1803,
		1771,
		1739,
		1707,
		1675,
		1643,
		1612,
		1581,
		1550,
		1519,
		1489,
		1459,
		1429,
		1400,
		1371,
		1342,
		1314,
		1286,
		1258,
		1231,
		1204,
		1178,
		1152,
		1127,
		1102,
		1077,
		1053,
		1029,
		1006,
		983,
		961,
		938,
		917,
		896,
		875,
		854,
		835,
		815,
		796,
		777,
		759,
		741,
		723,
		706,
		689,
		673,
		657,
		641,
		626,
		611,
		596,
		582,
		568,
		554,
		541,
		528,
		515,
		503,
		491,
		479,
		468,
		456,
		446,
		435,
		419,
		414,
		405,
		395,
		386,
		377,
		368,
		359,
		351,
		342,
		334,
		327,
		319,
		312,
		305,
		298,
		291,
		284,
		278,
		271,
		265,
		259,
		253,
		248,
		242,
		237,
		232,
		227,
		222,
		217,
		212,
		207,
		203,
		199,
		194,
		190,
		186,
		182,
		178,
		175,
		171
};
#endif

//온도값 계산 함수
int _MW_NTC_cal_ntc(uint16_t i_temp)
{
	int j, k;
	unsigned char flag = 0;
	int m_result;

	for (j = 0; j < 17; j++)
	{
		if (i_temp > Temp_array[(j + 1) * 10 - 1])
		{
			for (k = 0; k < 10; k++)
			{
				if (i_temp > Temp_array[j * 10 + k])
				{
					m_result = (j * 10 + k) - 40;
					flag = 1;
					break;
				}
			}
		}
		if (flag == 1) break;
	}
	if (flag == 0) m_result = 0;//m_result = -100;	// ?>?????? 220413 PES Todo Type : m_result is no '-'

	return m_result;
}
//NTC 온도센서 ADC 결과값 저장 함수
uint8_t _MW_NTC_store_value(uint16_t ADC_value)
{
	static int index = 0;

	NTC_Temp_ADC[index] = ADC_value;
	index++;

	if(index >= NTC_TEMP_ADC_LENGTH)
	{
		index = 0;
		return _TRUE;
	}

	return _CONTINUE;
}
int16_t _MW_NTC_get_temp()
{
	return NTC_Temp;
}
//NTC ADC 값 -> 전압 변환 -> 온도값 반환
uint8_t _MW_NTC_cal_temp()
{
	static float ADCvalue;
	static float ADC_mV;

	ADCvalue = 0;

	for (int i = 0; i < NTC_TEMP_ADC_LENGTH; i++)
	{
		ADCvalue += NTC_Temp_ADC[i];
	}

	ADCvalue = ADCvalue / NTC_TEMP_ADC_LENGTH;


	ADC_mV = (ADCvalue / 4096.0F) * 3.35F * 1000.0F; //mv 단위

#if ((__NTC_DEBUG__)==1)
		//_LIB_LOGGING_printf("ADC : %d \r\n",(int)ADC_mV);
#endif

	NTC_Temp = _MW_NTC_cal_ntc(ADC_mV); //전압에 맞는 온도값 반환

	return _TRUE;
}

uint8_t _MW_NTC_loop()
{
    uint8_t ret_value = _FALSE;
    static uint32_t print_counter = 0; // 출력 카운터

    if (_TRUE == _MW_NTC_store_value(gADCData[ADC_TEMP_INDEX_])) {
        if (_TRUE == _MW_NTC_cal_temp()) {

            print_counter++; // 호출 횟수 증가

            // 1000번마다 출력
            if (print_counter >= 100) {
                print_counter = 0; // 카운터 리셋

#if ((__NTC_DEBUG__) == 1)
                _LIB_LOGGING_printf("temp : %d \r\n", _MW_NTC_get_temp());
#endif
            }

            ret_value = _TRUE;
        }
    }

    return ret_value;
}




