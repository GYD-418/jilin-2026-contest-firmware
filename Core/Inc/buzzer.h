#ifndef __BUZZER_H__
#define __BUZZER_H__

#ifdef __cplusplus
extern "C" {
#endif

#include "main.h"
#include <stdint.h>

void Buzzer_Init(void);
void Buzzer_On(void);
void Buzzer_Off(void);
void Buzzer_AutoOn(void);
void Buzzer_AutoOff(void);
void Buzzer_Toggle(void);
uint8_t Buzzer_IsOn(void);
void Buzzer_Process(void);

#ifdef __cplusplus
}
#endif

#endif
