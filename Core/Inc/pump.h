#ifndef __PUMP_H__
#define __PUMP_H__

#ifdef __cplusplus
extern "C" {
#endif

#include "main.h"
#include <stdint.h>

void Pump_Init(void);
void Pump_On(void);
void Pump_Off(void);
void Pump_RequestOn(void);
void Pump_RequestOff(void);
void Pump_Process(void);
uint8_t Pump_IsOn(void);

#ifdef __cplusplus
}
#endif

#endif
