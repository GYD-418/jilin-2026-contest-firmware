#ifndef __ULTRASONIC_H__
#define __ULTRASONIC_H__

#ifdef __cplusplus
extern "C" {
#endif

#include "main.h"
#include <stdint.h>
#include <string.h>

#define WATER_LEVEL_REFERENCE_X10_CM    162U /* 传感器到容器底部 16.2cm，单位 0.1cm */

void Ultrasonic_Init(void);
void Ultrasonic_Process(void);
uint8_t Ultrasonic_ReadDistanceX10Cm(uint16_t *distance_x10_cm);
uint8_t Ultrasonic_ReadDistanceCm(uint16_t *distance_cm);
uint16_t Ultrasonic_GetLastDistanceCm(void);
uint16_t Ultrasonic_GetLastDistanceX10Cm(void);
uint8_t Ultrasonic_HasValidData(void);
uint16_t Ultrasonic_ConvertDistanceToWaterLevelX10Cm(uint16_t distance_x10_cm);
uint8_t Ultrasonic_GetDebugState(void);
uint32_t Ultrasonic_GetLastEchoWidthUs(void);

#ifdef __cplusplus
}
#endif

#endif
