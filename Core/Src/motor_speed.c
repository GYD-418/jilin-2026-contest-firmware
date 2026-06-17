#include "motor_speed.h"

typedef struct
{
    volatile uint32_t pulse_accumulator;
    uint32_t sample_pulses;
    uint32_t last_sample_tick;
    float frequency_hz;
    float rpm;
} MotorSpeed_State_t;

static MotorSpeed_State_t g_motor_speed = {0};

static float MotorSpeed_MapDisplayRpm(float rpm)
{
    const float low_in = MOTOR_SPEED_MAP_LOW_INPUT_RPM;
    const float low_out = MOTOR_SPEED_MAP_LOW_OUTPUT_RPM;
    const float high_in = MOTOR_SPEED_MAP_HIGH_INPUT_RPM;
    const float high_out = MOTOR_SPEED_MAP_HIGH_OUTPUT_RPM;

    if (rpm <= 0.0f)
    {
        return 0.0f;
    }

    if (low_in <= 0.0f)
    {
        return rpm;
    }

    if (rpm <= low_in)
    {
        return rpm * (low_out / low_in);
    }

    if (high_in <= low_in)
    {
        return rpm;
    }

    if (rpm <= high_in)
    {
        return low_out + ((rpm - low_in) * (high_out - low_out) / (high_in - low_in));
    }

    return high_out + ((rpm - high_in) * (high_out - low_out) / (high_in - low_in));
}

void MotorSpeed_Init(void)
{
    g_motor_speed.pulse_accumulator = 0U;
    g_motor_speed.sample_pulses = 0U;
    g_motor_speed.last_sample_tick = HAL_GetTick();
    g_motor_speed.frequency_hz = 0.0f;
    g_motor_speed.rpm = 0.0f;
}

void MotorSpeed_PulseCallback(uint16_t gpio_pin)
{
    if (gpio_pin == cesu_Pin)
    {
        g_motor_speed.pulse_accumulator++;
    }
}

void MotorSpeed_Update(void)
{
    uint32_t now_tick;
    uint32_t elapsed_ms;
    uint32_t pulses;

    now_tick = HAL_GetTick();
    elapsed_ms = now_tick - g_motor_speed.last_sample_tick;
    if (elapsed_ms < MOTOR_SPEED_SAMPLE_PERIOD_MS)
    {
        return;
    }

    __disable_irq();
    pulses = g_motor_speed.pulse_accumulator;
    g_motor_speed.pulse_accumulator = 0U;
    __enable_irq();

    g_motor_speed.sample_pulses = pulses;
    g_motor_speed.last_sample_tick = now_tick;
    g_motor_speed.frequency_hz = ((float)pulses * 1000.0f) / (float)elapsed_ms;
    g_motor_speed.rpm = ((g_motor_speed.frequency_hz * 60.0f) / (float)MOTOR_SPEED_PULSES_PER_REV) *
                        MOTOR_SPEED_RPM_SCALE;
    g_motor_speed.rpm = MotorSpeed_MapDisplayRpm(g_motor_speed.rpm);
}

uint32_t MotorSpeed_GetPulseCount(void)
{
    return g_motor_speed.sample_pulses;
}

float MotorSpeed_GetFrequencyHz(void)
{
    return g_motor_speed.frequency_hz;
}

float MotorSpeed_GetRpm(void)
{
    return g_motor_speed.rpm;
}
