#ifndef __MOTOR_PWM_H__
#define __MOTOR_PWM_H__

#ifdef __cplusplus
extern "C" {
#endif

#include "main.h"
#include <stdint.h>

void MotorPwm_Init(void);
void MotorPwm_SetDutyPercent(uint8_t duty_percent);
uint8_t MotorPwm_GetDutyPercent(void);

#ifdef __cplusplus
}
#endif

#endif
