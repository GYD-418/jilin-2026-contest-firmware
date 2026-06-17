/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * File Name          : freertos.c
  * Description        : Code for freertos applications
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
#include "FreeRTOS.h"
#include "task.h"
#include "main.h"
#include "cmsis_os.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "alarm_led.h"
#include "auto_light.h"
#include "buzzer.h"
#include "curtain.h"
#include "dht11.h"
#include "fan_program.h"
#include "flow_sensor.h"
#include "irrigation.h"
#include "light_sensor.h"
#include "light_pwm.h"
#include "motor_pwm.h"
#include "motor_speed.h"
#include "oled.h"
#include "pump.h"
#include "servo.h"
#include "temperature_sensor.h"
#include "ultrasonic.h"
#include "usart.h"
#include "bluetooth.h"
#include "water_level_alarm.h"
#include "button.h"
#include "gate_access.h"
#include "output_fault.h"
#include "water_control.h"
#include <stdarg.h>
#include <stdio.h>

/*
 * 当前工程总说明
 *
 * 一、已使用引脚与功能对应
 * 1. OLED 显示屏（I2C1）
 *    PB6  -> I2C1_SCL
 *    PB7  -> I2C1_SDA
 *
 * 2. 串口1（HC-08 蓝牙透传，9600bps）
 *    PA9  -> USART1_TX
 *    PA10 -> USART1_RX
 *
 * 3. 串口2（串口屏，256000bps）
 *    PD5  -> USART2_TX
 *    PD6  -> USART2_RX
 *
 * 4. 串口3（闲置，9600bps）
 *    PD8  -> USART3_TX
 *    PD9  -> USART3_RX
 *
 * 5. 风扇 PWM
 *    PA15 -> TIM2_CH1
 *
 * 6. 8 路补光灯（10 档逻辑）
 *    PB3  -> TIM2_CH2 -> LED1
 *    PA2  -> TIM2_CH3 -> LED2
 *    PA3  -> TIM2_CH4 -> LED3
 *    PC7  -> TIM3_CH2 -> LED4
 *    PC8  -> TIM3_CH3 -> LED5
 *    PC9  -> TIM3_CH4 -> LED6
 *    PD12 -> GPIO     -> LED7
 *    PD13 -> GPIO     -> LED8
 *
 * 7. 舵机 / 闸门
 *    PC6  -> TIM3_CH1（0 度关，230 度开）
 *
 * 8. 风扇测速
 *    PA8  -> cesu（外部中断计脉冲，78 脉冲/转）
 *
 * 9. 水泵
 *    PB0  -> shuibeng
 *
 * 10. 卷帘
 *     PB1  -> juanlian_zheng（正转）
 *     PB2  -> juanlian_fan（反转）
 *
 * 11. 灌溉
 *     PA11 -> TIM1_CH4（比例阀 PWM）
 *     PE14 -> guangai（总开关 GPIO）
 *
 * 12. DHT11（仅湿度）
 *     PB12 -> DHT11_DAT
 *
 * 13. 蜂鸣器
 *     PB13 -> fengmingqi（翻转驱动）
 *
 * 14. 报警灯
 *     PB14 -> baojingdeng
 *
 * 15. 超声波 HC-SR04
 *     PB15 -> TRIG
 *     PB8  -> ECHO（TIM4_CH3 输入捕获，1MHz 计时）
 *     校准参数：48 us/cm，水位基准 11.3 cm
 *
 * 16. 光照（模拟量）
 *     PA0  -> guangzhao / ADC1_IN0（0~3.3V）
 *
 * 17. 温度 NTC（模拟量）
 *     PA1  -> shuiwei / ADC1_IN1（10kΩ B=3950，偏置 -12℃）
 *
 * 18. 流量 YF-S401
 *     PC0  -> liuliang（外部中断，588 脉冲/L）
 *
 * 19. 按键（消抖 30ms，长按 800ms，连发 200ms）
 *     PE10 -> KEY1（菜单/选择）
 *     PE11 -> KEY2（上/增加）
 *     PE12 -> KEY3（下/减少）
 *     PE13 -> KEY4（返回/取消）
 *
 * 二、当前主要功能模块
 * 1.  风扇：PWM 调速 + 测速 + 60 秒 6 段程控（闭环 PID，偏差报警）
 * 2.  灯光：手动 / 自动补光（光照传感器反馈），10 档光柱
 * 3.  水泵：继电器开/关，带舵机保护序列（RequestOn/Off）
 * 4.  灌溉：4 档离散（OFF/LOW/MED/HIGH，0/33/66/100% PWM）+ 总开关
 * 5.  卷帘：正转 / 反转 / 停止（双 GPIO H 桥）
 * 6.  闸门：舵机开/关 + 密码授权（默认 123456，30 秒超时）
 * 7.  蜂鸣器：翻转驱动 + 手动/自动双源（AutoOn 不覆盖手动）
 * 8.  报警灯：手动/自动双源
 * 9.  水位报警：阈值可调，超限自动触发蜂鸣器+报警灯
 * 10. 水位自控：滞后控制水泵自动启停，5 分钟超时保护，流量异常检测
 * 11. 输出故障检测：8 路故障（风扇/水泵/灌溉/卷帘/蜂鸣器/报警灯/补光/流量泄漏）
 * 12. 蓝牙推送：每 2 秒推送温湿度/水位/光照/风扇/灌溉/故障到手机 APP
 * 13. OLED 菜单：按键导航，灌溉/水位/灯光/门禁/故障/系统 6 个子页
 * 14. 传感器：光照电压、NTC 温度、DHT11 湿度、超声波距离/水位、风扇 RPM、流量
 *
 * 三、灯光 10 档光柱逻辑
 * 奇数档新增 LED 亮 50%，偶数档亮 100%；第 9 档全部 75%，第 10 档全部 100%
 * 1.  10%  -> LED1=50%
 * 2.  20%  -> LED1=100%
 * 3.  30%  -> LED1=100%, LED2=50%
 * 4.  40%  -> LED1~2=100%
 * 5.  50%  -> LED1~2=100%, LED3=50%
 * 6.  60%  -> LED1~3=100%
 * 7.  70%  -> LED1~3=100%, LED4=50%
 * 8.  80%  -> LED1~4=100%
 * 9.  90%  -> LED1~8 全部 75%
 * 10. 100% -> LED1~8 全部 100%
 *
 * 四、串口协议（三条 UART 均可接收命令）
 * 1. 数据包格式：<命令>KK，以 KK 结尾
 * 2. 回复路由：USART1(蓝牙)→USART1 | USART2(串口屏)→USART2 | USART3→USART3
 * 3. printf 输出定向到 USART2（供串口屏调试）
 *
 * 五、全部串口命令（38 条）
 * 1. 风扇
 *    fan+KK / fan-KK / fanrunKK / fanstopKK
 *
 * 2. 灯光
 *    L+KK / L-KK / zidongKK / shoudongKK / light?KK
 *
 * 3. 水泵 / 灌溉
 *    pumpOnKK / pumpOffKK
 *    irri+KK / irri-KK / irri?KK
 *    waterOnKK / waterOffKK
 *
 * 4. 蜂鸣器 / 报警灯
 *    buzzOnKK / buzzOffKK
 *    alarmOnKK / alarmOffKK
 *
 * 5. 卷帘
 *    curtain+KK / curtain-KK / curtain0KK
 *
 * 6. 闸门（gateOn 需先 auth+密码 授权）
 *    auth+123456KK / gateOnKK / gateOffKK
 *
 * 7. 传感器查询
 *    temp?KK / humi?KK / dht?KK / dist?KK / level?KK
 *
 * 8. 水位报警
 *    wAlarmOnKK / wAlarmOffKK / wAlarm?KK
 *    water+KK / water-KK
 *
 * 9. 系统
 *    status?KK
 *
 * 六、FreeRTOS 任务分工
 * 1. defaultTask（Normal）
 *    - 所有传感器采集（DHT11 1.2s / 超声波 25ms / ADC 按需）
 *    - 执行器状态机（水泵保护序列 / 风扇程控 / 自动补光）
 *    - 水位报警 / 水位自控 / 输出故障检测
 *    - OLED 刷新（50ms，菜单激活时自动跳过）
 *    - 串口屏数据发送（120ms 5 相轮发）
 *    - 蓝牙状态推送（2000ms）
 *
 * 2. displayTask（BelowNormal）
 *    - 按键轮询（50ms）
 *    - OLED 菜单状态机（IDLE→主菜单→6 个子页）
 *    - 灌溉子页中 KEY2/KEY3 直接调节 4 档级别
 *    - 菜单激活时置位 g_menu_active，defaultTask 暂停 OLED 刷新
 *
 * 3. buzzerTask（Low）
 *    - 每 1ms 调用 Buzzer_Process() 翻转 GPIO 产生 ~1kHz 方波
 *
 * 七、USART2 串口屏通信（PD5/PD6, 256000bps）
 *
 * 1. 发送数据（MCU→串口屏，26 条显示指令，分 5 相每 120ms 轮发）
 *
 *   【相位 0】波形 s0（4 通道，各占 1/4 高度）
 *     add s0.id,0,<v>  — 风扇 RPM（150~195）
 *     add s0.id,1,<v>  — 灯光亮度%（100~145）
 *     add s0.id,2,<v>  — 水泵开关（50~95）
 *     add s0.id,3,<v>  — 蜂鸣器开关（0~45）
 *
 *   【相位 1】波形 s1（4 通道，各占 1/4 高度）
 *     add s1.id,0,<v>  — 报警灯 0/1（150~195）
 *     add s1.id,1,<v>  — 闸门 0/1（100~145）
 *     add s1.id,2,<v>  — 光照电压×100（50~95）
 *
 *   【相位 2】波形 s1 通道 3 + 波形 s2（3 通道，各占 1/3 高度）+ 文本
 *     add s1.id,3,<v>  — 温度×10（0~45）
 *     add s2.id,0,<v>  — 湿度%（133~193）
 *     add s2.id,1,<v>  — 水位×10cm（66~126）
 *     add s2.id,2,<v>  — 流量 ml/min（0~60）
 *     page0.t11.txt    — 风扇实时 RPM
 *     page0.t12.txt    — 灯光亮度%
 *
 *   【相位 3】执行器状态文本
 *     page0.t13.txt    — 灌溉 OPEN/CLOSE
 *     page0.t14.txt    — 水泵 OPEN/CLOSE
 *     page0.t15.txt    — 报警灯 OPEN/CLOSE
 *     page0.t16.txt    — 闸门 OPEN/CLOSE
 *     page0.t33.txt    — 卷帘 OPEN/CLOSE/STOP
 *     page0.t34.txt    — 蜂鸣器 OPEN/CLOSE
 *
 *   【相位 4】传感器数值文本
 *     page0.t20.txt    — 光照电压 x.xxV
 *     page0.t9.txt     — 温度 xx.xC
 *     page0.t10.txt    — 湿度 xx%
 *     page0.t21.txt    — 当前水位 xx.xcm
 *     page0.t36.txt    — 水位报警阈值 xx.xcm
 *     page0.t37.txt    — 风扇程控 T:xxxs,RPM:xxx
 *
 * 2. 接收指令（串口屏→MCU，全部 38 条命令可用）
 *    风扇: fan+ fan- fanrun fanstop
 *    灯光: L+ L- zidong shoudong light?
 *    水泵灌溉: pumpOn pumpOff waterOn waterOff irri+ irri- irri?
 *    蜂鸣器报警: buzzOn buzzOff alarmOn alarmOff
 *    卷帘: curtain+ curtain- curtain0
 *    门禁: auth+密码 gateOn gateOff
 *    传感器: temp? humi? dht? dist? level?
 *    水位报警: wAlarmOn wAlarmOff wAlarm? water+ water-
 *    系统: status?
 *
 * 3. 流控
 *    串口屏收到数据后暂停 120ms 再继续发送，避免串口屏处理不过来。
 *    数据帧以 \\xFF\\xFF\\xFF 结尾。
 */

