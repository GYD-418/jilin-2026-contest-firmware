#ifndef __ALARM_LED_H__
#define __ALARM_LED_H__

#ifdef __cplusplus
extern "C" {
#endif

#include "main.h"
#include <stdint.h>

void AlarmLed_Init(void);
void AlarmLed_On(void);
void AlarmLed_Off(void);
void AlarmLed_AutoOn(void);
void AlarmLed_AutoOff(void);
void AlarmLed_Toggle(void);
uint8_t AlarmLed_IsOn(void);

#ifdef __cplusplus
}
#endif

#endif
