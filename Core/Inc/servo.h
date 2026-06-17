#ifndef __SERVO_H__
#define __SERVO_H__

#ifdef __cplusplus
extern "C" {
#endif

#include "main.h"
#include <stdint.h>

#define SERVO_GATE_CLOSE_ANGLE  0U    /* 闸门关闭角度 */
#define SERVO_GATE_OPEN_ANGLE   230U  /* 闸门打开角度 */

void Servo_Init(void);
void Servo_Enable(void);
void Servo_Disable(void);
void Servo_SetAngle(uint16_t angle);
uint16_t Servo_GetAngle(void);
void Servo_GateClose(void);
void Servo_GateOpen(void);

#ifdef __cplusplus
}
#endif

#endif