/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */
#define USART2_TX_PAUSE_AFTER_RX_MS  120U /* 串口屏接收后暂停发送时间 */
#define AUTO_LIGHT_CONTROL_PERIOD_MS 100U /* 自动补光控制刷新周期 */
#define OLED_REFRESH_PERIOD_MS        50U /* OLED 刷新周期 */
#define SCREEN_REFRESH_PERIOD_MS     120U /* 串口屏刷新周期 */
#define ESP32_PUSH_MS               1000U /* ESP32-S3 传感器数据推送周期 */

/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/
/* USER CODE BEGIN Variables */
/* DisplayData_t is defined in usart.h */

volatile DisplayData_t g_display_data = {0};
static volatile uint8_t g_menu_active = 0U;

static uint8_t Wave_MapToLayer(uint16_t value, uint16_t value_max, uint8_t layer_base, uint8_t layer_span)
{
  uint32_t scaled;

  if (value_max == 0U)
  {
    return layer_base;
  }

  if (value > value_max)
  {
    value = value_max;
  }

  scaled = ((uint32_t)value * (uint32_t)layer_span) / (uint32_t)value_max;
  return (uint8_t)(layer_base + (uint8_t)scaled);
}

static uint8_t QuantizeToStep(uint8_t value, uint8_t step, uint8_t max_value)
{
  uint16_t quantized;

  if (step == 0U)
  {
    return value;
  }

  quantized = (uint16_t)(((uint16_t)value + (uint16_t)(step / 2U)) / (uint16_t)step) * (uint16_t)step;
  if (quantized > max_value)
  {
    quantized = max_value;
  }

  return (uint8_t)quantized;
}

