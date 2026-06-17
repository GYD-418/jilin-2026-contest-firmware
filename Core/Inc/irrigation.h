#ifndef __IRRIGATION_H__
#define __IRRIGATION_H__

#ifdef __cplusplus
extern "C" {
#endif

#include "main.h"
#include <stdint.h>

/* Irrigation 4-level discrete mode (competition compliance) */
#define IRRIGATION_LEVEL_OFF     0U
#define IRRIGATION_LEVEL_LOW     1U
#define IRRIGATION_LEVEL_MEDIUM  2U
#define IRRIGATION_LEVEL_HIGH    3U
#define IRRIGATION_LEVEL_MAX     3U

void Irrigation_Init(void);
void Irrigation_On(void);
void Irrigation_Off(void);
void Irrigation_Toggle(void);
uint8_t Irrigation_IsOn(void);

/* Continuous duty API (retained for backward compatibility) */
void Irrigation_SetDutyPercent(uint8_t duty_percent);
uint8_t Irrigation_GetDutyPercent(void);

/* 4-level discrete API (competition compliance) */
uint8_t Irrigation_GetLevel(void);
void Irrigation_SetLevel(uint8_t level);
const char *Irrigation_GetLevelName(uint8_t level);

#ifdef __cplusplus
}
#endif

#endif
