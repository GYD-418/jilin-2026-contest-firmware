/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file    usart.c
  * @brief   This file provides code for the configuration
  *          of the USART instances.
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
/* Includes ------------------------------------------------------------------*/
#include "usart.h"

/* USER CODE BEGIN 0 */
#include "alarm_led.h"
#include "auto_light.h"
#include "buzzer.h"
#include "curtain.h"
#include "dht11.h"
#include "fan_program.h"
#include "irrigation.h"
#include "light_sensor.h"
#include "light_pwm.h"
#include "motor_pwm.h"
#include "motor_speed.h"
#include "pump.h"
#include "servo.h"
#include "gate_access.h"
#include "bluetooth.h"
#include "temperature_sensor.h"
#include "ultrasonic.h"
#include "water_level_alarm.h"
#include "water_control.h"
#include <string.h>
#include <stdio.h>
#include <stdlib.h>

extern volatile DisplayData_t g_display_data;

#define USART_RX_BUFFER_SIZE          32U /* 串口命令接收缓冲区长度 */
#define USART_COMMAND_STEP_PERCENT    10U /* 风扇/灯光单次调节步进 */
#define USART_RX_TIMEOUT_MS          100U /* RX 接收超时刷新 */
static uint8_t usart1_rx_byte = 0U;
static uint8_t usart2_rx_byte = 0U;
static uint8_t usart3_rx_byte = 0U;
static char usart1_rx_buffer[USART_RX_BUFFER_SIZE];
static char usart2_rx_buffer[USART_RX_BUFFER_SIZE];
static char usart3_rx_buffer[USART_RX_BUFFER_SIZE];
static uint8_t usart1_rx_index = 0U;
static uint8_t usart2_rx_index = 0U;
static uint8_t usart3_rx_index = 0U;
static UART_HandleTypeDef *g_printf_uart = &huart2;
static volatile uint32_t g_usart2_last_rx_tick = 0U;
volatile uint8_t g_usart_polling_enabled = 0U; /* 0=关闭蓝牙周期推送 1=开启 */

/* 延迟到任务上下文处理的命令包 */
static volatile uint8_t g_usart1_packet_ready = 0U;
static volatile uint8_t g_usart2_packet_ready = 0U;
static volatile uint8_t g_usart3_packet_ready = 0U;
static char g_usart1_packet_buf[USART_RX_BUFFER_SIZE];
static char g_usart2_packet_buf[USART_RX_BUFFER_SIZE];
static char g_usart3_packet_buf[USART_RX_BUFFER_SIZE];

/* 超时刷新：记录每个 UART 开始接收的时间戳 */
static volatile uint32_t g_usart1_rx_start_tick = 0U;
static volatile uint32_t g_usart2_rx_start_tick = 0U;
static volatile uint32_t g_usart3_rx_start_tick = 0U;