static void DisplayTask_SendUart2(const char *format, ...)
{
  char tx_buffer[64];
  va_list args;
  int length;

  va_start(args, format);
  length = vsnprintf(tx_buffer, sizeof(tx_buffer), format, args);
  va_end(args);

  if (length <= 0)
  {
    return;
  }

  if (length > (int)(sizeof(tx_buffer) - 1))
  {
    length = (int)(sizeof(tx_buffer) - 1);
  }

  (void)HAL_UART_Transmit(&huart2, (uint8_t *)tx_buffer, (uint16_t)length, 100U);
}

/* USER CODE END Variables */
/* Definitions for defaultTask */
osThreadId_t defaultTaskHandle;
const osThreadAttr_t defaultTask_attributes = {
  .name = "defaultTask",
  .stack_size = 256 * 4,
  .priority = (osPriority_t) osPriorityNormal,
};
/* Definitions for displayTask */
osThreadId_t displayTaskHandle;
const osThreadAttr_t displayTask_attributes = {
  .name = "displayTask",
  .stack_size = 256 * 4,
  .priority = (osPriority_t) osPriorityBelowNormal,
};
/* Definitions for buzzerTask */
osThreadId_t buzzerTaskHandle;
const osThreadAttr_t buzzerTask_attributes = {
  .name = "buzzerTask",
  .stack_size = 128 * 4,
  .priority = (osPriority_t) osPriorityLow,
};
/* Definitions for esp32Task */
osThreadId_t esp32TaskHandle;
const osThreadAttr_t esp32Task_attributes = {
  .name = "esp32Task",
  .stack_size = 512 * 4,
  .priority = (osPriority_t) osPriorityBelowNormal,
};

/* Private function prototypes -----------------------------------------------*/
/* USER CODE BEGIN FunctionPrototypes */

/* USER CODE END FunctionPrototypes */

void StartDefaultTask(void *argument);
void StartdisplayTask(void *argument);
void StartbuzzerTask(void *argument);
void Startesp32Task(void *argument);

void MX_FREERTOS_Init(void); /* (MISRA C 2004 rule 8.1) */

/**
  * @brief  FreeRTOS initialization
  * @param  None
  * @retval None
  */
void MX_FREERTOS_Init(void) {
  /* USER CODE BEGIN Init */

  /* USER CODE END Init */

  /* USER CODE BEGIN RTOS_MUTEX */
  /* add mutexes, ... */
  /* USER CODE END RTOS_MUTEX */

  /* USER CODE BEGIN RTOS_SEMAPHORES */
  /* add semaphores, ... */
  /* USER CODE END RTOS_SEMAPHORES */

  /* USER CODE BEGIN RTOS_TIMERS */
  /* start timers, add new ones, ... */
  /* USER CODE END RTOS_TIMERS */

  /* USER CODE BEGIN RTOS_QUEUES */
  /* add queues, ... */
  /* USER CODE END RTOS_QUEUES */

  /* Create the thread(s) */
  /* creation of defaultTask */
  defaultTaskHandle = osThreadNew(StartDefaultTask, NULL, &defaultTask_attributes);

  /* creation of displayTask */
  displayTaskHandle = osThreadNew(StartdisplayTask, NULL, &displayTask_attributes);

  /* creation of buzzerTask */
  buzzerTaskHandle = osThreadNew(StartbuzzerTask, NULL, &buzzerTask_attributes);

  /* creation of esp32Task */
  esp32TaskHandle = osThreadNew(Startesp32Task, NULL, &esp32Task_attributes);

  /* USER CODE BEGIN RTOS_THREADS */
  /* add threads, ... */
  /* USER CODE END RTOS_THREADS */

  /* USER CODE BEGIN RTOS_EVENTS */
  /* add events, ... */
  /* USER CODE END RTOS_EVENTS */

}

/* USER CODE BEGIN Header_StartDefaultTask */
/**
  * @brief  Function implementing the defaultTask thread.
  * @param  argument: Not used
  * @retval None
  */
