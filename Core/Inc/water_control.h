#ifndef __WATER_CONTROL_H__
#define __WATER_CONTROL_H__

#ifdef __cplusplus
extern "C" {
#endif

#include "main.h"
#include <stdint.h>

#define WATER_CONTROL_DEFAULT_TARGET_X10_CM  50U
#define WATER_CONTROL_HYSTERESIS_X10_CM      10U
#define WATER_CONTROL_PERIOD_MS              500U
#define WATER_CONTROL_MAX_RUN_MS             300000U
#define WATER_CONTROL_MIN_FLOW_ML_PER_MIN    50U

void WaterControl_Init(void);
void WaterControl_Process(uint32_t now_tick);
void WaterControl_SetTargetX10Cm(uint16_t target_x10_cm);
uint16_t WaterControl_GetTargetX10Cm(void);
void WaterControl_SetAutoMode(uint8_t enable);
uint8_t WaterControl_IsAutoMode(void);
uint8_t WaterControl_IsPumpProtectionActive(void);
const char *WaterControl_GetStateString(void);

#ifdef __cplusplus
}
#endif

#endif
