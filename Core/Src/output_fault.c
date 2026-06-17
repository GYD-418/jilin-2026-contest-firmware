#include "output_fault.h"

#include "motor_pwm.h"
#include "motor_speed.h"
#include "pump.h"
#include "flow_sensor.h"
#include "irrigation.h"
#include "curtain.h"
#include "buzzer.h"
#include "alarm_led.h"
#include "light_pwm.h"

#define OUTPUT_FAULT_FAN_THRESHOLD_MS     3000U
#define OUTPUT_FAULT_PUMP_THRESHOLD_MS    5000U
#define OUTPUT_FAULT_FAN_RPM_MIN          50.0f
#define OUTPUT_FAULT_FLOW_LEAK_THRESHOLD  100U

static uint32_t s_last_check_tick = 0U;
static uint16_t s_fault_flags = 0U;
static uint32_t s_fan_fault_duration_ms = 0U;
static uint32_t s_pump_fault_duration_ms = 0U;

static uint16_t append_str(char *buf, uint16_t buf_size, uint16_t pos,
                           const char *str)
{
  while (*str != '\0' && pos < (buf_size - 1U))
  {
    buf[pos++] = *str++;
  }
  return pos;
}

void OutputFault_Init(void)
{
  s_last_check_tick = 0U;
  s_fault_flags = 0U;
  s_fan_fault_duration_ms = 0U;
  s_pump_fault_duration_ms = 0U;
}

