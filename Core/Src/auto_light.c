#include "auto_light.h"

#include "light_pwm.h"
#include "light_sensor.h"

#define AUTO_LIGHT_SENSOR_MAX_VOLTAGE  1.73f /* 光照传感器现场实测最大电压 */
#define AUTO_LIGHT_LEVEL_COUNT         10U   /* 自动调光分级数量 */

static AutoLightMode_t g_auto_light_mode = AUTO_LIGHT_MODE_MANUAL;
static uint8_t g_auto_light_inited = 0U;

void AutoLight_Init(void)
{
    if (g_auto_light_inited == 0U)
    {
        g_auto_light_mode = AUTO_LIGHT_MODE_MANUAL;
        g_auto_light_inited = 1U;
    }
}

void AutoLight_SetMode(AutoLightMode_t mode)
{
    AutoLight_Init();
    g_auto_light_mode = mode;
}

AutoLightMode_t AutoLight_GetMode(void)
{
    AutoLight_Init();
    return g_auto_light_mode;
}

uint8_t AutoLight_IsAutoMode(void)
{
    return (uint8_t)(AutoLight_GetMode() == AUTO_LIGHT_MODE_AUTO);
}

uint8_t AutoLight_GetTargetDutyFromPercent(uint8_t light_percent)
{
    uint8_t bucket;

    if (light_percent > 100U)
    {
        light_percent = 100U;
    }

    bucket = light_percent / 10U;
    if (bucket >= AUTO_LIGHT_LEVEL_COUNT)
    {
        bucket = (AUTO_LIGHT_LEVEL_COUNT - 1U);
    }

    return (uint8_t)((AUTO_LIGHT_LEVEL_COUNT - bucket) * 10U);
}

uint8_t AutoLight_GetTargetDutyFromVoltage(float light_voltage)
{
    uint8_t bucket;
    float normalized;

    if (light_voltage <= 0.0f)
    {
        return 100U;
    }

    normalized = light_voltage / AUTO_LIGHT_SENSOR_MAX_VOLTAGE;
    if (normalized > 1.0f)
    {
        normalized = 1.0f;
    }

    bucket = (uint8_t)(normalized * (float)AUTO_LIGHT_LEVEL_COUNT);
    if (bucket >= AUTO_LIGHT_LEVEL_COUNT)
    {
        bucket = (AUTO_LIGHT_LEVEL_COUNT - 1U);
    }

    return (uint8_t)((AUTO_LIGHT_LEVEL_COUNT - bucket) * 10U);
}

uint8_t AutoLight_Process(void)
{
    float light_voltage;
    uint8_t target_duty;

    if (AutoLight_IsAutoMode() == 0U)
    {
        return LightPwm_GetDutyPercent();
    }

    light_voltage = LightSensor_ReadVoltageAverage(LIGHT_SENSOR_SAMPLE_COUNT);
    target_duty = AutoLight_GetTargetDutyFromVoltage(light_voltage);
    LightPwm_SetDutyPercent(target_duty);
    return target_duty;
}
