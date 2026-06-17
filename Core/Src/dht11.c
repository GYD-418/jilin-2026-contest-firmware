#include "dht11.h"

static DHT11_Data_t g_dht11_last_data = {0U, 0U};
static uint8_t g_dht11_has_data = 0U;
static uint8_t g_dwt_inited = 0U;

static void DHT11_DelayUs(uint32_t us)
{
    uint32_t start;
    uint32_t ticks;

    if (g_dwt_inited == 0U)
    {
        CoreDebug->DEMCR |= CoreDebug_DEMCR_TRCENA_Msk;
        DWT->CYCCNT = 0U;
        DWT->CTRL |= DWT_CTRL_CYCCNTENA_Msk;
        g_dwt_inited = 1U;
    }

    start = DWT->CYCCNT;
    ticks = us * (SystemCoreClock / 1000000U);

    while ((DWT->CYCCNT - start) < ticks)
    {
    }
}

static void DHT11_SetOutput(void)
{
    GPIO_InitTypeDef GPIO_InitStruct = {0};

    GPIO_InitStruct.Pin = DHT11_DAT_Pin;
    GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_OD;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
    HAL_GPIO_Init(DHT11_DAT_GPIO_Port, &GPIO_InitStruct);
}

static void DHT11_SetInput(void)
{
    GPIO_InitTypeDef GPIO_InitStruct = {0};

    GPIO_InitStruct.Pin = DHT11_DAT_Pin;
    GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    HAL_GPIO_Init(DHT11_DAT_GPIO_Port, &GPIO_InitStruct);
}

static uint8_t DHT11_WaitForLevel(GPIO_PinState level, uint32_t timeout_us)
{
    while (timeout_us-- > 0U)
    {
        if (HAL_GPIO_ReadPin(DHT11_DAT_GPIO_Port, DHT11_DAT_Pin) == level)
        {
            return 1U;
        }
        DHT11_DelayUs(1U);
    }

    return 0U;
}

void DHT11_Init(void)
{
    DHT11_SetOutput();
    HAL_GPIO_WritePin(DHT11_DAT_GPIO_Port, DHT11_DAT_Pin, GPIO_PIN_SET);
    DHT11_DelayUs(50U);
}

uint8_t DHT11_Read(DHT11_Data_t *data)
{
    uint8_t bytes[5] = {0U};
    uint8_t i;
    uint8_t j;

    if (data == NULL)
    {
        return 0U;
    }

    DHT11_SetOutput();
    HAL_GPIO_WritePin(DHT11_DAT_GPIO_Port, DHT11_DAT_Pin, GPIO_PIN_RESET);
    HAL_Delay(20U);
    HAL_GPIO_WritePin(DHT11_DAT_GPIO_Port, DHT11_DAT_Pin, GPIO_PIN_SET);
    DHT11_DelayUs(30U);
    DHT11_SetInput();

    if (DHT11_WaitForLevel(GPIO_PIN_RESET, 100U) == 0U)
    {
        return 0U;
    }
    if (DHT11_WaitForLevel(GPIO_PIN_SET, 100U) == 0U)
    {
        return 0U;
    }
    if (DHT11_WaitForLevel(GPIO_PIN_RESET, 100U) == 0U)
    {
        return 0U;
    }

    for (i = 0U; i < 5U; i++)
    {
        for (j = 0U; j < 8U; j++)
        {
            if (DHT11_WaitForLevel(GPIO_PIN_SET, 100U) == 0U)
            {
                return 0U;
            }

            DHT11_DelayUs(40U);
            bytes[i] <<= 1U;
            if (HAL_GPIO_ReadPin(DHT11_DAT_GPIO_Port, DHT11_DAT_Pin) == GPIO_PIN_SET)
            {
                bytes[i] |= 0x01U;
            }

            if (DHT11_WaitForLevel(GPIO_PIN_RESET, 100U) == 0U)
            {
                return 0U;
            }
        }
    }

    DHT11_SetOutput();
    HAL_GPIO_WritePin(DHT11_DAT_GPIO_Port, DHT11_DAT_Pin, GPIO_PIN_SET);

    if ((uint8_t)(bytes[0] + bytes[1] + bytes[2] + bytes[3]) != bytes[4])
    {
        return 0U;
    }

    data->humidity = bytes[0];
    data->temperature = bytes[2];
    g_dht11_last_data = *data;
    g_dht11_has_data = 1U;

    return 1U;
}

uint8_t DHT11_GetLastData(DHT11_Data_t *data)
{
    if ((data == NULL) || (g_dht11_has_data == 0U))
    {
        return 0U;
    }

    *data = g_dht11_last_data;
    return 1U;
}

uint8_t DHT11_GetHumidity(void)
{
    if (g_dht11_has_data == 0U)
    {
        return 0xFFU;
    }

    return g_dht11_last_data.humidity;
}
