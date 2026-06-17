#include "irrigation.h"

#include "tim.h"

#define IRRIGATION_DEFAULT_DUTY_PERCENT  100U /* 灌溉默认占空比 */

static uint8_t g_irrigation_is_on = 0U;
static uint8_t g_irrigation_duty_percent = IRRIGATION_DEFAULT_DUTY_PERCENT;

static void Irrigation_ApplyDuty(uint8_t duty_percent)
{
    uint32_t pulse;
    uint32_t period;

    if (duty_percent > 100U)
    {
        duty_percent = 100U;
    }

    period = __HAL_TIM_GET_AUTORELOAD(&htim1);
    pulse = ((period + 1U) * duty_percent) / 100U;
    if (pulse > period)
    {
        pulse = period;
    }

    __HAL_TIM_SET_COMPARE(&htim1, TIM_CHANNEL_4, pulse);
}

void Irrigation_Init(void)
{
    HAL_TIM_PWM_Start(&htim1, TIM_CHANNEL_4);
    HAL_GPIO_WritePin(guangai_GPIO_Port, guangai_Pin, GPIO_PIN_RESET);
    Irrigation_Off();
}

void Irrigation_On(void)
{
    HAL_GPIO_WritePin(guangai_GPIO_Port, guangai_Pin, GPIO_PIN_SET);
    Irrigation_ApplyDuty(g_irrigation_duty_percent);
    g_irrigation_is_on = 1U;
}

void Irrigation_Off(void)
{
    Irrigation_ApplyDuty(0U);
    HAL_GPIO_WritePin(guangai_GPIO_Port, guangai_Pin, GPIO_PIN_RESET);
    g_irrigation_is_on = 0U;
}

void Irrigation_Toggle(void)
{
    if (g_irrigation_is_on != 0U)
    {
        Irrigation_Off();
    }
    else
    {
        Irrigation_On();
    }
}

uint8_t Irrigation_IsOn(void)
{
    return g_irrigation_is_on;
}

void Irrigation_SetDutyPercent(uint8_t duty_percent)
{
    if (duty_percent > 100U)
    {
        duty_percent = 100U;
    }

    g_irrigation_duty_percent = duty_percent;

    if (g_irrigation_is_on != 0U)
    {
        Irrigation_ApplyDuty(g_irrigation_duty_percent);
    }
}

uint8_t Irrigation_GetDutyPercent(void)
{
    return g_irrigation_duty_percent;
}

/* 4 档离散灌溉控制（竞赛要求） */
uint8_t Irrigation_GetLevel(void)
{
    if (g_irrigation_is_on == 0U)
    {
        return IRRIGATION_LEVEL_OFF;
    }

    if (g_irrigation_duty_percent <= 0U)
    {
        return IRRIGATION_LEVEL_OFF;
    }
    else if (g_irrigation_duty_percent <= 33U)
    {
        return IRRIGATION_LEVEL_LOW;
    }
    else if (g_irrigation_duty_percent <= 66U)
    {
        return IRRIGATION_LEVEL_MEDIUM;
    }

    return IRRIGATION_LEVEL_HIGH;
}

void Irrigation_SetLevel(uint8_t level)
{
    if (level > IRRIGATION_LEVEL_MAX)
    {
        level = IRRIGATION_LEVEL_MAX;
    }

    switch (level)
    {
        case IRRIGATION_LEVEL_OFF:
            Irrigation_Off();
            break;

        case IRRIGATION_LEVEL_LOW:
            g_irrigation_duty_percent = 33U;
            Irrigation_On();
            break;

        case IRRIGATION_LEVEL_MEDIUM:
            g_irrigation_duty_percent = 66U;
            Irrigation_On();
            break;

        case IRRIGATION_LEVEL_HIGH:
            g_irrigation_duty_percent = 100U;
            Irrigation_On();
            break;

        default:
            break;
    }
}

const char *Irrigation_GetLevelName(uint8_t level)
{
    switch (level)
    {
        case IRRIGATION_LEVEL_OFF:
            return "OFF";

        case IRRIGATION_LEVEL_LOW:
            return "LOW";

        case IRRIGATION_LEVEL_MEDIUM:
            return "MED";

        case IRRIGATION_LEVEL_HIGH:
            return "HIGH";

        default:
            return "N/A";
    }
}