void OutputFault_Process(uint32_t now_tick)
{
  uint32_t elapsed;
  GPIO_PinState pin_fwd;
  GPIO_PinState pin_rev;
  uint8_t any_led_high;

  if ((now_tick - s_last_check_tick) < OUTPUT_FAULT_CHECK_MS)
  {
    return;
  }
  elapsed = now_tick - s_last_check_tick;
  s_last_check_tick = now_tick;

  /* Fan fault: PWM on but RPM too low for sustained period */
  if (MotorPwm_GetDutyPercent() > 0U)
  {
    if (MotorSpeed_GetRpm() < OUTPUT_FAULT_FAN_RPM_MIN)
    {
      s_fan_fault_duration_ms += elapsed;
      if (s_fan_fault_duration_ms >= OUTPUT_FAULT_FAN_THRESHOLD_MS)
      {
        s_fault_flags |= OUTPUT_FAULT_FLAG_FAN;
      }
    }
    else
    {
      s_fan_fault_duration_ms = 0U;
      s_fault_flags &= ~OUTPUT_FAULT_FLAG_FAN;
    }
  }
  else
  {
    s_fan_fault_duration_ms = 0U;
    s_fault_flags &= ~OUTPUT_FAULT_FLAG_FAN;
  }

  /* Pump fault: pump on but no flow for sustained period */
  if (Pump_IsOn() != 0U)
  {
    if (FlowSensor_GetFlowMlPerMin() == 0U)
    {
      s_pump_fault_duration_ms += elapsed;
      if (s_pump_fault_duration_ms >= OUTPUT_FAULT_PUMP_THRESHOLD_MS)
      {
        s_fault_flags |= OUTPUT_FAULT_FLAG_PUMP;
      }
    }
    else
    {
      s_pump_fault_duration_ms = 0U;
      s_fault_flags &= ~OUTPUT_FAULT_FLAG_PUMP;
    }
  }
  else
  {
    s_pump_fault_duration_ms = 0U;
    s_fault_flags &= ~OUTPUT_FAULT_FLAG_PUMP;
  }

  /* Irrigation fault: relay on but GPIO reads low */
  if (Irrigation_IsOn() != 0U)
  {
    if (HAL_GPIO_ReadPin(guangai_GPIO_Port, guangai_Pin) == GPIO_PIN_RESET)
    {
      s_fault_flags |= OUTPUT_FAULT_FLAG_IRRIGATION;
    }
    else
    {
      s_fault_flags &= ~OUTPUT_FAULT_FLAG_IRRIGATION;
    }
  }
  else
  {
    s_fault_flags &= ~OUTPUT_FAULT_FLAG_IRRIGATION;
  }

  /* Curtain fault: motor running but both direction pins stuck high */
  if (Curtain_GetState() != CURTAIN_STATE_STOP)
  {
    pin_fwd = HAL_GPIO_ReadPin(juanlian_zheng_GPIO_Port, juanlian_zheng_Pin);
    pin_rev = HAL_GPIO_ReadPin(juanlian_fan_GPIO_Port, juanlian_fan_Pin);
    if (pin_fwd == GPIO_PIN_SET && pin_rev == GPIO_PIN_SET)
    {
      s_fault_flags |= OUTPUT_FAULT_FLAG_CURTAIN;
    }
    else
    {
      s_fault_flags &= ~OUTPUT_FAULT_FLAG_CURTAIN;
    }
  }
  else
  {
    s_fault_flags &= ~OUTPUT_FAULT_FLAG_CURTAIN;
  }

  /* Buzzer fault: logic says on but GPIO reads low */
  if (Buzzer_IsOn() != 0U)
  {
    if (HAL_GPIO_ReadPin(fengmingqi_GPIO_Port, fengmingqi_Pin) == GPIO_PIN_RESET)
    {
      s_fault_flags |= OUTPUT_FAULT_FLAG_BUZZER;
    }
    else
    {
      s_fault_flags &= ~OUTPUT_FAULT_FLAG_BUZZER;
    }
  }
  else
  {
    s_fault_flags &= ~OUTPUT_FAULT_FLAG_BUZZER;
  }

  /* Alarm LED fault: logic says on but GPIO reads low */
  if (AlarmLed_IsOn() != 0U)
  {
    if (HAL_GPIO_ReadPin(baojingdeng_GPIO_Port, baojingdeng_Pin) == GPIO_PIN_RESET)
    {
      s_fault_flags |= OUTPUT_FAULT_FLAG_ALARM_LED;
    }
    else
    {
      s_fault_flags &= ~OUTPUT_FAULT_FLAG_ALARM_LED;
    }
  }
  else
  {
    s_fault_flags &= ~OUTPUT_FAULT_FLAG_ALARM_LED;
  }

  /* Light fault: PWM on but all LED GPIO pins read low */
  if (LightPwm_GetDutyPercent() > 0U)
  {
    any_led_high = 0U;
    if (HAL_GPIO_ReadPin(TIM2CH2_LED2_GPIO_Port, TIM2CH2_LED2_Pin) == GPIO_PIN_SET)
    {
      any_led_high = 1U;
    }
    if (HAL_GPIO_ReadPin(TIM2CH3_LED3_GPIO_Port, TIM2CH3_LED3_Pin) == GPIO_PIN_SET)
    {
      any_led_high = 1U;
    }
    if (HAL_GPIO_ReadPin(TIM2CH4_LED4_GPIO_Port, TIM2CH4_LED4_Pin) == GPIO_PIN_SET)
    {
      any_led_high = 1U;
    }
    if (HAL_GPIO_ReadPin(TIM3CH2_LED1_GPIO_Port, TIM3CH2_LED1_Pin) == GPIO_PIN_SET)
    {
      any_led_high = 1U;
    }
    if (HAL_GPIO_ReadPin(TIM3CH2_LED5_GPIO_Port, TIM3CH2_LED5_Pin) == GPIO_PIN_SET)
    {
      any_led_high = 1U;
    }
    if (HAL_GPIO_ReadPin(TIM3CH2_LED6_GPIO_Port, TIM3CH2_LED6_Pin) == GPIO_PIN_SET)
    {
      any_led_high = 1U;
    }
    if (HAL_GPIO_ReadPin(TIM4_CH1_LED7_GPIO_Port, TIM4_CH1_LED7_Pin) == GPIO_PIN_SET)
    {
      any_led_high = 1U;
    }
    if (HAL_GPIO_ReadPin(TIM4_CH2_LED8_GPIO_Port, TIM4_CH2_LED8_Pin) == GPIO_PIN_SET)
    {
      any_led_high = 1U;
    }
    if (any_led_high == 0U)
    {
      s_fault_flags |= OUTPUT_FAULT_FLAG_LIGHT;
    }
    else
    {
      s_fault_flags &= ~OUTPUT_FAULT_FLAG_LIGHT;
    }
  }
  else
  {
    s_fault_flags &= ~OUTPUT_FAULT_FLAG_LIGHT;
  }

  /* Flow leak: pump off but flow sensor still reports flow */
  if (Pump_IsOn() == 0U)
  {
    if (FlowSensor_GetFlowMlPerMin() > OUTPUT_FAULT_FLOW_LEAK_THRESHOLD)
    {
      s_fault_flags |= OUTPUT_FAULT_FLAG_FLOW_LEAK;
    }
    else
    {
      s_fault_flags &= ~OUTPUT_FAULT_FLAG_FLOW_LEAK;
    }
  }
  else
  {
    s_fault_flags &= ~OUTPUT_FAULT_FLAG_FLOW_LEAK;
  }
}