/*
 * 串口通信说明
 *
 * 一、当前工程已接入的外设引脚（与 CubeMX 配置一致）
 * 1. OLED（I2C2）
 *    PB10 -> I2C2_SCL
 *    PB11 -> I2C2_SDA
 *
 * 2. USART1（HC-08 蓝牙透传，9600bps）
 *    PA9  -> USART1_TX
 *    PA10 -> USART1_RX
 *
 * 3. USART2（串口屏，256000bps）
 *    PD5  -> USART2_TX
 *    PD6  -> USART2_RX
 *
 * 4. USART3（闲置，9600bps）
 *    PD8  -> USART3_TX
 *    PD9  -> USART3_RX
 *
 * 5. 风扇 PWM / 测速
 *    PA15 -> TIM2_CH1（PWM）
 *    PA8  -> 测速输入（外部中断）
 *
 * 6. 8 路补光灯
 *    PB3  -> TIM2_CH2 -> LED1
 *    PA2  -> TIM2_CH3 -> LED2
 *    PA3  -> TIM2_CH4 -> LED3
 *    PC7  -> TIM3_CH2 -> LED4
 *    PC8  -> TIM3_CH3 -> LED5
 *    PC9  -> TIM3_CH4 -> LED6
 *    PD12 -> GPIO -> LED7
 *    PD13 -> GPIO -> LED8
 *
 * 7. 舵机/闸门
 *    PC6  -> TIM3_CH1
 *
 * 8. 水泵
 *    PB0  -> shuibeng
 *
 * 9. 卷帘
 *    PB1  -> juanlian_zheng（正转）
 *    PB2  -> juanlian_fan（反转）
 *
 * 10. 灌溉
 *     PA11 -> TIM1_CH4（比例阀 PWM）
 *     PE14 -> guangai（总开关）
 *
 * 11. DHT11
 *     PB12 -> DHT11_DAT
 *
 * 12. 蜂鸣器
 *     PB13 -> fengmingqi
 *
 * 13. 报警灯
 *     PB14 -> baojingdeng
 *
 * 14. 超声波 HC-SR04
 *     PB15 -> TRIG
 *     PB8  -> ECHO（TIM4_CH3 输入捕获）
 *
 * 15. 光照 ADC
 *     PA0  -> ADC1_IN0
 *
 * 16. 温度 NTC
 *     PA1  -> ADC1_IN1
 *
 * 17. 流量计 YF-S401
 *     PC0  -> 外部中断
 *
 * 18. 按键
 *     PE10 -> KEY1（菜单）
 *     PE11 -> KEY2（上）
 *     PE12 -> KEY3（下）
 *     PE13 -> KEY4（返回）
 *
 * 二、串口协议
 * 1. USART1 9600bps（蓝牙），USART2 256000bps（串口屏），USART3 9600bps（闲置）
 * 2. 数据包格式：<命令>KK，KK 是包尾
 * 3. 三条 UART 均可接收命令，回复各回各的
 *
 * 三、支持的命令
 * pollOnKK    -> 开启命令轮询（始终可识别）
 * pollOffKK   -> 关闭命令轮询（始终可识别，关闭后只有 pollOn 能唤醒）
 * fan+KK      -> 风扇速度增加 10%
 * fan-KK      -> 风扇速度减小 10%
 * fanrunKK    -> 开始排气扇转速程控
 * fanstopKK   -> 停止排气扇转速程控
 * L+KK        -> 灯光亮度增加 10%
 * L-KK        -> 灯光亮度减小 10%
 * pumpOnKK    -> 打开水泵
 * pumpOffKK   -> 关闭水泵
 * waterOnKK   -> 打开灌溉
 * waterOffKK  -> 关闭灌溉
 * irri+KK     -> 灌溉强度增加 10%
 * irri-KK     -> 灌溉强度减小 10%
 * irri?KK     -> 读取灌溉状态和占空比
 * buzzOnKK    -> 打开蜂鸣器
 * buzzOffKK   -> 关闭蜂鸣器
 * alarmOnKK   -> 打开报警灯
 * alarmOffKK  -> 关闭报警灯
 * curtain+KK  -> 卷帘正转
 * curtain-KK  -> 卷帘反转
 * curtain0KK  -> 卷帘停止
 * gateOffKK   -> 门禁闸门关闭（0 度）
 * gateOnKK    -> 门禁闸门打开（230 度）
 * light?KK    -> 读取光照原始值、百分比、电压
 * zidongKK    -> 灯光切到自动模式
 * shoudongKK  -> 灯光切到手动模式
 * temp?KK     -> 读取温度原始值、电压、摄氏温度
 * humi?KK     -> 读取 DHT11 湿度
 * dht?KK      -> 读取 DHT11 温湿度
 * dist?KK     -> 读取超声波距离
 * level?KK    -> 读取当前水位
 * wAlarmOnKK  -> 打开水位报警
 * wAlarmOffKK -> 关闭水位报警
 * wAlarm?KK   -> 读取水位报警阈值和状态
 * water+KK    -> 水位报警阈值增加 0.5cm
 * water-KK    -> 水位报警阈值减小 0.5cm
 * status?KK   -> 读取当前系统状态
 *
 * 四、回复示例
 * 风扇转速50%
 * 排气扇程控已经开始
 * 排气扇程控已经停止
 * 灯光亮度 60%
 * 水泵已经打开
 * 水泵已经关闭
 * 灌溉已经打开
 * 灌溉已经关闭
 * 灌溉强度 60%
 * 当前灌溉开，强度 60%
 * 蜂鸣器已经打开
 * 报警灯已经关闭
 * 卷帘正在正转
 * 闸门已经打开
 * 当前光照原始值 1234，亮度 56%，电压 1.23V
 * 灯光已经切到自动模式
 * 灯光已经切到手动模式
 * 当前温度原始值 1479，电压 1.19V，温度 28.6C
 * 当前湿度是 65%
 * 当前温度 27C，当前湿度 65%
 * 当前距离是 16.5cm
 * 当前水位是 1.2cm
 * 水位报警已经打开
 * 水位报警已经关闭
 * 当前水位报警阈值 5.0cm，状态 开
 * 水位报警阈值已经调到 5.5cm
 * 水位报警阈值已经调到 4.5cm
 * 当前状态分两行发送
 * 未识别这条命令
 *
 * 五、排气扇转速程控说明
 * 1. 发送 fanrunKK 后开始执行 60 秒程控
 * 2. 程控目标按时间线性上升：
 *    0~10 秒   -> 0~500 RPM
 *    10~20 秒  -> 500~600 RPM
 *    20~30 秒  -> 600~700 RPM
 *    30~40 秒  -> 700~800 RPM
 *    40~50 秒  -> 800~900 RPM
 *    50~60 秒  -> 900~1000 RPM
 * 3. 只在 0~60 秒正式运行阶段内判报警
 * 4. 当人为操作使实际转速和当前目标转速的绝对偏差超过 20 RPM，并持续一段时间时，
 *    自动打开蜂鸣器和报警灯
 * 5. 正常起步、正常分段跟踪、60 秒后减速停机，这些过程本身不作为报警条件
 * 6. 发送 fanstopKK 后停止程控，并关闭这一路自动报警
 */

static void USART_SendString(UART_HandleTypeDef *huart, const char *string)
{
  if ((huart != NULL) && (string != NULL))
  {
    uint16_t len = (uint16_t)strlen(string);
    uint32_t timeout_ms;

    if (len == 0U)
    {
      return;
    }

    /* 低波特率链路按报文长度给足超时，避免蓝牙回复被 100ms 截断 */
    timeout_ms = (uint32_t)len * 2U + 200U;
    (void)HAL_UART_Transmit(huart, (uint8_t *)string, len, timeout_ms);
  }
}

static UART_HandleTypeDef *USART_GetReplyUart(UART_HandleTypeDef *rx_huart)
{
  if (rx_huart == NULL)
  {
    return &huart1;
  }

  /* 蓝牙(USART1)命令 → 回复走 USART1 返回给手机 APP */
  if (rx_huart->Instance == USART1)
  {
    return &huart1;
  }

  /* USART2(串口屏) / USART3 → 各回各的 */
  return rx_huart;
}

static uint8_t USART_IsBluetoothUart(const UART_HandleTypeDef *huart)
{
  return (uint8_t)((huart != NULL) && (huart->Instance == USART1));
}

static uint8_t USART_IsAllDigits(const char *str, uint8_t len)
{
  uint8_t i;
  if (str == NULL || len == 0U) { return 0U; }
  for (i = 0U; i < len; i++)
  {
    if (str[i] < '0' || str[i] > '9') { return 0U; }
  }
  return 1U;
}

