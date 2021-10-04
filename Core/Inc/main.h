/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.h
  * @brief          : Header for main.c file.
  *                   This file contains the common defines of the application.
  ******************************************************************************
  * @attention
  *
  * <h2><center>&copy; Copyright (c) 2020 STMicroelectronics.
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
#include "stm32f0xx_ll_adc.h"
#include "stm32f0xx_ll_dma.h"
#include "stm32f0xx_ll_iwdg.h"
#include "stm32f0xx_ll_crs.h"
#include "stm32f0xx_ll_rcc.h"
#include "stm32f0xx_ll_bus.h"
#include "stm32f0xx_ll_system.h"
#include "stm32f0xx_ll_exti.h"
#include "stm32f0xx_ll_cortex.h"
#include "stm32f0xx_ll_utils.h"
#include "stm32f0xx_ll_pwr.h"
#include "stm32f0xx_ll_rtc.h"
#include "stm32f0xx_ll_tim.h"
#include "stm32f0xx_ll_usart.h"
#include "stm32f0xx_ll_gpio.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "my_main.h"
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
#define pullsw1_Pin LL_GPIO_PIN_0
#define pullsw1_GPIO_Port GPIOC
#define in1_Pin LL_GPIO_PIN_1
#define in1_GPIO_Port GPIOC
#define in1_EXTI_IRQn EXTI0_1_IRQn
#define pullsw2_Pin LL_GPIO_PIN_2
#define pullsw2_GPIO_Port GPIOC
#define in2_Pin LL_GPIO_PIN_3
#define in2_GPIO_Port GPIOC
#define in2_EXTI_IRQn EXTI2_3_IRQn
#define GSM_PWR_ON_Pin LL_GPIO_PIN_5
#define GSM_PWR_ON_GPIO_Port GPIOA
#define GSM_PWRKEY_Pin LL_GPIO_PIN_6
#define GSM_PWRKEY_GPIO_Port GPIOA
#define ADC_PWR_Pin LL_GPIO_PIN_7
#define ADC_PWR_GPIO_Port GPIOA
#define GNSS_EN_Pin LL_GPIO_PIN_11
#define GNSS_EN_GPIO_Port GPIOB
#define CS_ACCEL_Pin LL_GPIO_PIN_12
#define CS_ACCEL_GPIO_Port GPIOB
#define ONE_WIRE_RX_Pin LL_GPIO_PIN_6
#define ONE_WIRE_RX_GPIO_Port GPIOC
#define ONE_WIRE_RX_EXTI_IRQn EXTI4_15_IRQn
#define ONE_WIRE_TX_Pin LL_GPIO_PIN_7
#define ONE_WIRE_TX_GPIO_Port GPIOC
#define LED_R_Pin LL_GPIO_PIN_8
#define LED_R_GPIO_Port GPIOA
#define LED_G_Pin LL_GPIO_PIN_9
#define LED_G_GPIO_Port GPIOA
#define LED_B_Pin LL_GPIO_PIN_10
#define LED_B_GPIO_Port GPIOA
#define CS_MEM_Pin LL_GPIO_PIN_15
#define CS_MEM_GPIO_Port GPIOA
#define RS485_TX_Pin LL_GPIO_PIN_10
#define RS485_TX_GPIO_Port GPIOC
#define RS485_RX_Pin LL_GPIO_PIN_11
#define RS485_RX_GPIO_Port GPIOC
#define dir_485_Pin LL_GPIO_PIN_2
#define dir_485_GPIO_Port GPIOD
#define GNSS_RST_Pin LL_GPIO_PIN_8
#define GNSS_RST_GPIO_Port GPIOB
#define GNSS_ON_OFF_Pin LL_GPIO_PIN_9
#define GNSS_ON_OFF_GPIO_Port GPIOB
/* USER CODE BEGIN Private defines */

/* USER CODE END Private defines */

#ifdef __cplusplus
}
#endif

#endif /* __MAIN_H */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
