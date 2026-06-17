#include "buzzer.h"

static uint8_t g_buzzer_is_on = 0U;
static uint8_t g_buzzer_manual_on = 0U;
static uint8_t g_buzzer_auto_on = 0U;

static void Buzzer_UpdateState(void)
{
    g_buzzer_is_on = (uint8_t)((g_buzzer_manual_on != 0U) || (g_buzzer_auto_on != 0U));
    if (g_buzzer_is_on != 0U)
    {
        HAL_GPIO_WritePin(fengmingqi_GPIO_Port, fengmingqi_Pin, GPIO_PIN_RESET);
    }
    else
    {
        HAL_GPIO_WritePin(fengmingqi_GPIO_Port, fengmingqi_Pin, GPIO_PIN_SET);
    }
}

void Buzzer_Init(void)
{
    g_buzzer_manual_on = 0U;
    g_buzzer_auto_on = 0U;
    Buzzer_Off();
}

void Buzzer_On(void)
{
    g_buzzer_manual_on = 1U;
    Buzzer_UpdateState();
}

void Buzzer_Off(void)
{
    g_buzzer_manual_on = 0U;
    g_buzzer_auto_on = 0U;
    Buzzer_UpdateState();
}

void Buzzer_AutoOn(void)
{
    g_buzzer_auto_on = 1U;
    Buzzer_UpdateState();
}

void Buzzer_AutoOff(void)
{
    g_buzzer_auto_on = 0U;
    Buzzer_UpdateState();
}

void Buzzer_Toggle(void)
{
    if (g_buzzer_is_on != 0U)
    {
        Buzzer_Off();
    }
    else
    {
        Buzzer_On();
    }
}

uint8_t Buzzer_IsOn(void)
{
    return g_buzzer_is_on;
}

void Buzzer_Process(void)
{
    if (g_buzzer_is_on != 0U)
    {
        HAL_GPIO_TogglePin(fengmingqi_GPIO_Port, fengmingqi_Pin);
    }
    else
    {
        HAL_GPIO_WritePin(fengmingqi_GPIO_Port, fengmingqi_Pin, GPIO_PIN_SET);
    }
}