static void USART_ResetBuffer(char *buffer, uint8_t *index)
{
  if ((buffer != NULL) && (index != NULL))
  {
    memset(buffer, 0, USART_RX_BUFFER_SIZE);
    *index = 0U;
  }
}

static void USART_HandlePacket(UART_HandleTypeDef *huart, const char *packet)
{
  uint8_t duty;
  char reply[192];
  UART_HandleTypeDef *reply_huart = USART_GetReplyUart(huart);
  uint16_t voltage_x100;
  uint16_t temperature_x10;
  DHT11_Data_t dht11_data;
  uint16_t distance_x10_cm;
  uint16_t water_level_x10_cm;

  if (strcmp(packet, "ping") == 0)
  {
    USART_SendString(reply_huart, "PONG\r\n");
    return;
  }
  else if (strcmp(packet, "pollOn") == 0)
  {
    g_usart_polling_enabled = 1U;
    USART_SendString(reply_huart,
                     (USART_IsBluetoothUart(reply_huart) != 0U) ? "POLL ON\r\n" : "轮询已开启");
    return;
  }
  else if (strcmp(packet, "pollOff") == 0)
  {
    g_usart_polling_enabled = 0U;
    USART_SendString(reply_huart,
                     (USART_IsBluetoothUart(reply_huart) != 0U) ? "POLL OFF\r\n" : "轮询已关闭");
    return;
  }

  if (strcmp(packet, "fan+") == 0)
  {
    duty = MotorPwm_GetDutyPercent();
    duty = (duty <= (100U - USART_COMMAND_STEP_PERCENT)) ?
           (uint8_t)(duty + USART_COMMAND_STEP_PERCENT) : 100U;
    MotorPwm_SetDutyPercent(duty);
    (void)snprintf(reply, sizeof(reply), "风扇转速%u%%", duty);
    USART_SendString(reply_huart, reply);
  }
  else if (strcmp(packet, "fan-") == 0)
  {
    duty = MotorPwm_GetDutyPercent();
    duty = (duty >= USART_COMMAND_STEP_PERCENT) ?
           (uint8_t)(duty - USART_COMMAND_STEP_PERCENT) : 0U;
    MotorPwm_SetDutyPercent(duty);
    (void)snprintf(reply, sizeof(reply), "风扇转速%u%%", duty);
    USART_SendString(reply_huart, reply);
  }
  else if (strcmp(packet, "fanrun") == 0)
  {
    FanProgram_Start();
    USART_SendString(reply_huart, "排气扇程控已经开始");
  }
  else if (strcmp(packet, "fanstop") == 0)
  {
    FanProgram_Stop();
    USART_SendString(reply_huart, "排气扇程控已经停止");
  }
  else if (strcmp(packet, "L+") == 0)
  {
    if (AutoLight_IsAutoMode() != 0U)
    {
      USART_SendString(reply_huart, "现在是自动模式，灯光会自己调");
      return;
    }
    duty = LightPwm_GetDutyPercent();
    duty = (duty <= (100U - USART_COMMAND_STEP_PERCENT)) ?
           (uint8_t)(duty + USART_COMMAND_STEP_PERCENT) : 100U;
    LightPwm_SetDutyPercent(duty);
    (void)snprintf(reply, sizeof(reply), "灯光亮度%u%%", duty);
    USART_SendString(reply_huart, reply);
  }
  else if (strcmp(packet, "L-") == 0)
  {
    if (AutoLight_IsAutoMode() != 0U)
    {
      USART_SendString(reply_huart, "现在是自动模式，灯光会自己调");
      return;
    }
    duty = LightPwm_GetDutyPercent();
    duty = (duty >= USART_COMMAND_STEP_PERCENT) ?
           (uint8_t)(duty - USART_COMMAND_STEP_PERCENT) : 0U;
    LightPwm_SetDutyPercent(duty);
    (void)snprintf(reply, sizeof(reply), "灯光亮度%u%%", duty);
    USART_SendString(reply_huart, reply);
  }
  else if (strcmp(packet, "pumpOn") == 0)
  {
    WaterControl_SetAutoMode(0U);
    Pump_RequestOn();
    USART_SendString(reply_huart, "水泵已经打开");
  }
  else if (strcmp(packet, "pumpOff") == 0)
  {
    WaterControl_SetAutoMode(0U);
    Pump_RequestOff();
    USART_SendString(reply_huart, "水泵已经关闭");
  }
  else if (strcmp(packet, "waterOn") == 0)
  {
    Irrigation_On();
    (void)snprintf(reply, sizeof(reply), "灌溉已经打开，当前强度%u%%", Irrigation_GetDutyPercent());
    USART_SendString(reply_huart, reply);
  }
  else if (strcmp(packet, "waterOff") == 0)
  {
    Irrigation_Off();
    USART_SendString(reply_huart, "灌溉已经关闭");
  }
  else if (strcmp(packet, "irri+") == 0)
  {
    uint8_t level = Irrigation_GetLevel();
    level = (level < IRRIGATION_LEVEL_MAX) ? (uint8_t)(level + 1U) : IRRIGATION_LEVEL_MAX;
    Irrigation_SetLevel(level);
    (void)snprintf(reply, sizeof(reply), "灌溉强度%s", Irrigation_GetLevelName(level));
    USART_SendString(reply_huart, reply);
  }
  else if (strcmp(packet, "irri-") == 0)
  {
    uint8_t level = Irrigation_GetLevel();
    level = (level > IRRIGATION_LEVEL_OFF) ? (uint8_t)(level - 1U) : IRRIGATION_LEVEL_OFF;
    Irrigation_SetLevel(level);
    (void)snprintf(reply, sizeof(reply), "灌溉强度%s", Irrigation_GetLevelName(level));
    USART_SendString(reply_huart, reply);
  }
  else if (strcmp(packet, "irri?") == 0)
  {
    (void)snprintf(reply, sizeof(reply), "当前灌溉%s，档位%s",
                   (Irrigation_IsOn() != 0U) ? "开" : "关",
                   Irrigation_GetLevelName(Irrigation_GetLevel()));
    USART_SendString(reply_huart, reply);
  }
  else if (strcmp(packet, "buzzOn") == 0)
  {
    Buzzer_On();
    USART_SendString(reply_huart, "蜂鸣器已经打开");
  }
  else if (strcmp(packet, "buzzOff") == 0)
  {
    Buzzer_Off();
    USART_SendString(reply_huart, "蜂鸣器已经关闭");
  }
  else if (strcmp(packet, "alarmOn") == 0)
  {
    AlarmLed_On();
    USART_SendString(reply_huart, "报警灯已经打开");
  }
  else if (strcmp(packet, "alarmOff") == 0)
  {
    AlarmLed_Off();
    USART_SendString(reply_huart, "报警灯已经关闭");
  }
  else if (strcmp(packet, "curtain+") == 0)
  {
    Curtain_Forward();
    USART_SendString(reply_huart, "卷帘正在正转");
  }
  else if (strcmp(packet, "curtain-") == 0)
  {
    Curtain_Reverse();
    USART_SendString(reply_huart, "卷帘正在反转");
  }
  else if (strcmp(packet, "curtain0") == 0)
  {
    Curtain_Stop();
    USART_SendString(reply_huart, "卷帘已经停下");
  }
  else if (strncmp(packet, "auth+", 5) == 0)
  {
    if (GateAccess_VerifyPassword(packet + 5) != 0U)
    {
      USART_SendString(reply_huart, "Auth OK\r\n");
    }
    else
    {
      USART_SendString(reply_huart, "Auth FAIL\r\n");
    }
  }
  else if (strcmp(packet, "gateOff") == 0)
  {
    Servo_GateClose();
    USART_SendString(reply_huart, "闸门已经关闭");
  }
  else if (strcmp(packet, "gateOn") == 0)
  {
    if ((huart != NULL) && (huart->Instance != USART1))
    {
      Servo_GateOpen();
      USART_SendString(reply_huart, "闸门已经打开");
    }
    else if (GateAccess_TryOpenGate() == 0U)
    {
      USART_SendString(reply_huart, "Gate locked. Use auth+passwordKK\r\n");
    }
    else
    {
      USART_SendString(reply_huart, "闸门已经打开");
    }
  }
  else if (strcmp(packet, "light?") == 0)
  {
    /* 使用 defaultTask 缓存的数据，避免重复 ADC 轮询 */
    uint16_t raw = g_display_data.light_raw;
    uint8_t percent = g_display_data.light_percent;
    uint16_t volt = g_display_data.light_voltage_x100;
    (void)snprintf(reply, sizeof(reply), "当前光照原始值%u，亮度%u%%，电压%u.%02uV",
                   raw,
                   percent,
                   volt / 100U,
                   volt % 100U);
    USART_SendString(reply_huart, reply);
  }
  else if (strcmp(packet, "zidong") == 0)
  {
    AutoLight_SetMode(AUTO_LIGHT_MODE_AUTO);
    USART_SendString(reply_huart, "灯光已经切到自动模式");
  }
  else if (strcmp(packet, "shoudong") == 0)
  {
    AutoLight_SetMode(AUTO_LIGHT_MODE_MANUAL);
    USART_SendString(reply_huart, "灯光已经切到手动模式");
  }
  else if (strcmp(packet, "temp?") == 0)
  {
    /* 单次读取原始值，温度用缓存避免重复多采样 ADC */
    uint16_t raw = TemperatureSensor_ReadRaw();
    float voltage = (TEMPERATURE_SENSOR_VREF * (float)raw) / (float)TEMPERATURE_SENSOR_ADC_MAX_VALUE;
    voltage_x100 = (voltage <= 0.0f) ? 0U : (uint16_t)(voltage * 100.0f + 0.5f);
    temperature_x10 = g_display_data.temperature_x10;
    (void)snprintf(reply, sizeof(reply), "当前温度原始值%u，电压%u.%02uV，温度%u.%uC",
                   raw,
                   voltage_x100 / 100U,
                   voltage_x100 % 100U,
                   temperature_x10 / 10U,
                   temperature_x10 % 10U);
    USART_SendString(reply_huart, reply);
  }
  else if (strcmp(packet, "humi?") == 0)
  {
    uint8_t humidity = g_display_data.dht11_humidity;
    if (humidity != 0U || DHT11_GetLastData(&dht11_data) != 0U)
    {
      if (humidity == 0U) { humidity = dht11_data.humidity; }
      (void)snprintf(reply, sizeof(reply), "当前湿度是%u%%", humidity);
    }
    else
    {
      (void)snprintf(reply, sizeof(reply), "湿度暂时没读到");
    }
    USART_SendString(reply_huart, reply);
  }
  else if (strcmp(packet, "dht?") == 0)
  {
    uint8_t humidity = g_display_data.dht11_humidity;
    uint8_t dht_temp = g_display_data.dht11_temperature;
    temperature_x10 = g_display_data.temperature_x10;
    if (humidity != 0U || DHT11_GetLastData(&dht11_data) != 0U)
    {
      if (humidity == 0U)
      {
        humidity = dht11_data.humidity;
        dht_temp = dht11_data.temperature;
      }
      (void)snprintf(reply, sizeof(reply), "当前温度%u.%uC(DHT:%uC)，湿度%u%%",
                     temperature_x10 / 10U,
                     temperature_x10 % 10U,
                     dht_temp,
                     humidity);
    }
    else
    {
      (void)snprintf(reply, sizeof(reply), "温度%u.%uC，湿度暂时没读到",
                     temperature_x10 / 10U,
                     temperature_x10 % 10U);
    }
    USART_SendString(reply_huart, reply);
  }
  else if (strcmp(packet, "dist?") == 0)
  {
    /* 使用缓存数据，避免实时超声波查询的延迟 */
    if (g_display_data.distance_x10_cm != 0U)
    {
      distance_x10_cm = g_display_data.distance_x10_cm;
      (void)snprintf(reply, sizeof(reply), "当前距离是%u.%ucm",
                     distance_x10_cm / 10U,
                     distance_x10_cm % 10U);
    }
    else if (Ultrasonic_HasValidData() != 0U)
    {
      distance_x10_cm = Ultrasonic_GetLastDistanceX10Cm();
      (void)snprintf(reply, sizeof(reply), "当前距离是%u.%ucm",
                     distance_x10_cm / 10U,
                     distance_x10_cm % 10U);
    }
    else
    {
      (void)snprintf(reply, sizeof(reply), "距离暂时没测到");
    }
    USART_SendString(reply_huart, reply);
  }
  else if (strcmp(packet, "level?") == 0)
  {
    /* 使用缓存数据 */
    if (g_display_data.water_level_x10_cm != 0U || g_display_data.distance_x10_cm != 0U)
    {
      water_level_x10_cm = g_display_data.water_level_x10_cm;
      if (water_level_x10_cm == 0U)
      {
        water_level_x10_cm = Ultrasonic_ConvertDistanceToWaterLevelX10Cm(g_display_data.distance_x10_cm);
      }
      (void)snprintf(reply, sizeof(reply), "当前水位是%u.%ucm",
                     water_level_x10_cm / 10U,
                     water_level_x10_cm % 10U);
    }
    else if (Ultrasonic_HasValidData() != 0U)
    {
      distance_x10_cm = Ultrasonic_GetLastDistanceX10Cm();
      water_level_x10_cm = Ultrasonic_ConvertDistanceToWaterLevelX10Cm(distance_x10_cm);
      (void)snprintf(reply, sizeof(reply), "当前水位是%u.%ucm",
                     water_level_x10_cm / 10U,
                     water_level_x10_cm % 10U);
    }
    else
    {
      (void)snprintf(reply, sizeof(reply), "水位暂时算不出来");
    }
    USART_SendString(reply_huart, reply);
  }
  else if (strcmp(packet, "wAlarmOn") == 0)
  {
    WaterLevelAlarm_Enable();
    USART_SendString(reply_huart, "水位报警已经打开");
  }
  else if (strcmp(packet, "wAlarmOff") == 0)
  {
    WaterLevelAlarm_Disable();
    AlarmLed_Off();
    Buzzer_Off();
    USART_SendString(reply_huart, "水位报警已经关闭");
  }
  else if (strcmp(packet, "wAlarm?") == 0)
  {
    uint16_t threshold_x10 = WaterLevelAlarm_GetThresholdX10Cm();
    (void)snprintf(reply, sizeof(reply), "当前水位报警阈值%u.%ucm，状态%s",
                   threshold_x10 / 10U,
                   threshold_x10 % 10U,
                   (WaterLevelAlarm_IsEnabled() != 0U) ? "开" : "关");
    USART_SendString(reply_huart, reply);
  }
  else if (strcmp(packet, "water+") == 0)
  {
    uint16_t threshold_x10 = WaterLevelAlarm_GetThresholdX10Cm();
    threshold_x10 = (threshold_x10 <= 994U) ? (uint16_t)(threshold_x10 + 5U) : 999U;
    WaterLevelAlarm_SetThresholdX10Cm(threshold_x10);
    (void)snprintf(reply, sizeof(reply), "水位报警阈值已经调到%u.%ucm",
                   threshold_x10 / 10U,
                   threshold_x10 % 10U);
    USART_SendString(reply_huart, reply);
  }
  else if (strcmp(packet, "water-") == 0)
  {
    uint16_t threshold_x10 = WaterLevelAlarm_GetThresholdX10Cm();
    threshold_x10 = (threshold_x10 >= 5U) ? (uint16_t)(threshold_x10 - 5U) : 0U;
    WaterLevelAlarm_SetThresholdX10Cm(threshold_x10);
    (void)snprintf(reply, sizeof(reply), "水位报警阈值已经调到%u.%ucm",
                   threshold_x10 / 10U,
                   threshold_x10 % 10U);
    USART_SendString(reply_huart, reply);
  }
  else if (strcmp(packet, "bin?") == 0)
  {
    /* 已由 esp32Task 自动推送二进制帧，此命令保留为空操作 */
  }
  else if (strcmp(packet, "status?") == 0)
  {
    const char *pump_state = (Pump_IsOn() != 0U) ? "开" : "关";
    const char *irrigation_state = Irrigation_GetLevelName(Irrigation_GetLevel());
    const char *alarm_state = (AlarmLed_IsOn() != 0U) ? "开" : "关";
    const char *buzzer_state = (Buzzer_IsOn() != 0U) ? "开" : "关";
    const char *light_mode = (AutoLight_IsAutoMode() != 0U) ? "自动" : "手动";
    const char *fan_program_state = (FanProgram_IsActive() != 0U) ? "运行" : "停止";
    const char *curtain_state;
    char reply_part1[224];
    char reply_part2[192];
    char reply_part3[96];
    char reply_part4[128];
    /* 使用缓存数据，避免重复 ADC/DHT11/超声波 查询 */
    voltage_x100 = g_display_data.light_voltage_x100;
    temperature_x10 = g_display_data.temperature_x10;
    uint8_t humidity = g_display_data.dht11_humidity;
    uint16_t distance_x10 = g_display_data.distance_x10_cm;
    uint16_t water_level_x10 = g_display_data.water_level_x10_cm;

    switch (Curtain_GetState())
    {
      case CURTAIN_STATE_FORWARD:
        curtain_state = "正";
        break;
      case CURTAIN_STATE_REVERSE:
        curtain_state = "反";
        break;
      default:
        curtain_state = "停";
        break;
    }

    (void)snprintf(reply_part1, sizeof(reply_part1),
                   "风扇转速%u%%，风扇程控%s，灯光亮度%u%%，灯光模式%s，舵机%u度，水泵%s，灌溉%s，报警灯%s，蜂鸣器%s，卷帘%s",
                   MotorPwm_GetDutyPercent(),
                   fan_program_state,
                   LightPwm_GetDutyPercent(),
                   light_mode,
                   Servo_GetAngle(),
                   pump_state,
                   irrigation_state,
                   alarm_state,
                   buzzer_state,
                   curtain_state);
    (void)snprintf(reply_part2, sizeof(reply_part2),
                   "当前转速%luRPM，目标转速%uRPM，灌溉%s，光照电压%u.%02uV，温度%u.%uC",
                   (unsigned long)(MotorSpeed_GetRpm() + 0.5f),
                   FanProgram_GetTargetRpm(),
                   Irrigation_GetLevelName(Irrigation_GetLevel()),
                   voltage_x100 / 100U,
                   voltage_x100 % 100U,
                   temperature_x10 / 10U,
                   temperature_x10 % 10U);
    (void)snprintf(reply_part3, sizeof(reply_part3),
                   "H=%u%%",
                   humidity);
    (void)snprintf(reply_part4, sizeof(reply_part4),
                   "距离%u.%ucm，水位%u.%ucm",
                   distance_x10 / 10U,
                   distance_x10 % 10U,
                   water_level_x10 / 10U,
                   water_level_x10 % 10U);
    USART_SendString(reply_huart, reply_part1);
    USART_SendString(reply_huart, "\r\n");
    USART_SendString(reply_huart, reply_part2);
    USART_SendString(reply_huart, "\r\n");
    USART_SendString(reply_huart, reply_part3);
    USART_SendString(reply_huart, "\r\n");
    USART_SendString(reply_huart, reply_part4);
  }
  else if ((strlen(packet) == 18U) && (USART_IsAllDigits(packet, 18U) != 0U))
  {
    /* 排气扇转速程控设置: xxx1xxx2xxx3xxx4xxx5xxx6KK (6段,每段3位数字) */
    uint16_t targets[6];
    uint8_t i;
    for (i = 0U; i < 6U; i++)
    {
      targets[i] = (uint16_t)((packet[i * 3U] - '0') * 100U +
                              (packet[i * 3U + 1U] - '0') * 10U +
                              (packet[i * 3U + 2U] - '0'));
    }
    FanProgram_SetTargets(targets);
    (void)snprintf(reply, sizeof(reply),
                   "转速程控已设置: %u/%u/%u/%u/%u/%u RPM",
                   targets[0], targets[1], targets[2],
                   targets[3], targets[4], targets[5]);
    USART_SendString(reply_huart, reply);
  }
  else
  {
    USART_SendString(reply_huart,
                     (USART_IsBluetoothUart(reply_huart) != 0U) ? "UNKNOWN\r\n" : "这条命令我没认出来");
  }
}

