/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.h
  * @brief          : Header for main.c file.
  *                   This file contains the common defines of the application.
  ******************************************************************************
  * @attention
  *
  * <h2><center>&copy; Copyright (c) 2023 STMicroelectronics.
  * All rights reserved.</center></h2>
  *
  * This software component is licensed by ST under BSD 3-Clause license,
  * the "License"; You may not use this file except in compliance with the
  * License. You may obtain a copy of the License at:
  *                        opensource.org/licenses/BSD-3-Clause
  *
  ******************************************************************************
  */
/* USER CODE END Header */

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __MAIN_H
#define __MAIN_H

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "stm32f0xx_hal.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */

/* USER CODE END Includes */

/* Exported types ------------------------------------------------------------*/
/* USER CODE BEGIN ET */

/* USER CODE END ET */

/* Exported constants --------------------------------------------------------*/
/* USER CODE BEGIN EC */

/* USER CODE END EC */

/* Exported macro ------------------------------------------------------------*/
/* USER CODE BEGIN EM */

/* USER CODE END EM */

/* Exported functions prototypes ---------------------------------------------*/
void Error_Handler(void);

/* USER CODE BEGIN EFP */

/* USER CODE END EFP */

/* Private defines -----------------------------------------------------------*/
#define I_WD1_Pin GPIO_PIN_14
#define I_WD1_GPIO_Port GPIOC
#define O_BLED_Pin GPIO_PIN_0
#define O_BLED_GPIO_Port GPIOA
#define O_GLED_Pin GPIO_PIN_1
#define O_GLED_GPIO_Port GPIOA
#define O_RLED_Pin GPIO_PIN_2
#define O_RLED_GPIO_Port GPIOA
#define MC2_MCU_Pin GPIO_PIN_6
#define MC2_MCU_GPIO_Port GPIOA
#define MC1_MCU_Pin GPIO_PIN_7
#define MC1_MCU_GPIO_Port GPIOA
#define CP_RY_MCU_Pin GPIO_PIN_2
#define CP_RY_MCU_GPIO_Port GPIOB
#define DIPSW4_Pin GPIO_PIN_12
#define DIPSW4_GPIO_Port GPIOB
#define DIPSW3_Pin GPIO_PIN_13
#define DIPSW3_GPIO_Port GPIOB
#define DIPSW2_Pin GPIO_PIN_14
#define DIPSW2_GPIO_Port GPIOB
#define DIPSW1_Pin GPIO_PIN_15
#define DIPSW1_GPIO_Port GPIOB
#define STE_LED2_Pin GPIO_PIN_11
#define STE_LED2_GPIO_Port GPIOA
#define STE_LED1_Pin GPIO_PIN_12
#define STE_LED1_GPIO_Port GPIOA
#define I_TEST_Pin GPIO_PIN_13
#define I_TEST_GPIO_Port GPIOA
#define I_EMG_Pin GPIO_PIN_7
#define I_EMG_GPIO_Port GPIOF
#define EEPROM_nWP_Pin GPIO_PIN_8
#define EEPROM_nWP_GPIO_Port GPIOB
/* USER CODE BEGIN Private defines */

/* USER CODE END Private defines */

#ifdef __cplusplus
}
#endif

#endif /* __MAIN_H */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
