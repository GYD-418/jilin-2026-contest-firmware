#include "alarm_led.h"

static uint8_t g_alarm_led_is_on = 0U;
static uint8_t g_alarm_led_manual_on = 0U;
static uint8_t g_alarm_led_auto_on = 0U;

static void AlarmLed_UpdateState(void)
{
    g_alarm_led_is_on = (uint8_t)((g_alarm_led_manual_on != 0U) || (g_alarm_led_auto_on != 0U));
    HAL_GPIO_WritePin(baojingdeng_GPIO_Port,
                      baojingdeng_Pin,
                      (g_alarm_led_is_on != 0U) ? GPIO_PIN_SET : GPIO_PIN_RESET);
}

void AlarmLed_Init(void)
{
    g_alarm_led_manual_on = 0U;
    g_alarm_led_auto_on = 0U;
    AlarmLed_Off();
}

void AlarmLed_On(void)
{
    g_alarm_led_manual_on = 1U;
    AlarmLed_UpdateState();
}

void AlarmLed_Off(void)
{
    g_alarm_led_manual_on = 0U;
    g_alarm_led_auto_on = 0U;
    AlarmLed_UpdateState();
}

void AlarmLed_AutoOn(void)
{
    g_alarm_led_auto_on = 1U;
    AlarmLed_UpdateState();
}

void AlarmLed_AutoOff(void)
{
    g_alarm_led_auto_on = 0U;
    AlarmLed_UpdateState();
}

void AlarmLed_Toggle(void)
{
    if (g_alarm_led_is_on != 0U)
    {
        AlarmLed_Off();
    }
    else
    {
        AlarmLed_On();
    }
}

uint8_t AlarmLed_IsOn(void)
{
    return g_alarm_led_is_on;
}