/* USER CODE END Header_StartDefaultTask */
void StartDefaultTask(void *argument)
{
  /* USER CODE BEGIN StartDefaultTask */
  DHT11_Data_t dht11_data;
  uint8_t dht11_humidity = 0U;
  uint32_t dht11_last_tick = 0U;
  uint32_t auto_light_last_tick = 0U;
  uint32_t usart2_report_tick = 0U;
  uint32_t oled_refresh_tick = 0U;
  uint32_t bluetooth_push_tick = 0U;
  uint16_t light_voltage_x100 = 0U;
  uint16_t temperature_x10 = 0U;
  uint16_t distance_x10_cm = 0U;
  uint16_t water_level_x10_cm = 0U;
  uint16_t flow_ml_per_min = 0U;
  uint16_t motor_rpm = 0U;
  uint16_t fan_target_rpm = 0U;
  uint16_t fan_elapsed_s = 0U;
  uint8_t fan_segment = 0U;
  uint8_t light_duty = 0U;
  const char *pump_screen_state;
  const char *irrigation_screen_state;
  const char *buzzer_screen_state;
  const char *alarm_screen_state;
  const char *gate_screen_state;
  const char *curtain_screen_state;
  uint8_t s0_ch1_value;
  uint8_t s0_ch2_value;
  uint8_t s0_ch3_value;
  uint8_t s0_ch4_value;
  uint8_t s1_ch1_value;
  uint8_t s1_ch2_value;
  uint8_t s1_ch3_value;
  uint8_t s1_ch4_value;
  uint8_t s2_ch1_value;
  uint8_t s2_ch2_value;
  uint8_t s2_ch3_value;
  uint8_t light_duty_quantized;
  uint8_t screen_tx_phase = 0U;
  uint32_t now_tick;
  float light_voltage_display;
  float temperature_display;

  AlarmLed_Init();
  AutoLight_Init();
  Buzzer_Init();
  Curtain_Init();
  DHT11_Init();
  USART_SetPrintfUart(&huart2);
  FanProgram_Init();
  FlowSensor_Init();
  Irrigation_Init();
  LightSensor_Init();
  TemperatureSensor_Init();
  Ultrasonic_Init();
  Bluetooth_Init();
  WaterLevelAlarm_Init();
  OutputFault_Init();
  WaterControl_Init();
  GateAccess_Init();
  Button_Init();
  Pump_Init();
  LightPwm_Init();
  LightPwm_SetDutyPercent(0);
  MotorPwm_Init();
  MotorPwm_SetDutyPercent(50);
  Servo_Init();
  MotorSpeed_Init();
  /* Infinite loop */
  for(;;)
  {
    /* 处理从 ISR 排队过来的串口命令包（非阻塞） */
    USART_PollPackets();

    Pump_Process();
    FlowSensor_Update();
    MotorSpeed_Update();
    OutputFault_Process(HAL_GetTick());
    WaterControl_Process(HAL_GetTick());
    Button_Process();
    motor_rpm = (uint16_t)(MotorSpeed_GetRpm() + 0.5f);
    FanProgram_Process(HAL_GetTick(), motor_rpm);
    if ((HAL_GetTick() - dht11_last_tick) >= 1200U)
    {
      dht11_last_tick = HAL_GetTick();
      if (DHT11_Read(&dht11_data) != 0U)
      {
        dht11_humidity = dht11_data.humidity;
        g_display_data.dht11_temperature = dht11_data.temperature;
      }
    }
    Ultrasonic_Process();
    if (Ultrasonic_HasValidData() != 0U)
    {
      distance_x10_cm = Ultrasonic_GetLastDistanceX10Cm();
      water_level_x10_cm = Ultrasonic_ConvertDistanceToWaterLevelX10Cm(distance_x10_cm);
    }

    if ((HAL_GetTick() - auto_light_last_tick) >= AUTO_LIGHT_CONTROL_PERIOD_MS)
    {
      auto_light_last_tick = HAL_GetTick();
      (void)AutoLight_Process();
    }

    if (FanProgram_IsActive() == 0U)
    {
      WaterLevelAlarm_Process(water_level_x10_cm);
    }

    light_voltage_display = LightSensor_ReadVoltageAverage(LIGHT_SENSOR_SAMPLE_COUNT);
    temperature_display = TemperatureSensor_ReadCelsiusAverage(TEMPERATURE_SENSOR_SAMPLE_COUNT);
    light_voltage_x100 = (light_voltage_display <= 0.0f) ? 0U : (uint16_t)(light_voltage_display * 100.0f + 0.5f);
    temperature_x10 = (temperature_display <= 0.0f) ? 0U : (uint16_t)(temperature_display * 10.0f + 0.5f);

    /* 缓存原始值，供 USART 命令查询使用（避免重复 ADC 轮询） */
    {
      uint16_t cached_light_raw = LightSensor_ReadRaw();
      uint8_t cached_light_percent;
      uint32_t percent_tmp = ((uint32_t)cached_light_raw * 100U) / LIGHT_SENSOR_ADC_MAX_VALUE;
      if (percent_tmp > 100U) { percent_tmp = 100U; }
      cached_light_percent = (uint8_t)percent_tmp;
      g_display_data.light_raw = cached_light_raw;
      g_display_data.light_percent = cached_light_percent;
    }
    flow_ml_per_min = FlowSensor_GetFlowMlPerMin();
    motor_rpm = (uint16_t)(MotorSpeed_GetRpm() + 0.5f);
    fan_target_rpm = FanProgram_GetTargetRpm();
    fan_elapsed_s = (uint16_t)(FanProgram_GetElapsedMs() / 1000U);
    fan_segment = FanProgram_GetSegmentIndex();
    light_duty = LightPwm_GetDutyPercent();

    g_display_data.motor_rpm = motor_rpm;
    g_display_data.light_duty = light_duty;
    g_display_data.light_voltage_x100 = light_voltage_x100;
    g_display_data.temperature_x10 = temperature_x10;
    g_display_data.dht11_humidity = dht11_humidity;
    g_display_data.distance_x10_cm = distance_x10_cm;
    g_display_data.water_level_x10_cm = water_level_x10_cm;
    g_display_data.flow_ml_per_min = flow_ml_per_min;
    g_display_data.pump_on = (uint8_t)((Pump_IsOn() != 0U) ? 1U : 0U);
    g_display_data.buzzer_on = (uint8_t)((Buzzer_IsOn() != 0U) ? 1U : 0U);
    g_display_data.alarm_on = (uint8_t)((AlarmLed_IsOn() != 0U) ? 1U : 0U);
    g_display_data.gate_open = (uint8_t)((Servo_GetAngle() >= SERVO_GATE_OPEN_ANGLE) ? 1U : 0U);

    now_tick = HAL_GetTick();

    if ((now_tick - oled_refresh_tick) >= OLED_REFRESH_PERIOD_MS)
    {
      oled_refresh_tick = now_tick;
      if (g_menu_active == 0U)
      {
        OLED_AreaClear(0, 0, OLED_WIDTH, OLED_HEIGHT);
      OLED_Printf5x8(0, 0,  "Fan:%u%%", MotorPwm_GetDutyPercent());
      OLED_Printf5x8(64, 0, "Light:%u%%", g_display_data.light_duty);
      OLED_Printf5x8(0, 8,  "RPM:%u", g_display_data.motor_rpm);
      OLED_Printf5x8(64, 8, "P:%lu", (unsigned long)MotorSpeed_GetPulseCount());
      OLED_Printf5x8(0, 16, "T:%u", fan_target_rpm);
      OLED_Printf5x8(34, 16, "S%u", fan_segment);
      OLED_Printf5x8(48, 16, "%us", fan_elapsed_s);
      OLED_Printf5x8(88, 16, "%s", FanProgram_GetStateText());
      OLED_Printf5x8(0, 24, "Hum:%u%%", g_display_data.dht11_humidity);
      OLED_Printf5x8(64, 24, "Temp:%u.%uC",
                     g_display_data.temperature_x10 / 10U,
                     g_display_data.temperature_x10 % 10U);
      OLED_Printf5x8(0, 32, "Dist:%u.%u",
                     g_display_data.distance_x10_cm / 10U,
                     g_display_data.distance_x10_cm % 10U);
      OLED_Printf5x8(64, 32, "U:%u E:%lu", Ultrasonic_GetDebugState(),
                     (unsigned long)Ultrasonic_GetLastEchoWidthUs());
      OLED_Printf5x8(0, 40, "Water:%u.%u",
                     g_display_data.water_level_x10_cm / 10U,
                     g_display_data.water_level_x10_cm % 10U);
      OLED_Printf5x8(76, 40, "cm");
        OLED_Printf5x8(0, 48, "Flow:%u/m", g_display_data.flow_ml_per_min);
        OLED_Printf5x8(64, 48, "Tot:%lumL", (unsigned long)FlowSensor_GetTotalMl());
        OLED_Update();
      }
    }

    /* 蓝牙轮询-响应模式：仅在手机先发过数据后才开始周期推送 */
    if ((g_usart_polling_enabled != 0U) &&
        (Bluetooth_IsConnected() != 0U) &&
        ((now_tick - bluetooth_push_tick) >= BLUETOOTH_STATUS_PUSH_MS))
    {
      bluetooth_push_tick = now_tick;
      {
        char bt_buf[256];
        uint16_t bt_distance_x10 = (Ultrasonic_HasValidData() != 0U) ? Ultrasonic_GetLastDistanceX10Cm() : 0U;
        uint16_t bt_water_level_x10 = Ultrasonic_ConvertDistanceToWaterLevelX10Cm(bt_distance_x10);
        uint16_t bt_temp_x10 = g_display_data.temperature_x10;
        uint8_t bt_humidity = g_display_data.dht11_humidity;
        uint16_t bt_light_x100 = g_display_data.light_voltage_x100;
        uint16_t bt_rpm = g_display_data.motor_rpm;
        uint16_t bt_flow = g_display_data.flow_ml_per_min;
        uint8_t bt_pump = g_display_data.pump_on;
        uint8_t bt_buzzer = g_display_data.buzzer_on;
        uint8_t bt_alarm = g_display_data.alarm_on;
        uint8_t bt_gate = g_display_data.gate_open;
        uint8_t bt_curtain = (uint8_t)Curtain_GetState();
        uint8_t bt_fan_active = FanProgram_IsActive();
        uint16_t bt_fan_target = FanProgram_GetTargetRpm();
        uint8_t bt_fan_seg = FanProgram_GetSegmentIndex();
        uint16_t bt_fan_elapsed = (uint16_t)(FanProgram_GetElapsedMs() / 1000U);
        uint8_t bt_fan_duty = MotorPwm_GetDutyPercent();
        uint8_t bt_light_duty = g_display_data.light_duty;
        uint8_t bt_light_auto = AutoLight_IsAutoMode();
        uint8_t bt_irri_level = Irrigation_GetLevel();
        uint8_t bt_irri_duty = Irrigation_GetDutyPercent();
        uint8_t bt_walarm_on = WaterLevelAlarm_IsEnabled();
        uint16_t bt_walarm_thr = WaterLevelAlarm_GetThresholdX10Cm();
        uint8_t bt_wctrl_auto = WaterControl_IsAutoMode();
        uint16_t bt_wctrl_target = WaterControl_GetTargetX10Cm();
        uint8_t bt_fault = OutputFault_HasAnyFault();
        const char *curtain_str;
        const char *fault_str;

        switch (bt_curtain)
        {
          case CURTAIN_STATE_FORWARD:  curtain_str = "FWD";  break;
          case CURTAIN_STATE_REVERSE:  curtain_str = "REV";  break;
          default:                     curtain_str = "STOP"; break;
        }

        fault_str = (bt_fault != 0U) ? "FAULT" : "OK";

        /* ---- Line 1: センサデータ ---- */
        (void)snprintf(bt_buf, sizeof(bt_buf),
                       "T:%u.%uC H:%u%% WL:%u.%ucm D:%u.%ucm L:%u.%02uV FL:%uml/m\r\n",
                       bt_temp_x10 / 10U, bt_temp_x10 % 10U,
                       bt_humidity,
                       bt_water_level_x10 / 10U, bt_water_level_x10 % 10U,
                       bt_distance_x10 / 10U, bt_distance_x10 % 10U,
                       bt_light_x100 / 100U, bt_light_x100 % 100U,
                       bt_flow);
        Bluetooth_SendString(bt_buf);

        /* ---- Line 2: アクチュエータ状態 ---- */
        (void)snprintf(bt_buf, sizeof(bt_buf),
                       "FAN:%uRPM/%u%% PGM:%s S%u T:%uRPM E:%us "
                       "PUMP:%s BUZZ:%s ALM:%s GATE:%s CUR:%s "
                       "IRR:%s/%u%%\r\n",
                       bt_rpm,
                       bt_fan_duty,
                       (bt_fan_active != 0U) ? "ON" : "OFF",
                       bt_fan_seg,
                       bt_fan_target,
                       bt_fan_elapsed,
                       (bt_pump != 0U) ? "ON" : "OFF",
                       (bt_buzzer != 0U) ? "ON" : "OFF",
                       (bt_alarm != 0U) ? "ON" : "OFF",
                       (bt_gate != 0U) ? "OPEN" : "CLOSED",
                       curtain_str,
                       Irrigation_GetLevelName(bt_irri_level),
                       bt_irri_duty);
        Bluetooth_SendString(bt_buf);

        /* ---- Line 3: モード + 故障 ---- */
        (void)snprintf(bt_buf, sizeof(bt_buf),
                       "LM:%s LD:%u%% WA:%s/%u.%ucm WC:%s/%u.%ucm [%s]\r\n",
                       (bt_light_auto != 0U) ? "AUTO" : "MANUAL",
                       bt_light_duty,
                       (bt_walarm_on != 0U) ? "ON" : "OFF",
                       bt_walarm_thr / 10U, bt_walarm_thr % 10U,
                       (bt_wctrl_auto != 0U) ? "AUTO" : "MANUAL",
                       bt_wctrl_target / 10U, bt_wctrl_target % 10U,
                       fault_str);
        Bluetooth_SendString(bt_buf);
      }
    }

    if ((now_tick - usart2_report_tick) >= SCREEN_REFRESH_PERIOD_MS)
    {
      usart2_report_tick = now_tick;
      if ((now_tick - USART2_GetLastRxTick()) >= USART2_TX_PAUSE_AFTER_RX_MS)
      {
        pump_screen_state = (Pump_IsOn() != 0U) ? "OPEN" : "CLOSE";
        irrigation_screen_state = (Irrigation_IsOn() != 0U) ? "OPEN" : "CLOSE";
        buzzer_screen_state = (Buzzer_IsOn() != 0U) ? "OPEN" : "CLOSE";
        alarm_screen_state = (AlarmLed_IsOn() != 0U) ? "OPEN" : "CLOSE";
        gate_screen_state = (g_display_data.gate_open != 0U) ? "OPEN" : "CLOSE";
        switch (Curtain_GetState())
        {
          case CURTAIN_STATE_FORWARD:
            curtain_screen_state = "OPEN";
            break;

          case CURTAIN_STATE_REVERSE:
            curtain_screen_state = "CLOSE";
            break;

          default:
            curtain_screen_state = "STOP";
            break;
        }
        light_duty_quantized = QuantizeToStep(g_display_data.light_duty, 10U, 100U);
        s0_ch1_value = Wave_MapToLayer(g_display_data.motor_rpm, 1000U, 150U, 45U);
        s0_ch2_value = Wave_MapToLayer(light_duty_quantized,  100U, 100U, 45U);
        s0_ch3_value = Wave_MapToLayer(g_display_data.pump_on,   1U,  50U, 45U);
        s0_ch4_value = Wave_MapToLayer(g_display_data.buzzer_on, 1U,   0U, 45U);

        s1_ch1_value = Wave_MapToLayer(g_display_data.alarm_on,              1U,  150U, 45U);
        s1_ch2_value = Wave_MapToLayer(g_display_data.gate_open,             1U,  100U, 45U);
        s1_ch3_value = Wave_MapToLayer(g_display_data.light_voltage_x100,  330U,   50U, 45U);
        s1_ch4_value = Wave_MapToLayer(g_display_data.temperature_x10,     500U,    0U, 45U);

        s2_ch1_value = Wave_MapToLayer(g_display_data.dht11_humidity,       100U,  133U, 60U);
        s2_ch2_value = Wave_MapToLayer(g_display_data.water_level_x10_cm,   200U,   66U, 60U);
        s2_ch3_value = Wave_MapToLayer(g_display_data.flow_ml_per_min,     2000U,    0U, 60U);

        switch (screen_tx_phase)
        {
          case 0U:
            DisplayTask_SendUart2("add s0.id,0,%d\xFF\xFF\xFF", s0_ch1_value);
            DisplayTask_SendUart2("add s0.id,1,%d\xFF\xFF\xFF", s0_ch2_value);
            DisplayTask_SendUart2("add s0.id,2,%d\xFF\xFF\xFF", s0_ch3_value);
            DisplayTask_SendUart2("add s0.id,3,%d\xFF\xFF\xFF", s0_ch4_value);
            break;

          case 1U:
            DisplayTask_SendUart2("add s1.id,0,%d\xFF\xFF\xFF", s1_ch1_value);
            DisplayTask_SendUart2("add s1.id,1,%d\xFF\xFF\xFF", s1_ch2_value);
            DisplayTask_SendUart2("add s1.id,2,%d\xFF\xFF\xFF", s1_ch3_value);
            break;

          case 2U:
            DisplayTask_SendUart2("add s1.id,3,%d\xFF\xFF\xFF", s1_ch4_value);
            DisplayTask_SendUart2("add s2.id,0,%d\xFF\xFF\xFF", s2_ch1_value);
            DisplayTask_SendUart2("add s2.id,1,%d\xFF\xFF\xFF", s2_ch2_value);
            DisplayTask_SendUart2("add s2.id,2,%d\xFF\xFF\xFF", s2_ch3_value);
            DisplayTask_SendUart2("page0.t11.txt=\"%uRPM\"\xFF\xFF\xFF", g_display_data.motor_rpm);
            DisplayTask_SendUart2("page0.t12.txt=\"%u%%\"\xFF\xFF\xFF", g_display_data.light_duty);
            break;

          case 3U:
            DisplayTask_SendUart2("page0.t13.txt=\"%s\"\xFF\xFF\xFF", irrigation_screen_state);
            DisplayTask_SendUart2("page0.t14.txt=\"%s\"\xFF\xFF\xFF", pump_screen_state);
            DisplayTask_SendUart2("page0.t15.txt=\"%s\"\xFF\xFF\xFF", alarm_screen_state);
            DisplayTask_SendUart2("page0.t16.txt=\"%s\"\xFF\xFF\xFF", gate_screen_state);
            DisplayTask_SendUart2("page0.t33.txt=\"%s\"\xFF\xFF\xFF", curtain_screen_state);
            DisplayTask_SendUart2("page0.t34.txt=\"%s\"\xFF\xFF\xFF", buzzer_screen_state);
            break;

          default:
            DisplayTask_SendUart2("page0.t20.txt=\"%u.%02uV\"\xFF\xFF\xFF",
                                  g_display_data.light_voltage_x100 / 100U,
                                  g_display_data.light_voltage_x100 % 100U);
            DisplayTask_SendUart2("page0.t9.txt=\"%u.%uC\"\xFF\xFF\xFF",
                                  g_display_data.temperature_x10 / 10U,
                                  g_display_data.temperature_x10 % 10U);
            DisplayTask_SendUart2("page0.t10.txt=\"%u%%\"\xFF\xFF\xFF", g_display_data.dht11_humidity);
            DisplayTask_SendUart2("page0.t21.txt=\"%u.%ucm\"\xFF\xFF\xFF",
                                  g_display_data.water_level_x10_cm / 10U,
                                  g_display_data.water_level_x10_cm % 10U);
            DisplayTask_SendUart2("page0.t36.txt=\"%u.%ucm\"\xFF\xFF\xFF",
                                  WaterLevelAlarm_GetThresholdX10Cm() / 10U,
                                  WaterLevelAlarm_GetThresholdX10Cm() % 10U);
            DisplayTask_SendUart2("page2.t37.txt=\"T:%us,RPM:%u\"\xFF\xFF\xFF",
                                  fan_elapsed_s,
                                  g_display_data.motor_rpm);
            DisplayTask_SendUart2("page0.t39.txt=\"%lumL\"\xFF\xFF\xFF",
                                  (unsigned long)FlowSensor_GetTotalMl());
            DisplayTask_SendUart2("page0.t40.txt=\"%u.%ucm\"\xFF\xFF\xFF",
                                  WaterControl_GetTargetX10Cm() / 10U,
                                  WaterControl_GetTargetX10Cm() % 10U);
            break;
        }

        screen_tx_phase++;
        if (screen_tx_phase >= 5U)
        {
          screen_tx_phase = 0U;
        }
      }
    }

    osDelay(5);
  }
  /* USER CODE END StartDefaultTask */
}

