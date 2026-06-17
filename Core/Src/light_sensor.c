#include "light_sensor.h"

#include "adc.h"

static uint8_t g_light_sensor_inited = 0U;

static uint16_t LightSensor_ReadChannel(uint32_t channel)
{
    ADC_ChannelConfTypeDef sConfig = {0};

    sConfig.Channel = channel;
    sConfig.Rank = 1U;
    sConfig.SamplingTime = ADC_SAMPLETIME_84CYCLES;

    if (HAL_ADC_ConfigChannel(&hadc1, &sConfig) != HAL_OK)
    {
        return 0U;
    }

    if (HAL_ADC_Start(&hadc1) != HAL_OK)
    {
        return 0U;
    }

    if (HAL_ADC_PollForConversion(&hadc1, 10U) != HAL_OK)
    {
        (void)HAL_ADC_Stop(&hadc1);
        return 0U;
    }

    {
        uint16_t value = (uint16_t)HAL_ADC_GetValue(&hadc1);
        (void)HAL_ADC_Stop(&hadc1);
        return value;
    }
}

void LightSensor_Init(void)
{
    if (g_light_sensor_inited == 0U)
    {
        g_light_sensor_inited = 1U;
    }
}

uint16_t LightSensor_ReadRaw(void)
{
    LightSensor_Init();
    return LightSensor_ReadChannel(ADC_CHANNEL_0);
}

uint16_t LightSensor_ReadAverage(uint8_t sample_count)
{
    uint32_t sum = 0U;
    uint8_t i;

    if (sample_count == 0U)
    {
        sample_count = LIGHT_SENSOR_SAMPLE_COUNT;
    }

    for (i = 0U; i < sample_count; i++)
    {
        sum += LightSensor_ReadRaw();
    }

    return (uint16_t)(sum / sample_count);
}

float LightSensor_ReadVoltage(void)
{
    return (LIGHT_SENSOR_VREF * (float)LightSensor_ReadRaw()) / (float)LIGHT_SENSOR_ADC_MAX_VALUE;
}

float LightSensor_ReadVoltageAverage(uint8_t sample_count)
{
    return (LIGHT_SENSOR_VREF * (float)LightSensor_ReadAverage(sample_count)) / (float)LIGHT_SENSOR_ADC_MAX_VALUE;
}

uint8_t LightSensor_ReadPercent(void)
{
    uint32_t percent = ((uint32_t)LightSensor_ReadAverage(LIGHT_SENSOR_SAMPLE_COUNT) * 100U) / LIGHT_SENSOR_ADC_MAX_VALUE;

    if (percent > 100U)
    {
        percent = 100U;
    }

    return (uint8_t)percent;
}
