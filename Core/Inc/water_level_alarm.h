#ifndef __WATER_LEVEL_ALARM_H__
#define __WATER_LEVEL_ALARM_H__

#ifdef __cplusplus
extern "C" {
#endif

#include "main.h"
#include <stdint.h>

#define WATER_LEVEL_ALARM_DEFAULT_THRESHOLD_X10_CM 120U  /* 默认水位报警阈值 12.0cm */

void WaterLevelAlarm_Init(void);
void WaterLevelAlarm_Enable(void);
void WaterLevelAlarm_Disable(void);
void WaterLevelAlarm_SetThresholdX10Cm(uint16_t threshold_x10_cm);
uint16_t WaterLevelAlarm_GetThresholdX10Cm(void);
uint8_t WaterLevelAlarm_IsEnabled(void);
uint8_t WaterLevelAlarm_IsActive(void);
void WaterLevelAlarm_Process(uint16_t water_level_x10_cm);

#ifdef __cplusplus
}
#endif

#endif
