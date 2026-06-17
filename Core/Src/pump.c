#include "pump.h"

#include "servo.h"

static uint8_t g_pump_is_on = 0U;
static uint8_t g_pump_request = 0U;
static uint8_t g_pump_guard_state = 0U;
static uint32_t g_pump_guard_tick = 0U;

#define PUMP_REQUEST_NONE   0U /* 无待处理请求 */
#define PUMP_REQUEST_ON     1U /* 请求开泵 */
#define PUMP_REQUEST_OFF    2U /* 请求关泵 */

#define PUMP_GUARD_IDLE         0U /* 保护状态空闲 */
#define PUMP_GUARD_WAIT_ON      1U /* 等待安全开泵 */
#define PUMP_GUARD_WAIT_OFF     2U /* 等待安全关泵 */

void Pump_Init(void)
{
    Pump_Off();
    g_pump_request = PUMP_REQUEST_NONE;
    g_pump_guard_state = PUMP_GUARD_IDLE;
    g_pump_guard_tick = 0U;
}

void Pump_On(void)
{
    HAL_GPIO_WritePin(shuibeng_GPIO_Port, shuibeng_Pin, GPIO_PIN_SET);
    g_pump_is_on = 1U;
}

void Pump_Off(void)
{
    HAL_GPIO_WritePin(shuibeng_GPIO_Port, shuibeng_Pin, GPIO_PIN_RESET);
    g_pump_is_on = 0U;
}

void Pump_RequestOn(void)
{
    if (g_pump_guard_state == PUMP_GUARD_IDLE)
    {
        Servo_SetAngle(Servo_GetAngle());
        g_pump_request = PUMP_REQUEST_ON;
        g_pump_guard_state = PUMP_GUARD_WAIT_ON;
        g_pump_guard_tick = HAL_GetTick();
    }
}

void Pump_RequestOff(void)
{
    if (g_pump_guard_state == PUMP_GUARD_IDLE)
    {
        Pump_Off();
        g_pump_request = PUMP_REQUEST_OFF;
        g_pump_guard_state = PUMP_GUARD_WAIT_OFF;
        g_pump_guard_tick = HAL_GetTick();
    }
}

void Pump_Process(void)
{
    uint32_t now_tick = HAL_GetTick();

    if (g_pump_guard_state == PUMP_GUARD_WAIT_ON)
    {
        if ((now_tick - g_pump_guard_tick) >= 100U)
        {
            Servo_Disable();
            Pump_On();
            g_pump_request = PUMP_REQUEST_NONE;
            g_pump_guard_state = PUMP_GUARD_IDLE;
        }
    }
    else if (g_pump_guard_state == PUMP_GUARD_WAIT_OFF)
    {
        if ((now_tick - g_pump_guard_tick) >= 100U)
        {
            Servo_Enable();
            Servo_SetAngle(Servo_GetAngle());
            g_pump_request = PUMP_REQUEST_NONE;
            g_pump_guard_state = PUMP_GUARD_IDLE;
        }
    }
}

uint8_t Pump_IsOn(void)
{
    return g_pump_is_on;
}
