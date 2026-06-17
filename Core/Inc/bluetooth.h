#ifndef __BLUETOOTH_H__
#define __BLUETOOTH_H__

#ifdef __cplusplus
extern "C" {
#endif

#include "main.h"
#include <stdint.h>

/* USART1 (PA9/PA10, 9600bps) HC-08 蓝牙透传 */
#define BLUETOOTH_STATUS_PUSH_MS     2000U  /* 周期性推送传感器数据间隔 */

void Bluetooth_Init(void);
void Bluetooth_SendString(const char *str);
void Bluetooth_OnDataReceived(void);
uint8_t Bluetooth_IsConnected(void);

#ifdef __cplusplus
}
#endif

#endif
