/*
 * _mw_gpio.h
 *
 *  Created on: Jan 6, 2023
 *      Author: MSI
 */

#ifndef MW_INC__MW_GPIO_H_
#define MW_INC__MW_GPIO_H_



#include "main.h"

#define GPO_STUS_LED_01_ON			HAL_GPIO_WritePin(STE_LED1_GPIO_Port, STE_LED1_Pin, GPIO_PIN_RESET)
#define GPO_STUS_LED_01_OFF			HAL_GPIO_WritePin(STE_LED1_GPIO_Port, STE_LED1_Pin, GPIO_PIN_SET)
#define GPO_STUS_LED_02_ON			HAL_GPIO_WritePin(STE_LED2_GPIO_Port, STE_LED2_Pin, GPIO_PIN_RESET)
#define GPO_STUS_LED_02_OFF			HAL_GPIO_WritePin(STE_LED2_GPIO_Port, STE_LED2_Pin, GPIO_PIN_SET)
#define GPO_SLED_R_ON				HAL_GPIO_WritePin(O_RLED_GPIO_Port, O_RLED_Pin, GPIO_PIN_SET)
#define GPO_SLED_R_OFF				HAL_GPIO_WritePin(O_RLED_GPIO_Port, O_RLED_Pin, GPIO_PIN_RESET)
#define GPO_SLED_G_ON				HAL_GPIO_WritePin(O_GLED_GPIO_Port, O_GLED_Pin, GPIO_PIN_SET)
#define GPO_SLED_G_OFF				HAL_GPIO_WritePin(O_GLED_GPIO_Port, O_GLED_Pin, GPIO_PIN_RESET)
#define GPO_SLED_B_ON				HAL_GPIO_WritePin(O_BLED_GPIO_Port, O_BLED_Pin, GPIO_PIN_SET)
#define GPO_SLED_B_OFF			HAL_GPIO_WritePin(O_BLED_GPIO_Port, O_BLED_Pin, GPIO_PIN_RESET)
#if 1
#define GPO_MC_TURN_ON_ENABLE		HAL_GPIO_WritePin(MC1_MCU_GPIO_Port, MC1_MCU_Pin, GPIO_PIN_SET)
#define GPO_MC_TURN_ON_DISABLE		HAL_GPIO_WritePin(MC1_MCU_GPIO_Port, MC1_MCU_Pin, GPIO_PIN_RESET)
#define GPO_MC_TURN_OFF_ENABLE		HAL_GPIO_WritePin(MC2_MCU_GPIO_Port, MC2_MCU_Pin, GPIO_PIN_SET)
#define GPO_MC_TURN_OFF_DISABLE		HAL_GPIO_WritePin(MC2_MCU_GPIO_Port, MC2_MCU_Pin, GPIO_PIN_RESET)
#else
#define GPO_MC_TURN_ON_ENABLE		HAL_GPIO_WritePin(MC2_MCU_GPIO_Port, MC2_MCU_Pin, GPIO_PIN_SET)
#define GPO_MC_TURN_ON_DISABLE		HAL_GPIO_WritePin(MC2_MCU_GPIO_Port, MC2_MCU_Pin, GPIO_PIN_RESET)
#define GPO_MC_TURN_OFF_ENABLE		HAL_GPIO_WritePin(MC1_MCU_GPIO_Port, MC1_MCU_Pin, GPIO_PIN_SET)
#define GPO_MC_TURN_OFF_DISABLE		HAL_GPIO_WritePin(MC1_MCU_GPIO_Port, MC1_MCU_Pin, GPIO_PIN_RESET)
#endif
#define GPO_CP_RY_ON				HAL_GPIO_WritePin(CP_RY_MCU_GPIO_Port, CP_RY_MCU_Pin, GPIO_PIN_SET)
#define GPO_CP_RY_OFF				HAL_GPIO_WritePin(CP_RY_MCU_GPIO_Port, CP_RY_MCU_Pin, GPIO_PIN_RESET)
#define GPO_EEPROM_WP_ENABLE		HAL_GPIO_WritePin(EEPROM_nWP_GPIO_Port, EEPROM_nWP_Pin, GPIO_PIN_RESET)
#define GPO_EEPROM_WP_DISABLE		HAL_GPIO_WritePin(EEPROM_nWP_GPIO_Port, EEPROM_nWP_Pin, GPIO_PIN_SET)

#define GPI_SET_SW_04_IN			HAL_GPIO_ReadPin(DIPSW1_GPIO_Port, DIPSW1_Pin)
#define GPI_SET_SW_03_IN			HAL_GPIO_ReadPin(DIPSW2_GPIO_Port, DIPSW2_Pin)
#define GPI_SET_SW_02_IN			HAL_GPIO_ReadPin(DIPSW3_GPIO_Port, DIPSW3_Pin)
#define GPI_SET_SW_01_IN			HAL_GPIO_ReadPin(DIPSW4_GPIO_Port, DIPSW4_Pin)
#define GPI_WELD_IN					HAL_GPIO_ReadPin(I_WD1_GPIO_Port, I_WD1_Pin)
#define GPI_DEV_IN					HAL_GPIO_ReadPin(I_TEST_GPIO_Port, I_TEST_Pin)
#define GPI_EMG_IN					HAL_GPIO_ReadPin(I_EMG_GPIO_Port, I_EMG_Pin)


#define GPI_DEBOUNCECHECKBUF_LENGTH		4
#define GPI_VALUE_INDEX					((GPI_DEBOUNCECHECKBUF_LENGTH)-(1))


struct s_GPO{
	uint16_t 	STUS_LED_01:1;
	uint16_t 	STUS_LED_02:1;
	uint16_t    SLED_R:1;
	uint16_t    SLED_G:1;
	uint16_t    SLED_B:1;
	uint16_t    MC_TURN_ON:1;
	uint16_t    MC_TURN_OFF:1;
	uint16_t    CP_RY:1;
};

typedef union u__gpio_out{
	uint16_t       all;
	struct s_GPO   bit;
}u__GPIO_OUT;

struct s_GPI{
	uint16_t SET_SW_01:1;
	uint16_t SET_SW_02:1;
	uint16_t SET_SW_03:1;
	uint16_t SET_SW_04:1;
	uint16_t WELD:1;
	uint16_t EMG:1;
};

typedef union u_gpi{
	uint16_t       all;
	struct s_GPI   bit;
}u__GPIO_IN;

void _MW_GPIO_init();
uint8_t _MW_GPIO_set_gpo(uint8_t gpo_num, uint8_t stus);
uint8_t _MW_GPIO_get_gpo(uint8_t gpo_num);
uint8_t _MW_GPIO_get_gpi(uint8_t gpi_num);
void _MW_GPIO_update();



#endif /* MW_INC__MW_GPIO_H_ */
