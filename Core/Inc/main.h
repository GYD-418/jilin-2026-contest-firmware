/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.h
  * @brief          : Header for main.c file.
  *                   This file contains the common defines of the application.
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2026 STMicroelectronics.
  * All rights reserved.
  *
  * This software is licensed under terms that can be found in the LICENSE file
  * in the root directory of this software component.
  * If no LICENSE file comes with this software, it is provided AS-IS.
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
#include "stm32f4xx_hal.h"

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
#define liuliang_Pin GPIO_PIN_0
#define liuliang_GPIO_Port GPIOC
#define liuliang_EXTI_IRQn EXTI0_IRQn
#define guangzhao_Pin GPIO_PIN_0
#define guangzhao_GPIO_Port GPIOA
#define shuiwei_Pin GPIO_PIN_1
#define shuiwei_GPIO_Port GPIOA
#define TIM2CH3_LED3_Pin GPIO_PIN_2
#define TIM2CH3_LED3_GPIO_Port GPIOA
#define TIM2CH4_LED4_Pin GPIO_PIN_3
#define TIM2CH4_LED4_GPIO_Port GPIOA
#define SERVO_Signal_Pin GPIO_PIN_6
#define SERVO_Signal_GPIO_Port GPIOA
#define shuibeng_Pin GPIO_PIN_0
#define shuibeng_GPIO_Port GPIOB
#define juanlian_zheng_Pin GPIO_PIN_1
#define juanlian_zheng_GPIO_Port GPIOB
#define key2_Pin GPIO_PIN_11
#define key2_GPIO_Port GPIOE
#define key3_Pin GPIO_PIN_12
#define key3_GPIO_Port GPIOE
#define key4_Pin GPIO_PIN_13
#define key4_GPIO_Port GPIOE
#define guangai_Pin GPIO_PIN_14
#define guangai_GPIO_Port GPIOE
#define I2C2_SCL_Pin GPIO_PIN_10
#define I2C2_SCL_GPIO_Port GPIOB
#define I2C2_SDA_Pin GPIO_PIN_11
#define I2C2_SDA_GPIO_Port GPIOB
#define DHT11_DAT_Pin GPIO_PIN_12
#define DHT11_DAT_GPIO_Port GPIOB
#define fengmingqi_Pin GPIO_PIN_13
#define fengmingqi_GPIO_Port GPIOB
#define baojingdeng_Pin GPIO_PIN_14
#define baojingdeng_GPIO_Port GPIOB
#define TRIG_Pin GPIO_PIN_15
#define TRIG_GPIO_Port GPIOB
#define TIM3CH2_LED1_Pin GPIO_PIN_7
#define TIM3CH2_LED1_GPIO_Port GPIOC
#define TIM3CH2_LED5_Pin GPIO_PIN_8
#define TIM3CH2_LED5_GPIO_Port GPIOC
#define TIM3_CH2_LED6_Pin GPIO_PIN_9
#define TIM3_CH2_LED6_GPIO_Port GPIOC
#define cesu_Pin GPIO_PIN_8
#define cesu_GPIO_Port GPIOA
#define cesu_EXTI_IRQn EXTI9_5_IRQn
#define USART1_TX_Pin GPIO_PIN_9
#define USART1_TX_GPIO_Port GPIOA
#define USART1_RX_Pin GPIO_PIN_10
#define USART1_RX_GPIO_Port GPIOA
#define TIM1CH4_guangai_Pin GPIO_PIN_11
#define TIM1CH4_guangai_GPIO_Port GPIOA
#define TIM2CH1_fengshan_Pin GPIO_PIN_15
#define TIM2CH1_fengshan_GPIO_Port GPIOA
#define USART2_TX_Pin GPIO_PIN_5
#define USART2_TX_GPIO_Port GPIOD
#define USART2_RX_Pin GPIO_PIN_6
#define USART2_RX_GPIO_Port GPIOD
#define TIM2CH2_LED2_Pin GPIO_PIN_3
#define TIM2CH2_LED2_GPIO_Port GPIOB
#define ECHO_Pin GPIO_PIN_4
#define ECHO_GPIO_Port GPIOB

/* USER CODE BEGIN Private defines */
#define juanlian_fan_Pin GPIO_PIN_2
#define juanlian_fan_GPIO_Port GPIOB
#define TIM4_CH1_LED7_Pin GPIO_PIN_12
#define TIM4_CH1_LED7_GPIO_Port GPIOD
#define TIM4_CH2_LED8_Pin GPIO_PIN_13
#define TIM4_CH2_LED8_GPIO_Port GPIOD
/* 舵机引脚：TIM3_CH1 → PA6，替代原 PC6 */
#define SERVO_Signal_Pin GPIO_PIN_6
#define SERVO_Signal_GPIO_Port GPIOA
/* CubeMX 生成 TIM3_CH2_LED6（有下划线），兼容旧名 */
#define TIM3CH2_LED6_Pin GPIO_PIN_9
#define TIM3CH2_LED6_GPIO_Port GPIOC
/* USER CODE END Private defines */

#ifdef __cplusplus
}
#endif

#endif /* __MAIN_H */
