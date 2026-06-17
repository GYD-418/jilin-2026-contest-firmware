#ifndef __TEMPERATURE_SENSOR_H__
#define __TEMPERATURE_SENSOR_H__

#ifdef __cplusplus
extern "C" {
#endif

#include "main.h"
#include <stdint.h>

#define TEMPERATURE_SENSOR_ADC_MAX_VALUE       4095U    /* 12 位 ADC 满量程 */
#define TEMPERATURE_SENSOR_VREF                3.3f     /* 温度采样参考电压 */
#define TEMPERATURE_SENSOR_SAMPLE_COUNT        16U      /* 温度默认平均采样次数 */
#define TEMPERATURE_SENSOR_PULLUP_RESISTOR     10000.0f /* 分压上拉电阻 */
#define TEMPERATURE_SENSOR_NTC_NOMINAL         10000.0f /* NTC 标称阻值 */
#define TEMPERATURE_SENSOR_NOMINAL_TEMPERATURE 25.0f    /* NTC 标称温度 */
#define TEMPERATURE_SENSOR_BETA                3950.0f  /* NTC Beta 系数 */
#define TEMPERATURE_SENSOR_SERIES_TO_VCC       0U       /* 0: NTC 接地，1: NTC 接 VCC */
#define TEMPERATURE_SENSOR_OFFSET_C            (-12.0f) /* 温度修正偏移量 */

void TemperatureSensor_Init(void);
uint16_t TemperatureSensor_ReadRaw(void);
uint16_t TemperatureSensor_ReadAverage(uint8_t sample_count);
float TemperatureSensor_ReadVoltage(void);
float TemperatureSensor_ReadVoltageAverage(uint8_t sample_count);
float TemperatureSensor_ReadCelsius(void);
float TemperatureSensor_ReadCelsiusAverage(uint8_t sample_count);

#ifdef __cplusplus
}
#endif

#endif
