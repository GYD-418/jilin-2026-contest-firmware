#ifndef __AUTO_LIGHT_H__
#define __AUTO_LIGHT_H__

#ifdef __cplusplus
extern "C" {
#endif

#include "main.h"
#include <stdint.h>

typedef enum
{
    AUTO_LIGHT_MODE_MANUAL = 0U,
    AUTO_LIGHT_MODE_AUTO = 1U
} AutoLightMode_t;

void AutoLight_Init(void);
void AutoLight_SetMode(AutoLightMode_t mode);
AutoLightMode_t AutoLight_GetMode(void);
uint8_t AutoLight_IsAutoMode(void);
uint8_t AutoLight_Process(void);
uint8_t AutoLight_GetTargetDutyFromPercent(uint8_t light_percent);
uint8_t AutoLight_GetTargetDutyFromVoltage(float light_voltage);

#ifdef __cplusplus
}
#endif

#endif