/* USER CODE BEGIN Header_StartdisplayTask */
/**
* @brief Function implementing the displayTask thread.
* @param argument: Not used
* @retval None
*/
/* USER CODE END Header_StartdisplayTask */
void StartdisplayTask(void *argument)
{
  /* USER CODE BEGIN StartdisplayTask */
  uint8_t menu_state = 0U;
  uint8_t cursor_pos = 0U;
  uint8_t menu_redraw = 1U;
  uint32_t last_button_tick = 0U;
  uint32_t now_tick;
  uint8_t any_event;
  uint8_t btn_id;
  uint8_t btn_event;

  for(;;)
  {
    now_tick = HAL_GetTick();

    if ((now_tick - last_button_tick) >= 50U)
    {
      last_button_tick = now_tick;
      Button_Process();
    }

    any_event = Button_GetAnyEvent();
    btn_id = (any_event >> 4) & 0x0FU;
    btn_event = any_event & 0x0FU;

    if (btn_event == BUTTON_EVENT_SHORT_PRESS || btn_event == BUTTON_EVENT_LONG_PRESS)
    {
      switch (menu_state)
      {
        case 0U:
          if (btn_id == BUTTON_KEY1 && btn_event == BUTTON_EVENT_SHORT_PRESS)
          {
            menu_state = 1U;
            cursor_pos = 0U;
            menu_redraw = 1U;
            g_menu_active = 1U;
          }
          break;

        case 1U:
          if (btn_id == BUTTON_KEY2)
          {
            if (cursor_pos > 0U) { cursor_pos--; menu_redraw = 1U; }
          }
          else if (btn_id == BUTTON_KEY3)
          {
            if (cursor_pos < 5U) { cursor_pos++; menu_redraw = 1U; }
          }
          else if (btn_id == BUTTON_KEY1)
          {
            menu_state = cursor_pos + 2U;
            menu_redraw = 1U;
          }
          else if (btn_id == BUTTON_KEY4)
          {
            menu_state = 0U;
            g_menu_active = 0U;
            menu_redraw = 0U;
            OLED_AreaClear(0, 0, OLED_WIDTH, OLED_HEIGHT);
          }
          break;

        case 2U:
        case 3U:
        case 4U:
        case 5U:
        case 6U:
        case 7U:
          if (btn_id == BUTTON_KEY4)
          {
            menu_state = 1U;
            cursor_pos = 0U;
            menu_redraw = 1U;
          }
          if (menu_state == 2U)
          {
            if (btn_id == BUTTON_KEY2)
            {
              uint8_t lvl = Irrigation_GetLevel();
              if (lvl < IRRIGATION_LEVEL_MAX) { Irrigation_SetLevel(lvl + 1U); menu_redraw = 1U; }
            }
            else if (btn_id == BUTTON_KEY3)
            {
              uint8_t lvl = Irrigation_GetLevel();
              if (lvl > 0U) { Irrigation_SetLevel(lvl - 1U); menu_redraw = 1U; }
            }
          }
          break;

        default:
          break;
      }
    }

    if (menu_redraw != 0U && menu_state != 0U)
    {
      menu_redraw = 0U;
      OLED_AreaClear(0, 0, OLED_WIDTH, OLED_HEIGHT);

      switch (menu_state)
      {
        case 1U:
        {
          const char *items[] = {"1.Irrigation", "2.Water Ctrl", "3.Light Ctrl",
                                 "4.Gate", "5.Fault", "6.System"};
          uint8_t i;
          for (i = 0U; i < 6U; i++)
          {
            OLED_Printf5x8(8, (int16_t)(i * 10), "%c%s",
                           (i == cursor_pos) ? '>' : ' ', items[i]);
          }
          break;
        }

        case 2U:
          OLED_Printf5x8(0, 0, "Irrigation:");
          OLED_Printf5x8(0, 10, "Level: %s", Irrigation_GetLevelName(Irrigation_GetLevel()));
          OLED_Printf5x8(0, 20, "Duty: %u%%", Irrigation_GetDutyPercent());
          OLED_Printf5x8(0, 30, "State: %s", Irrigation_IsOn() != 0U ? "ON" : "OFF");
          OLED_Printf5x8(0, 42, "KEY2/3:Adj KEY4:Back");
          break;

        case 3U:
          OLED_Printf5x8(0, 0, "Water Control:");
          OLED_Printf5x8(0, 10, "Target: %u.%ucm",
                         WaterControl_GetTargetX10Cm() / 10U,
                         WaterControl_GetTargetX10Cm() % 10U);
          OLED_Printf5x8(0, 20, "Current: %u.%ucm",
                         g_display_data.water_level_x10_cm / 10U,
                         g_display_data.water_level_x10_cm % 10U);
          OLED_Printf5x8(0, 30, "Mode: %s", WaterControl_IsAutoMode() != 0U ? "AUTO" : "MANUAL");
          OLED_Printf5x8(0, 42, "KEY2/3:Adj KEY4:Back");
          break;

        case 4U:
          OLED_Printf5x8(0, 0, "Light Control:");
          OLED_Printf5x8(0, 10, "Duty: %u%%", g_display_data.light_duty);
          OLED_Printf5x8(0, 20, "Voltage: %u.%uV",
                         g_display_data.light_voltage_x100 / 100U,
                         g_display_data.light_voltage_x100 % 100U);
          OLED_Printf5x8(0, 30, "Mode: %s", AutoLight_IsAutoMode() != 0U ? "AUTO" : "MANUAL");
          OLED_Printf5x8(0, 42, "KEY4:Back");
          break;

        case 5U:
          OLED_Printf5x8(0, 0, "Gate Status:");
          OLED_Printf5x8(0, 10, "Gate: %s", g_display_data.gate_open != 0U ? "OPEN" : "CLOSED");
          OLED_Printf5x8(0, 20, "Angle: %u deg", Servo_GetAngle());
          OLED_Printf5x8(0, 42, "KEY4:Back");
          break;

        case 6U:
        {
          OLED_Printf5x8(0, 0, "Fault Info:");
          char fault_buf[64];
          OutputFault_GetFlagsString(fault_buf, sizeof(fault_buf));
          OLED_Printf5x8(0, 10, "%s", fault_buf);
          OLED_Printf5x8(0, 42, "KEY4:Back");
          break;
        }

        case 7U:
          OLED_Printf5x8(0, 0, "System Info:");
          OLED_Printf5x8(0, 10, "Temp: %u.%uC",
                         g_display_data.temperature_x10 / 10U,
                         g_display_data.temperature_x10 % 10U);
          OLED_Printf5x8(0, 20, "Humidity: %u%%", g_display_data.dht11_humidity);
          OLED_Printf5x8(0, 30, "Water: %u.%ucm",
                         g_display_data.water_level_x10_cm / 10U,
                         g_display_data.water_level_x10_cm % 10U);
          OLED_Printf5x8(0, 42, "KEY4:Back");
          break;

        default:
          break;
      }

      OLED_Update();
    }

    osDelay(30);
  }
  /* USER CODE END StartdisplayTask */
}

