#include "water_level_alarm.h"

#include "alarm_led.h"
#include "buzzer.h"

static uint8_t g_water_level_alarm_enabled = 1U;
static uint8_t g_water_level_alarm_active = 0U;
static uint16_t g_water_level_alarm_threshold_x10_cm = WATER_LEVEL_ALARM_DEFAULT_THRESHOLD_X10_CM;

void WaterLevelAlarm_Init(void)
{
    g_water_level_alarm_enabled = 1U;
    g_water_level_alarm_active = 0U;
    g_water_level_alarm_threshold_x10_cm = WATER_LEVEL_ALARM_DEFAULT_THRESHOLD_X10_CM;
}

void WaterLevelAlarm_Enable(void)
{
    g_water_level_alarm_enabled = 1U;
}

void WaterLevelAlarm_Disable(void)
{
    g_water_level_alarm_enabled = 0U;
    g_water_level_alarm_active = 0U;
}

void WaterLevelAlarm_SetThresholdX10Cm(uint16_t threshold_x10_cm)
{
    g_water_level_alarm_threshold_x10_cm = threshold_x10_cm;
}

uint16_t WaterLevelAlarm_GetThresholdX10Cm(void)
{
    return g_water_level_alarm_threshold_x10_cm;
}

uint8_t WaterLevelAlarm_IsEnabled(void)
{
    return g_water_level_alarm_enabled;
}

uint8_t WaterLevelAlarm_IsActive(void)
{
    return g_water_level_alarm_active;
}

void WaterLevelAlarm_Process(uint16_t water_level_x10_cm)
{
    if (g_water_level_alarm_enabled == 0U)
    {
        g_water_level_alarm_active = 0U;
        return;
    }

    if (water_level_x10_cm >= g_water_level_alarm_threshold_x10_cm)
    {
        g_water_level_alarm_active = 1U;
        AlarmLed_AutoOn();
        Buzzer_AutoOn();
    }
    else
    {
        g_water_level_alarm_active = 0U;
        AlarmLed_AutoOff();
        Buzzer_AutoOff();
    }
}
