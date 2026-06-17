#include "motor_pwm.h"

#include "tim.h"

static uint8_t g_motor_pwm_duty_percent = 0U;

void MotorPwm_Init(void)
{
    HAL_TIM_PWM_Start(&htim2, TIM_CHANNEL_1);
    MotorPwm_SetDutyPercent(0U);
}

void MotorPwm_SetDutyPercent(uint8_t duty_percent)
{
    uint32_t pulse;
    uint32_t period;

    if (duty_percent > 100U)
    {
        duty_percent = 100U;
    }

    period = __HAL_TIM_GET_AUTORELOAD(&htim2);
    pulse = ((period + 1U) * duty_percent) / 100U;
    if (pulse > period)
    {
        pulse = period;
    }

    __HAL_TIM_SET_COMPARE(&htim2, TIM_CHANNEL_1, pulse);
    g_motor_pwm_duty_percent = duty_percent;
}

uint8_t MotorPwm_GetDutyPercent(void)
{
    return g_motor_pwm_duty_percent;
}
