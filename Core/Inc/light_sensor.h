#ifndef __LIGHT_SENSOR_H__
#define __LIGHT_SENSOR_H__

#ifdef __cplusplus
extern "C" {
#endif

#include "main.h"
#include <stdint.h>

#define LIGHT_SENSOR_ADC_MAX_VALUE   4095U  /* 12 位 ADC 满量程 */
#define LIGHT_SENSOR_VREF            3.3f   /* 光照采样参考电压 */
#define LIGHT_SENSOR_SAMPLE_COUNT    16U    /* 光照默认平均采样次数 */

void LightSensor_Init(void);
uint16_t LightSensor_ReadRaw(void);
uint16_t LightSensor_ReadAverage(uint8_t sample_count);
float LightSensor_ReadVoltage(void);
float LightSensor_ReadVoltageAverage(uint8_t sample_count);
uint8_t LightSensor_ReadPercent(void);

#ifdef __cplusplus
}
#endif

#endif
