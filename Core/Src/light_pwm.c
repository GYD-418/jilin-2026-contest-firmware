#include "light_pwm.h"

#include "tim.h"

typedef struct
{
    TIM_HandleTypeDef *htim;
    uint32_t channel;
} LightChannel_t;

static const LightChannel_t g_light_channels[8] =
{
    {&htim2, TIM_CHANNEL_2}, /* LED1 -> PB3  */
    {&htim2, TIM_CHANNEL_3}, /* LED2 -> PA2  */
    {&htim2, TIM_CHANNEL_4}, /* LED3 -> PA3  */
    {&htim3, TIM_CHANNEL_2}, /* LED4 -> PC7  */
    {&htim3, TIM_CHANNEL_3}, /* LED5 -> PC8  */
    {&htim3, TIM_CHANNEL_4}, /* LED6 -> PC9  */
    {NULL, 0U},              /* LED7 预留，TIM4 已让给超声波 */
    {NULL, 0U}               /* LED8 预留，TIM4 已让给超声波 */
};

static uint8_t g_light_pwm_duty_percent = 0U;

static void LightPwm_SetChannelDuty(const LightChannel_t *channel, uint8_t duty_percent)
{
    uint32_t pulse;
    uint32_t period;

    if (channel == NULL)
    {
        return;
    }

    if (channel->htim == NULL)
    {
        return;
    }

    if (duty_percent > 100U)
    {
        duty_percent = 100U;
    }

    period = __HAL_TIM_GET_AUTORELOAD(channel->htim);
    pulse = ((period + 1U) * duty_percent) / 100U;
    if (pulse > period)
    {
        pulse = period;
    }

    __HAL_TIM_SET_COMPARE(channel->htim, channel->channel, pulse);
}

static void LightPwm_SetGpioLed7(uint8_t on)
{
    HAL_GPIO_WritePin(TIM4_CH1_LED7_GPIO_Port, TIM4_CH1_LED7_Pin,
                      (on != 0U) ? GPIO_PIN_SET : GPIO_PIN_RESET);
}

static void LightPwm_SetGpioLed8(uint8_t on)
{
    HAL_GPIO_WritePin(TIM4_CH2_LED8_GPIO_Port, TIM4_CH2_LED8_Pin,
                      (on != 0U) ? GPIO_PIN_SET : GPIO_PIN_RESET);
}

static void LightPwm_ApplyDuty(uint8_t duty_percent)
{
    uint8_t level;
    uint8_t full_leds;
    uint8_t partial_duty;
    uint8_t i;

    if (duty_percent > 100U)
    {
        duty_percent = 100U;
    }

    level = (duty_percent + 5U) / 10U;
    if (level > 10U)
    {
        level = 10U;
    }

    if (level == 0U)
    {
        for (i = 0U; i < 6U; i++)
        {
            LightPwm_SetChannelDuty(&g_light_channels[i], 0U);
        }
        LightPwm_SetGpioLed7(0U);
        LightPwm_SetGpioLed8(0U);
        return;
    }

    if (level == 9U)
    {
        // 第 9 档（90%）：全部 75%
        for (i = 0U; i < 6U; i++)
        {
            LightPwm_SetChannelDuty(&g_light_channels[i], 75U);
        }
        LightPwm_SetGpioLed7(1U);
        LightPwm_SetGpioLed8(1U);
        return;
    }

    if (level == 10U)
    {
        // 第 10 档（100%）：全部 100%
        for (i = 0U; i < 6U; i++)
        {
            LightPwm_SetChannelDuty(&g_light_channels[i], 100U);
        }
        LightPwm_SetGpioLed7(1U);
        LightPwm_SetGpioLed8(1U);
        return;
    }

    // 第 1~8 档：奇数档顶部 LED 50%，偶数档全部满亮
    // level 1: LED1=50%
    // level 2: LED1=100%
    // level 3: LED1-2=100%, LED3=50%
    // level 4: LED1-2=100%  → 满 2 路
    // level 5: LED1-2=100%, LED3=50% → 满 2 路
    // level 6: LED1-3=100%  → 满 3 路
    // level 7: LED1-3=100%, LED4=50% → 满 3 路
    // level 8: LED1-4=100%  → 满 4 路

    full_leds = level / 2U;
    partial_duty = ((level & 0x01U) != 0U) ? 50U : 0U;

    for (i = 0U; i < 6U; i++)
    {
        if (i < full_leds)
        {
            LightPwm_SetChannelDuty(&g_light_channels[i], 100U);
        }
        else if ((i == full_leds) && (partial_duty != 0U))
        {
            LightPwm_SetChannelDuty(&g_light_channels[i], 50U);
        }
        else
        {
            LightPwm_SetChannelDuty(&g_light_channels[i], 0U);
        }
    }

    // LED7/LED8 随满亮 LED 数逐级点亮
    LightPwm_SetGpioLed7((full_leds >= 4U || (full_leds == 3U && partial_duty != 0U)) ? 1U : 0U);
    LightPwm_SetGpioLed8((full_leds >= 4U) ? 1U : 0U);
}

void LightPwm_Init(void)
{
    GPIO_InitTypeDef gpio_init = {0};

    HAL_TIM_PWM_Start(&htim2, TIM_CHANNEL_2);
    HAL_TIM_PWM_Start(&htim2, TIM_CHANNEL_3);
    HAL_TIM_PWM_Start(&htim2, TIM_CHANNEL_4);
    HAL_TIM_PWM_Start(&htim3, TIM_CHANNEL_2);
    HAL_TIM_PWM_Start(&htim3, TIM_CHANNEL_3);
    HAL_TIM_PWM_Start(&htim3, TIM_CHANNEL_4);
    __HAL_TIM_SET_COMPARE(&htim4, TIM_CHANNEL_1, 0U);
    __HAL_TIM_SET_COMPARE(&htim4, TIM_CHANNEL_2, 0U);

    gpio_init.Pin = TIM4_CH1_LED7_Pin | TIM4_CH2_LED8_Pin;
    gpio_init.Mode = GPIO_MODE_OUTPUT_PP;
    gpio_init.Pull = GPIO_NOPULL;
    gpio_init.Speed = GPIO_SPEED_FREQ_LOW;
    HAL_GPIO_Init(GPIOD, &gpio_init);

    LightPwm_SetGpioLed7(0U);
    LightPwm_SetGpioLed8(0U);
    LightPwm_SetDutyPercent(0U);
}

void LightPwm_SetDutyPercent(uint8_t duty_percent)
{
    if (duty_percent > 100U)
    {
        duty_percent = 100U;
    }

    LightPwm_ApplyDuty(duty_percent);
    g_light_pwm_duty_percent = duty_percent;
}

uint8_t LightPwm_GetDutyPercent(void)
{
    return g_light_pwm_duty_percent;
}
