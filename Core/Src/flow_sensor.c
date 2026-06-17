#include "flow_sensor.h"

typedef struct
{
    volatile uint32_t pulse_accumulator;
    uint32_t sample_pulses;
    uint32_t total_pulses;
    uint32_t last_sample_tick;
    uint16_t flow_ml_per_min;
    uint32_t total_ml;
} FlowSensor_State_t;

static FlowSensor_State_t g_flow_sensor = {0};

void FlowSensor_Init(void)
{
    g_flow_sensor.pulse_accumulator = 0U;
    g_flow_sensor.sample_pulses = 0U;
    g_flow_sensor.total_pulses = 0U;
    g_flow_sensor.last_sample_tick = HAL_GetTick();
    g_flow_sensor.flow_ml_per_min = 0U;
    g_flow_sensor.total_ml = 0U;
}

void FlowSensor_PulseCallback(uint16_t gpio_pin)
{
    if (gpio_pin == liuliang_Pin)
    {
        g_flow_sensor.pulse_accumulator++;
    }
}

void FlowSensor_Update(void)
{
    uint32_t now_tick;
    uint32_t elapsed_ms;
    uint32_t pulses;
    uint32_t numerator;

    now_tick = HAL_GetTick();
    elapsed_ms = now_tick - g_flow_sensor.last_sample_tick;
    if (elapsed_ms < FLOW_SENSOR_SAMPLE_PERIOD_MS)
    {
        return;
    }

    __disable_irq();
    pulses = g_flow_sensor.pulse_accumulator;
    g_flow_sensor.pulse_accumulator = 0U;
    __enable_irq();

    g_flow_sensor.sample_pulses = pulses;
    g_flow_sensor.total_pulses += pulses;
    g_flow_sensor.last_sample_tick = now_tick;

    if ((FLOW_SENSOR_PULSES_PER_LITER == 0U) || (elapsed_ms == 0U))
    {
        g_flow_sensor.flow_ml_per_min = 0U;
    }
    else
    {
        numerator = pulses * 60000UL;
        g_flow_sensor.flow_ml_per_min =
            (uint16_t)((numerator + ((FLOW_SENSOR_PULSES_PER_LITER * elapsed_ms) / 2UL)) /
                       (FLOW_SENSOR_PULSES_PER_LITER * elapsed_ms));
    }

    g_flow_sensor.total_ml =
        (g_flow_sensor.total_pulses * 1000UL + (FLOW_SENSOR_PULSES_PER_LITER / 2UL)) /
        FLOW_SENSOR_PULSES_PER_LITER;
}

uint32_t FlowSensor_GetPulseCount(void)
{
    return g_flow_sensor.sample_pulses;
}

uint16_t FlowSensor_GetFlowMlPerMin(void)
{
    return g_flow_sensor.flow_ml_per_min;
}

uint32_t FlowSensor_GetTotalMl(void)
{
    return g_flow_sensor.total_ml;
}
