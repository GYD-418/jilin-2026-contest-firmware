#include "bluetooth.h"
#include "usart.h"
#include <string.h>

static uint8_t g_bluetooth_connected = 0U;

void Bluetooth_Init(void)
{
    g_bluetooth_connected = 0U;
}

void Bluetooth_SendString(const char *str)
{
    uint16_t len;
    uint32_t timeout_ms;

    if (str == NULL)
    {
        return;
    }

    len = (uint16_t)strlen(str);
    if (len == 0U)
    {
        return;
    }

    /* HC-08 默认 9600bps，按报文长度给足发送超时，避免静默丢包 */
    timeout_ms = (uint32_t)len * 2U + 200U;
    (void)HAL_UART_Transmit(&huart1, (uint8_t *)str, len, timeout_ms);
}

uint8_t Bluetooth_IsConnected(void)
{
    return g_bluetooth_connected;
}

void Bluetooth_OnDataReceived(void)
{
    g_bluetooth_connected = 1U;
}
