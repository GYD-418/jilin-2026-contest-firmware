#ifndef __LIGHT_PWM_H__
#define __LIGHT_PWM_H__

#ifdef __cplusplus
extern "C" {
#endif

#include "main.h"
#include <stdint.h>

void LightPwm_Init(void);
void LightPwm_SetDutyPercent(uint8_t duty_percent);
uint8_t LightPwm_GetDutyPercent(void);

#ifdef __cplusplus
}
#endif

#endif
