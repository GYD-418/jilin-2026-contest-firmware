#include "temperature_sensor.h"

#include "adc.h"
#include <math.h>

static uint8_t g_temperature_sensor_inited = 0U;

static uint16_t TemperatureSensor_ReadChannel(uint32_t channel)
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

static float TemperatureSensor_RawToCelsius(uint16_t raw)
{
    float adc_ratio;
    float resistance;
    float steinhart;

    if (raw == 0U)
    {
        return -273.15f;
    }

    if (raw >= TEMPERATURE_SENSOR_ADC_MAX_VALUE)
    {
        raw = TEMPERATURE_SENSOR_ADC_MAX_VALUE - 1U;
    }

    adc_ratio = (float)raw / (float)(TEMPERATURE_SENSOR_ADC_MAX_VALUE - raw);

    if (TEMPERATURE_SENSOR_SERIES_TO_VCC != 0U)
    {
        resistance = TEMPERATURE_SENSOR_PULLUP_RESISTOR / adc_ratio;
    }
    else
    {
        resistance = TEMPERATURE_SENSOR_PULLUP_RESISTOR * adc_ratio;
    }

    steinhart = resistance / TEMPERATURE_SENSOR_NTC_NOMINAL;
    steinhart = logf(steinhart);
    steinhart /= TEMPERATURE_SENSOR_BETA;
    steinhart += 1.0f / (TEMPERATURE_SENSOR_NOMINAL_TEMPERATURE + 273.15f);
    steinhart = 1.0f / steinhart;

    return (steinhart - 273.15f) + TEMPERATURE_SENSOR_OFFSET_C;
}

void TemperatureSensor_Init(void)
{
    if (g_temperature_sensor_inited == 0U)
    {
        g_temperature_sensor_inited = 1U;
    }
}

uint16_t TemperatureSensor_ReadRaw(void)
{
    TemperatureSensor_Init();
    return TemperatureSensor_ReadChannel(ADC_CHANNEL_1);
}

uint16_t TemperatureSensor_ReadAverage(uint8_t sample_count)
{
    uint32_t sum = 0U;
    uint8_t i;

    if (sample_count == 0U)
    {
        sample_count = TEMPERATURE_SENSOR_SAMPLE_COUNT;
    }

    for (i = 0U; i < sample_count; i++)
    {
        sum += TemperatureSensor_ReadRaw();
    }

    return (uint16_t)(sum / sample_count);
}

float TemperatureSensor_ReadVoltage(void)
{
    return (TEMPERATURE_SENSOR_VREF * (float)TemperatureSensor_ReadRaw()) / (float)TEMPERATURE_SENSOR_ADC_MAX_VALUE;
}

float TemperatureSensor_ReadVoltageAverage(uint8_t sample_count)
{
    return (TEMPERATURE_SENSOR_VREF * (float)TemperatureSensor_ReadAverage(sample_count)) / (float)TEMPERATURE_SENSOR_ADC_MAX_VALUE;
}

float TemperatureSensor_ReadCelsius(void)
{
    return TemperatureSensor_RawToCelsius(TemperatureSensor_ReadRaw());
}

float TemperatureSensor_ReadCelsiusAverage(uint8_t sample_count)
{
    return TemperatureSensor_RawToCelsius(TemperatureSensor_ReadAverage(sample_count));
}