uint16_t OutputFault_GetFlags(void)
{
  return s_fault_flags;
}

uint8_t OutputFault_HasAnyFault(void)
{
  return (s_fault_flags != 0U) ? 1U : 0U;
}

const char *OutputFault_GetFlagsString(char *buf, uint16_t buf_size)
{
  uint16_t pos = 0U;
  uint8_t  first = 1U;

  if (buf_size == 0U)
  {
    return buf;
  }
  buf[0] = '\0';

  if ((s_fault_flags & OUTPUT_FAULT_FLAG_FAN) != 0U)
  {
    if (first == 0U) { pos = append_str(buf, buf_size, pos, " "); }
    pos = append_str(buf, buf_size, pos, "FAN");
    first = 0U;
  }
  if ((s_fault_flags & OUTPUT_FAULT_FLAG_PUMP) != 0U)
  {
    if (first == 0U) { pos = append_str(buf, buf_size, pos, " "); }
    pos = append_str(buf, buf_size, pos, "PUMP");
    first = 0U;
  }
  if ((s_fault_flags & OUTPUT_FAULT_FLAG_IRRIGATION) != 0U)
  {
    if (first == 0U) { pos = append_str(buf, buf_size, pos, " "); }
    pos = append_str(buf, buf_size, pos, "IRRIGATION");
    first = 0U;
  }
  if ((s_fault_flags & OUTPUT_FAULT_FLAG_CURTAIN) != 0U)
  {
    if (first == 0U) { pos = append_str(buf, buf_size, pos, " "); }
    pos = append_str(buf, buf_size, pos, "CURTAIN");
    first = 0U;
  }
  if ((s_fault_flags & OUTPUT_FAULT_FLAG_BUZZER) != 0U)
  {
    if (first == 0U) { pos = append_str(buf, buf_size, pos, " "); }
    pos = append_str(buf, buf_size, pos, "BUZZER");
    first = 0U;
  }
  if ((s_fault_flags & OUTPUT_FAULT_FLAG_ALARM_LED) != 0U)
  {
    if (first == 0U) { pos = append_str(buf, buf_size, pos, " "); }
    pos = append_str(buf, buf_size, pos, "ALARM_LED");
    first = 0U;
  }
  if ((s_fault_flags & OUTPUT_FAULT_FLAG_LIGHT) != 0U)
  {
    if (first == 0U) { pos = append_str(buf, buf_size, pos, " "); }
    pos = append_str(buf, buf_size, pos, "LIGHT");
    first = 0U;
  }
  if ((s_fault_flags & OUTPUT_FAULT_FLAG_FLOW_LEAK) != 0U)
  {
    if (first == 0U) { pos = append_str(buf, buf_size, pos, " "); }
    pos = append_str(buf, buf_size, pos, "FLOW_LEAK");
    first = 0U;
  }

  buf[pos] = '\0';
  return buf;
}
