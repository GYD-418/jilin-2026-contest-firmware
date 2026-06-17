#ifndef __FAN_PROGRAM_H__
#define __FAN_PROGRAM_H__

#ifdef __cplusplus
extern "C" {
#endif

#include "main.h"
#include <stdint.h>

void FanProgram_Init(void);
void FanProgram_Start(void);
void FanProgram_Stop(void);
void FanProgram_Process(uint32_t now_tick, uint16_t current_rpm);
uint8_t FanProgram_IsActive(void);
uint8_t FanProgram_IsAlarmActive(void);
uint16_t FanProgram_GetTargetRpm(void);
uint32_t FanProgram_GetElapsedMs(void);
uint8_t FanProgram_GetSegmentIndex(void);
const char *FanProgram_GetStateText(void);
void FanProgram_SetTargets(const uint16_t targets[6]);
void FanProgram_GetTargets(uint16_t targets[6]);

#ifdef __cplusplus
}
#endif

#endif
