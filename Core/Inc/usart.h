/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file    usart.h
  * @brief   This file contains all the function prototypes for
  *          the usart.c file
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
#ifndef __USART_H__
#define __USART_H__

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "main.h"

/* USER CODE BEGIN Includes */
#include <stdint.h>

typedef struct
{
  uint16_t motor_rpm;
  uint8_t light_duty;
  uint16_t light_voltage_x100;
  uint16_t light_raw;
  uint8_t light_percent;
  uint16_t temperature_x10;
  uint8_t dht11_humidity;
  uint8_t dht11_temperature;
  uint16_t distance_x10_cm;
  uint16_t water_level_x10_cm;
  uint16_t flow_ml_per_min;
  uint8_t pump_on;
  uint8_t buzzer_on;
  uint8_t alarm_on;
  uint8_t gate_open;
} DisplayData_t;

/* USER CODE END Includes */

extern UART_HandleTypeDef huart1;

extern UART_HandleTypeDef huart2;

extern UART_HandleTypeDef huart3;

/* USER CODE BEGIN Private defines */

/* USER CODE END Private defines */

void MX_USART1_UART_Init(void);
void MX_USART2_UART_Init(void);
void MX_USART3_UART_Init(void);

/* USER CODE BEGIN Prototypes */
void USART1_StartReceiveIT(void);
void USART2_StartReceiveIT(void);
void USART3_StartReceiveIT(void);
void USART_SetPrintfUart(UART_HandleTypeDef *huart);
uint32_t USART2_GetLastRxTick(void);
void USART_PollPackets(void);
void USART_FlushRxBuffer(UART_HandleTypeDef *huart);
extern volatile uint8_t g_usart_polling_enabled;

/* USER CODE END Prototypes */

#ifdef __cplusplus
}
#endif

#endif /* __USART_H__ */

