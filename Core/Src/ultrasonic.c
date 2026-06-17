#include "ultrasonic.h"
#include "tim.h"

#define ULTRASONIC_MIN_DISTANCE_X10_CM    20U
#define ULTRASONIC_MAX_DISTANCE_X10_CM    500U
#define ULTRASONIC_US_PER_CM_X10         58U
#define ULTRASONIC_ECHO_TIMEOUT_US       60000U
#define ULTRASONIC_MEASURE_INTERVAL_MS   65U   /* 文章建议 ≥60ms */
#define ULTRASONIC_SAMPLE_COUNT          5U

static uint16_t g_last_distance_x10_cm = 0U;
static uint8_t  g_has_valid_data = 0U;

static void Ultrasonic_DelayUs(uint32_t us)
{
    uint32_t start = DWT->CYCCNT;
    uint32_t ticks = us * (SystemCoreClock / 1000000U);
    while ((DWT->CYCCNT - start) < ticks) {}
}

static uint8_t Ultrasonic_WaitForLevel(GPIO_PinState level, uint32_t timeout_us)
{
    uint32_t start = DWT->CYCCNT;
    uint32_t ticks = timeout_us * (SystemCoreClock / 1000000U);
    while ((DWT->CYCCNT - start) < ticks)
    {
        if (HAL_GPIO_ReadPin(ECHO_GPIO_Port, ECHO_Pin) == level)
        {
            return 1U;
        }
    }
    return 0U;
}

void Ultrasonic_Init(void)
{
    CoreDebug->DEMCR |= CoreDebug_DEMCR_TRCENA_Msk;
    DWT->CYCCNT = 0U;
    DWT->CTRL |= DWT_CTRL_CYCCNTENA_Msk;

    HAL_TIM_Base_Start(&htim4);
}

/*
 * 单次测量：阻塞式，与文章相同逻辑
 * 1. TRIG 20us 高脉冲
 * 2. 等待 ECHO 高（超时返回 0）
 * 3. 启动 TIM4 计数
 * 4. 等待 ECHO 低（超时返回 0）
 * 5. 读计数值 → 距离
 */
static uint8_t Ultrasonic_MeasureOnce(uint16_t *distance_x10_cm)
{
    uint32_t echo_us;

    if (distance_x10_cm == NULL)
    {
        return 0U;
    }

    /* 触发：TRIG 20us 高脉冲 */
    HAL_GPIO_WritePin(TRIG_GPIO_Port, TRIG_Pin, GPIO_PIN_SET);
    Ultrasonic_DelayUs(20U);
    HAL_GPIO_WritePin(TRIG_GPIO_Port, TRIG_Pin, GPIO_PIN_RESET);

    /* 等待 ECHO 变高 */
    if (Ultrasonic_WaitForLevel(GPIO_PIN_SET, ULTRASONIC_ECHO_TIMEOUT_US) == 0U)
    {
        return 0U;  /* 超时：无回波 */
    }

    /* 启动计时 */
    __HAL_TIM_SET_COUNTER(&htim4, 0U);

    /* 等待 ECHO 变低 */
    if (Ultrasonic_WaitForLevel(GPIO_PIN_RESET, ULTRASONIC_ECHO_TIMEOUT_US) == 0U)
    {
        return 0U;  /* 超时：回波过长 */
    }

    /* 读取计数值（单位 us，因为 TIM4 分频=83，84MHz/84=1MHz） */
    echo_us = __HAL_TIM_GET_COUNTER(&htim4);

    /* 换算：距离 = echo_us / 58 （单位 0.1cm） */
    *distance_x10_cm = (uint16_t)((echo_us * 10U + (ULTRASONIC_US_PER_CM_X10 / 2U)) / ULTRASONIC_US_PER_CM_X10);

    /* 范围检查 */
    if ((*distance_x10_cm < ULTRASONIC_MIN_DISTANCE_X10_CM) ||
        (*distance_x10_cm > ULTRASONIC_MAX_DISTANCE_X10_CM))
    {
        return 0U;
    }

    return 1U;
}

/*
 * 周期性调用（由 defaultTask 调用）
 * 每 65ms 测一次，立即输出，跳变抑制抗干扰
 */
static uint32_t g_last_measure_tick = 0U;
static uint16_t g_last_output = 0U;

void Ultrasonic_Process(void)
{
    uint32_t now = HAL_GetTick();
    uint16_t dist;

    if ((now - g_last_measure_tick) < ULTRASONIC_MEASURE_INTERVAL_MS)
    {
        return;
    }
    g_last_measure_tick = now;

    if (Ultrasonic_MeasureOnce(&dist) == 0U)
    {
        return;
    }

    /* 抗干扰：与上次输出偏差超过 3cm 则丢弃 */
    if (g_has_valid_data != 0U)
    {
        uint16_t jump = (dist > g_last_output)
                      ? (dist - g_last_output)
                      : (g_last_output - dist);
        if (jump > 30U)
        {
            return;
        }
    }

    g_last_output = dist;
    g_last_distance_x10_cm = dist;
    g_has_valid_data = 1U;
}

uint16_t Ultrasonic_GetLastDistanceX10Cm(void)
{
    return g_last_distance_x10_cm;
}

uint16_t Ultrasonic_GetLastDistanceCm(void)
{
    return (uint16_t)((g_last_distance_x10_cm + 5U) / 10U);
}

uint8_t Ultrasonic_HasValidData(void)
{
    return g_has_valid_data;
}

uint8_t Ultrasonic_ReadDistanceX10Cm(uint16_t *distance_x10_cm)
{
    if (distance_x10_cm == NULL || g_has_valid_data == 0U)
    {
        return 0U;
    }
    *distance_x10_cm = g_last_distance_x10_cm;
    return 1U;
}

uint8_t Ultrasonic_ReadDistanceCm(uint16_t *distance_cm)
{
    uint16_t x10;
    if (Ultrasonic_ReadDistanceX10Cm(&x10) == 0U)
    {
        return 0U;
    }
    *distance_cm = (uint16_t)((x10 + 5U) / 10U);
    return 1U;
}

uint16_t Ultrasonic_ConvertDistanceToWaterLevelX10Cm(uint16_t distance_x10_cm)
{
    if (distance_x10_cm >= WATER_LEVEL_REFERENCE_X10_CM)
    {
        return 0U;
    }
    return (uint16_t)(WATER_LEVEL_REFERENCE_X10_CM - distance_x10_cm);
}

uint8_t Ultrasonic_GetDebugState(void)
{
    return 0U;
}

uint32_t Ultrasonic_GetLastEchoWidthUs(void)
{
    return 0U;
}
