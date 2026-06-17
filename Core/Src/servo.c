#include "servo.h"

#include "tim.h"
#include "gpio.h"

#define SERVO_MIN_PULSE_US   900U  /* 舵机最小脉宽 */
#define SERVO_MAX_PULSE_US   2100U /* 舵机最大脉宽 */
#define SERVO_MAX_ANGLE      270U  /* 舵机最大控制角度 */

static uint16_t g_servo_angle = SERVO_GATE_CLOSE_ANGLE;
static uint8_t g_servo_enabled = 0U;

void Servo_Init(void)
{
    GPIO_InitTypeDef gpio_init = {0};

    /* PA6 → TIM3_CH1，替代原 PC6 */
    __HAL_RCC_GPIOA_CLK_ENABLE();
    gpio_init.Pin = SERVO_Signal_Pin;
    gpio_init.Mode = GPIO_MODE_AF_PP;
    gpio_init.Pull = GPIO_NOPULL;
    gpio_init.Speed = GPIO_SPEED_FREQ_LOW;
    gpio_init.Alternate = GPIO_AF2_TIM3;
    HAL_GPIO_Init(SERVO_Signal_GPIO_Port, &gpio_init);

    Servo_Enable();
    Servo_GateClose();
}

void Servo_Enable(void)
{
    HAL_TIM_PWM_Start(&htim3, TIM_CHANNEL_1);
    g_servo_enabled = 1U;
}

void Servo_Disable(void)
{
    HAL_TIM_PWM_Stop(&htim3, TIM_CHANNEL_1);
    g_servo_enabled = 0U;
}

void Servo_SetAngle(uint16_t angle)
{
    uint32_t pulse;

    if (angle > SERVO_MAX_ANGLE)
    {
        angle = SERVO_MAX_ANGLE;
    }

    pulse = SERVO_MIN_PULSE_US +
            (((uint32_t)(SERVO_MAX_PULSE_US - SERVO_MIN_PULSE_US) * angle) / SERVO_MAX_ANGLE);

    __HAL_TIM_SET_COMPARE(&htim3, TIM_CHANNEL_1, pulse);
    g_servo_angle = angle;
}

uint16_t Servo_GetAngle(void)
{
    return g_servo_angle;
}

void Servo_GateClose(void)
{
    Servo_SetAngle(SERVO_GATE_CLOSE_ANGLE);
}

void Servo_GateOpen(void)
{
    Servo_SetAngle(SERVO_GATE_OPEN_ANGLE);
}
