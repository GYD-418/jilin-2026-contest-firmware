#ifndef __FLOW_SENSOR_H__
#define __FLOW_SENSOR_H__

#ifdef __cplusplus
extern "C" {
#endif

#include "main.h"
#include <stdint.h>

#define FLOW_SENSOR_PULSES_PER_LITER     588U   /* YF-S401 每升脉冲数，先按常用值 */
#define FLOW_SENSOR_SAMPLE_PERIOD_MS     5000U  /* 流量采样周期，拉长窗口便于小流量检测 */

void FlowSensor_Init(void);
void FlowSensor_PulseCallback(uint16_t gpio_pin);
void FlowSensor_Update(void);
uint32_t FlowSensor_GetPulseCount(void);
uint16_t FlowSensor_GetFlowMlPerMin(void);
uint32_t FlowSensor_GetTotalMl(void);

#ifdef __cplusplus
}
#endif

#endif