/* USER CODE END 0 */

UART_HandleTypeDef huart1;
UART_HandleTypeDef huart2;
UART_HandleTypeDef huart3;

/* USART1 init function */

void MX_USART1_UART_Init(void)
{

  /* USER CODE BEGIN USART1_Init 0 */

  /* USER CODE END USART1_Init 0 */

  /* USER CODE BEGIN USART1_Init 1 */

  /* USER CODE END USART1_Init 1 */
  huart1.Instance = USART1;
  huart1.Init.BaudRate = 115200;
  huart1.Init.WordLength = UART_WORDLENGTH_8B;
  huart1.Init.StopBits = UART_STOPBITS_1;
  huart1.Init.Parity = UART_PARITY_NONE;
  huart1.Init.Mode = UART_MODE_TX_RX;
  huart1.Init.HwFlowCtl = UART_HWCONTROL_NONE;
  huart1.Init.OverSampling = UART_OVERSAMPLING_16;
  if (HAL_UART_Init(&huart1) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN USART1_Init 2 */
  USART_ResetBuffer(usart1_rx_buffer, &usart1_rx_index);
  USART1_StartReceiveIT();

  /* USER CODE END USART1_Init 2 */

}
/* USART2 init function */

void MX_USART2_UART_Init(void)
{

  /* USER CODE BEGIN USART2_Init 0 */

  /* USER CODE END USART2_Init 0 */

  /* USER CODE BEGIN USART2_Init 1 */

  /* USER CODE END USART2_Init 1 */
  huart2.Instance = USART2;
  huart2.Init.BaudRate = 256000;
  huart2.Init.WordLength = UART_WORDLENGTH_8B;
  huart2.Init.StopBits = UART_STOPBITS_1;
  huart2.Init.Parity = UART_PARITY_NONE;
  huart2.Init.Mode = UART_MODE_TX_RX;
  huart2.Init.HwFlowCtl = UART_HWCONTROL_NONE;
  huart2.Init.OverSampling = UART_OVERSAMPLING_16;
  if (HAL_UART_Init(&huart2) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN USART2_Init 2 */
  USART_ResetBuffer(usart2_rx_buffer, &usart2_rx_index);
  USART2_StartReceiveIT();

  /* USER CODE END USART2_Init 2 */

}
/* USART3 init function */

void MX_USART3_UART_Init(void)
{

  /* USER CODE BEGIN USART3_Init 0 */

  /* USER CODE END USART3_Init 0 */

  /* USER CODE BEGIN USART3_Init 1 */

  /* USER CODE END USART3_Init 1 */
  huart3.Instance = USART3;
  huart3.Init.BaudRate = 115200;
  huart3.Init.WordLength = UART_WORDLENGTH_8B;
  huart3.Init.StopBits = UART_STOPBITS_1;
  huart3.Init.Parity = UART_PARITY_NONE;
  huart3.Init.Mode = UART_MODE_TX_RX;
  huart3.Init.HwFlowCtl = UART_HWCONTROL_NONE;
  huart3.Init.OverSampling = UART_OVERSAMPLING_16;
  if (HAL_UART_Init(&huart3) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN USART3_Init 2 */
  USART_ResetBuffer(usart3_rx_buffer, &usart3_rx_index);
  USART3_StartReceiveIT();

  /* USER CODE END USART3_Init 2 */

}

void HAL_UART_MspInit(UART_HandleTypeDef* uartHandle)
{

  GPIO_InitTypeDef GPIO_InitStruct = {0};
  if(uartHandle->Instance==USART1)
  {
  /* USER CODE BEGIN USART1_MspInit 0 */

  /* USER CODE END USART1_MspInit 0 */
    /* USART1 clock enable */
    __HAL_RCC_USART1_CLK_ENABLE();

    __HAL_RCC_GPIOA_CLK_ENABLE();
    /**USART1 GPIO Configuration
    PA9     ------> USART1_TX
    PA10     ------> USART1_RX
    */
    GPIO_InitStruct.Pin = USART1_TX_Pin|USART1_RX_Pin;
    GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
    GPIO_InitStruct.Alternate = GPIO_AF7_USART1;
    HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

    /* USART1 interrupt Init */
    HAL_NVIC_SetPriority(USART1_IRQn, 5, 0);
    HAL_NVIC_EnableIRQ(USART1_IRQn);
  /* USER CODE BEGIN USART1_MspInit 1 */

  /* USER CODE END USART1_MspInit 1 */
  }
  else if(uartHandle->Instance==USART2)
  {
  /* USER CODE BEGIN USART2_MspInit 0 */

  /* USER CODE END USART2_MspInit 0 */
    /* USART2 clock enable */
    __HAL_RCC_USART2_CLK_ENABLE();

    __HAL_RCC_GPIOD_CLK_ENABLE();
    /**USART2 GPIO Configuration
    PD5     ------> USART2_TX
    PD6     ------> USART2_RX
    */
    GPIO_InitStruct.Pin = USART2_TX_Pin|USART2_RX_Pin;
    GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
    GPIO_InitStruct.Alternate = GPIO_AF7_USART2;
    HAL_GPIO_Init(GPIOD, &GPIO_InitStruct);

    /* USART2 interrupt Init */
    HAL_NVIC_SetPriority(USART2_IRQn, 5, 0);
    HAL_NVIC_EnableIRQ(USART2_IRQn);
  /* USER CODE BEGIN USART2_MspInit 1 */

  /* USER CODE END USART2_MspInit 1 */
  }
  else if(uartHandle->Instance==USART3)
  {
  /* USER CODE BEGIN USART3_MspInit 0 */

  /* USER CODE END USART3_MspInit 0 */
    /* USART3 clock enable */
    __HAL_RCC_USART3_CLK_ENABLE();

    __HAL_RCC_GPIOD_CLK_ENABLE();
    /**USART3 GPIO Configuration
    PD8     ------> USART3_TX
    PD9     ------> USART3_RX
    */
    GPIO_InitStruct.Pin = GPIO_PIN_8|GPIO_PIN_9;
    GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
    GPIO_InitStruct.Alternate = GPIO_AF7_USART3;
    HAL_GPIO_Init(GPIOD, &GPIO_InitStruct);

  /* USER CODE BEGIN USART3_MspInit 1 */

  /* USER CODE END USART3_MspInit 1 */
  }
}

void HAL_UART_MspDeInit(UART_HandleTypeDef* uartHandle)
{

  if(uartHandle->Instance==USART1)
  {
  /* USER CODE BEGIN USART1_MspDeInit 0 */

  /* USER CODE END USART1_MspDeInit 0 */
    /* Peripheral clock disable */
    __HAL_RCC_USART1_CLK_DISABLE();

    /**USART1 GPIO Configuration
    PA9     ------> USART1_TX
    PA10     ------> USART1_RX
    */
    HAL_GPIO_DeInit(GPIOA, USART1_TX_Pin|USART1_RX_Pin);

    /* USART1 interrupt Deinit */
    HAL_NVIC_DisableIRQ(USART1_IRQn);
  /* USER CODE BEGIN USART1_MspDeInit 1 */

  /* USER CODE END USART1_MspDeInit 1 */
  }
  else if(uartHandle->Instance==USART2)
  {
  /* USER CODE BEGIN USART2_MspDeInit 0 */

  /* USER CODE END USART2_MspDeInit 0 */
    /* Peripheral clock disable */
    __HAL_RCC_USART2_CLK_DISABLE();

    /**USART2 GPIO Configuration
    PD5     ------> USART2_TX
    PD6     ------> USART2_RX
    */
    HAL_GPIO_DeInit(GPIOD, USART2_TX_Pin|USART2_RX_Pin);

    /* USART2 interrupt Deinit */
    HAL_NVIC_DisableIRQ(USART2_IRQn);
  /* USER CODE BEGIN USART2_MspDeInit 1 */

  /* USER CODE END USART2_MspDeInit 1 */
  }
  else if(uartHandle->Instance==USART3)
  {
  /* USER CODE BEGIN USART3_MspDeInit 0 */

  /* USER CODE END USART3_MspDeInit 0 */
    /* Peripheral clock disable */
    __HAL_RCC_USART3_CLK_DISABLE();

    /**USART3 GPIO Configuration
    PD8     ------> USART3_TX
    PD9     ------> USART3_RX
    */
    HAL_GPIO_DeInit(GPIOD, GPIO_PIN_8|GPIO_PIN_9);

  /* USER CODE BEGIN USART3_MspDeInit 1 */

  /* USER CODE END USART3_MspDeInit 1 */
  }
}

/* USER CODE BEGIN 1 */
void USART1_StartReceiveIT(void)
{
  (void)HAL_UART_Receive_IT(&huart1, &usart1_rx_byte, 1U);
}

void USART2_StartReceiveIT(void)
{
  (void)HAL_UART_Receive_IT(&huart2, &usart2_rx_byte, 1U);
}

void USART3_StartReceiveIT(void)
{
  (void)HAL_UART_Receive_IT(&huart3, &usart3_rx_byte, 1U);
}

void USART_SetPrintfUart(UART_HandleTypeDef *huart)
{
  if (huart != NULL)
  {
    g_printf_uart = huart;
  }
}

uint32_t USART2_GetLastRxTick(void)
{
  return g_usart2_last_rx_tick;
}

int __io_putchar(int ch)
{
  uint8_t data = (uint8_t)ch;

  if (g_printf_uart == NULL)
  {
    g_printf_uart = &huart2;
  }

  (void)HAL_UART_Transmit(g_printf_uart, &data, 1U, 100U);
  return ch;
}

static void USART_ProcessByte(UART_HandleTypeDef *huart, uint8_t byte, char *buffer, uint8_t *index)
{
  if ((byte == '\r') || (byte == '\n') || (byte == ' ') ||
      (byte == 0x00U) || (byte == 0xFFU) ||
      (buffer == NULL) || (index == NULL))
  {
    return;
  }

  /* 第一个字节时记录时间戳 */
  if (*index == 0U)
  {
    uint32_t now = HAL_GetTick();
    if (huart->Instance == USART1)
    {
      g_usart1_rx_start_tick = now;
    }
    else if (huart->Instance == USART2)
    {
      g_usart2_rx_start_tick = now;
    }
    else if (huart->Instance == USART3)
    {
      g_usart3_rx_start_tick = now;
    }
  }

  if (*index >= (USART_RX_BUFFER_SIZE - 1U))
  {
    USART_ResetBuffer(buffer, index);
  }

  buffer[(*index)++] = (char)byte;
  buffer[*index] = '\0';

  if ((*index >= 2U) &&
      (buffer[*index - 2U] == 'K') &&
      (buffer[*index - 1U] == 'K'))
  {
    buffer[*index - 2U] = '\0';

    /* 复制到独立缓冲区，设置就绪标志，由任务上下文处理 */
    if (huart->Instance == USART1)
    {
      memcpy(g_usart1_packet_buf, buffer, USART_RX_BUFFER_SIZE);
      g_usart1_packet_ready = 1U;
    }
    else if (huart->Instance == USART2)
    {
      memcpy(g_usart2_packet_buf, buffer, USART_RX_BUFFER_SIZE);
      g_usart2_packet_ready = 1U;
    }
    else if (huart->Instance == USART3)
    {
      memcpy(g_usart3_packet_buf, buffer, USART_RX_BUFFER_SIZE);
      g_usart3_packet_ready = 1U;
    }

    USART_ResetBuffer(buffer, index);
  }
}

void USART_PollPackets(void)
{
  uint32_t now = HAL_GetTick();

  if (g_usart1_packet_ready != 0U)
  {
    g_usart1_packet_ready = 0U;
    USART_HandlePacket(&huart1, g_usart1_packet_buf);
  }

  if (g_usart2_packet_ready != 0U)
  {
    g_usart2_packet_ready = 0U;
    USART_HandlePacket(&huart2, g_usart2_packet_buf);
  }

  if (g_usart3_packet_ready != 0U)
  {
    g_usart3_packet_ready = 0U;
    USART_HandlePacket(&huart3, g_usart3_packet_buf);
  }

  /* 超时刷新：未成包的残数据超过 500ms 则丢弃 */
  if ((usart1_rx_index > 0U) && ((now - g_usart1_rx_start_tick) > USART_RX_TIMEOUT_MS))
  {
    USART_ResetBuffer(usart1_rx_buffer, &usart1_rx_index);
  }
  if ((usart2_rx_index > 0U) && ((now - g_usart2_rx_start_tick) > USART_RX_TIMEOUT_MS))
  {
    USART_ResetBuffer(usart2_rx_buffer, &usart2_rx_index);
  }
  if ((usart3_rx_index > 0U) && ((now - g_usart3_rx_start_tick) > USART_RX_TIMEOUT_MS))
  {
    USART_ResetBuffer(usart3_rx_buffer, &usart3_rx_index);
  }
}

void USART_FlushRxBuffer(UART_HandleTypeDef *huart)
{
  if (huart == NULL)
  {
    return;
  }

  if (huart->Instance == USART1)
  {
    USART_ResetBuffer(usart1_rx_buffer, &usart1_rx_index);
  }
  else if (huart->Instance == USART2)
  {
    USART_ResetBuffer(usart2_rx_buffer, &usart2_rx_index);
  }
  else if (huart->Instance == USART3)
  {
    USART_ResetBuffer(usart3_rx_buffer, &usart3_rx_index);
  }
}

void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart)
{
  if (huart->Instance == USART1)
  {
    Bluetooth_OnDataReceived();
    USART_ProcessByte(&huart1, usart1_rx_byte, usart1_rx_buffer, &usart1_rx_index);
    USART1_StartReceiveIT();
  }
  else if (huart->Instance == USART2)
  {
    g_usart2_last_rx_tick = HAL_GetTick();
    USART_ProcessByte(&huart2, usart2_rx_byte, usart2_rx_buffer, &usart2_rx_index);
    USART2_StartReceiveIT();
  }
  else if (huart->Instance == USART3)
  {
    USART_ProcessByte(&huart3, usart3_rx_byte, usart3_rx_buffer, &usart3_rx_index);
    USART3_StartReceiveIT();
  }
}

/* USER CODE END 1 */