/* USER CODE BEGIN Header_StartbuzzerTask */
/**
* @brief Function implementing the buzzerTask thread.
* @param argument: Not used
* @retval None
*/
/* USER CODE END Header_StartbuzzerTask */
void StartbuzzerTask(void *argument)
{
  /* USER CODE BEGIN StartbuzzerTask */
  /* Infinite loop */
  for(;;)
  {
    Buzzer_Process();
    osDelay(1);
  }
  /* USER CODE END StartbuzzerTask */
}

/* Private application code --------------------------------------------------*/
/* USER CODE BEGIN Application */

void Startesp32Task(void *argument)
{
  (void)argument;
  for (;;)
  {
    {
      /* 二进制帧：AA 55 <len=16> <16字节payload>，与 ESP32 端 Stm32Data_t 一致 */
      typedef struct __attribute__((packed)) {
        uint16_t motor_rpm;
        uint8_t  light_duty;
        uint16_t light_voltage_x100;
        uint16_t temperature_x10;
        uint8_t  dht11_humidity;
        uint16_t distance_x10_cm;
        uint16_t water_level_x10_cm;
        uint16_t flow_ml_per_min;
        uint8_t  pump_on;
        uint8_t  buzzer_on;
        uint8_t  alarm_on;
        uint8_t  gate_open;
      } Esp32Frame_t;

      Esp32Frame_t frame;
      uint8_t pkt[3 + sizeof(Esp32Frame_t)];  /* AA 55 len payload */

      /* 编译时校验：struct 必须 18 字节（含 2 字节对齐填充） */
      _Static_assert(sizeof(Esp32Frame_t) == 18U, "Esp32Frame_t must be 18 bytes");

      frame.motor_rpm          = (uint16_t)(MotorSpeed_GetRpm() + 0.5f);
      frame.light_duty         = LightPwm_GetDutyPercent();
      frame.light_voltage_x100 = g_display_data.light_voltage_x100;
      frame.temperature_x10    = g_display_data.temperature_x10;
      frame.dht11_humidity     = g_display_data.dht11_humidity;
      frame.distance_x10_cm    = (Ultrasonic_HasValidData() != 0U)
                                 ? Ultrasonic_GetLastDistanceX10Cm() : 0U;
      frame.water_level_x10_cm = Ultrasonic_ConvertDistanceToWaterLevelX10Cm(frame.distance_x10_cm);
      frame.flow_ml_per_min    = FlowSensor_GetFlowMlPerMin();
      frame.pump_on            = Pump_IsOn();
      frame.buzzer_on          = Buzzer_IsOn();
      frame.alarm_on           = AlarmLed_IsOn();
      frame.gate_open          = g_display_data.gate_open;

      pkt[0] = 0xAAU;
      pkt[1] = 0x55U;
      pkt[2] = (uint8_t)sizeof(Esp32Frame_t);
      memcpy(&pkt[3], &frame, sizeof(Esp32Frame_t));

      (void)HAL_UART_Transmit(&huart3, pkt, sizeof(pkt), 100U);
    }
    osDelay(ESP32_PUSH_MS);
  }
}

/* USER CODE END Application */

