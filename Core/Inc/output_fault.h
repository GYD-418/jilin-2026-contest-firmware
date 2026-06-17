#ifndef __OUTPUT_FAULT_H__
#define __OUTPUT_FAULT_H__

#ifdef __cplusplus
extern "C" {
#endif

#include "main.h"
#include <stdint.h>

#define OUTPUT_FAULT_FLAG_FAN           (1U << 0)
#define OUTPUT_FAULT_FLAG_PUMP          (1U << 1)
#define OUTPUT_FAULT_FLAG_IRRIGATION    (1U << 2)
#define OUTPUT_FAULT_FLAG_CURTAIN       (1U << 3)
#define OUTPUT_FAULT_FLAG_BUZZER        (1U << 4)
#define OUTPUT_FAULT_FLAG_ALARM_LED     (1U << 5)
#define OUTPUT_FAULT_FLAG_LIGHT         (1U << 6)
#define OUTPUT_FAULT_FLAG_FLOW_LEAK     (1U << 7)

#define OUTPUT_FAULT_CHECK_MS           500U

void OutputFault_Init(void);
void OutputFault_Process(uint32_t now_tick);
uint16_t OutputFault_GetFlags(void);
uint8_t OutputFault_HasAnyFault(void);
const char *OutputFault_GetFlagsString(char *buf, uint16_t buf_size);

#ifdef __cplusplus
}
#endif

#endif
