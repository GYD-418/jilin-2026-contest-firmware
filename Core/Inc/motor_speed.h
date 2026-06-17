#ifndef __MOTOR_SPEED_H__
#define __MOTOR_SPEED_H__

#ifdef __cplusplus
extern "C" {
#endif

#include "main.h"
#include <stdint.h>

/*
 * Adjust this value to match your sensor:
 * - Many fan tach outputs: 2 pulses/rev
 * - Incremental encoder: set to actual pulses/rev of one channel
 */
#define MOTOR_SPEED_PULSES_PER_REV     78U     /* 测速输入每转对应的脉冲数 */
#define MOTOR_SPEED_SAMPLE_PERIOD_MS   500U    /* 转速采样周期 */
#define MOTOR_SPEED_RPM_SCALE          0.794f  /* 基础转速比例修正系数 */

#define MOTOR_SPEED_MAP_LOW_INPUT_RPM    395.0f   /* 低速标定输入点 */
#define MOTOR_SPEED_MAP_LOW_OUTPUT_RPM   500.0f   /* 低速标定目标点 */
#define MOTOR_SPEED_MAP_HIGH_INPUT_RPM   760.0f   /* 高速标定输入点 */
#define MOTOR_SPEED_MAP_HIGH_OUTPUT_RPM  1000.0f  /* 高速标定目标点 */

void MotorSpeed_Init(void);
void MotorSpeed_PulseCallback(uint16_t gpio_pin);
void MotorSpeed_Update(void);
uint32_t MotorSpeed_GetPulseCount(void);
float MotorSpeed_GetFrequencyHz(void);
float MotorSpeed_GetRpm(void);

#ifdef __cplusplus
}
#endif

#endif
