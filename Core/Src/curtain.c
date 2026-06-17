#include "curtain.h"

static CurtainState_t g_curtain_state = CURTAIN_STATE_STOP;

void Curtain_Init(void)
{
    Curtain_Stop();
}

void Curtain_Forward(void)
{
    HAL_GPIO_WritePin(juanlian_fan_GPIO_Port, juanlian_fan_Pin, GPIO_PIN_RESET);
    HAL_GPIO_WritePin(juanlian_zheng_GPIO_Port, juanlian_zheng_Pin, GPIO_PIN_SET);
    g_curtain_state = CURTAIN_STATE_FORWARD;
}

void Curtain_Reverse(void)
{
    HAL_GPIO_WritePin(juanlian_zheng_GPIO_Port, juanlian_zheng_Pin, GPIO_PIN_RESET);
    HAL_GPIO_WritePin(juanlian_fan_GPIO_Port, juanlian_fan_Pin, GPIO_PIN_SET);
    g_curtain_state = CURTAIN_STATE_REVERSE;
}

void Curtain_Stop(void)
{
    HAL_GPIO_WritePin(juanlian_zheng_GPIO_Port, juanlian_zheng_Pin, GPIO_PIN_RESET);
    HAL_GPIO_WritePin(juanlian_fan_GPIO_Port, juanlian_fan_Pin, GPIO_PIN_RESET);
    g_curtain_state = CURTAIN_STATE_STOP;
}

CurtainState_t Curtain_GetState(void)
{
    return g_curtain_state;
}
