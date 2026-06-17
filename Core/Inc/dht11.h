#ifndef __DHT11_H__
#define __DHT11_H__

#ifdef __cplusplus
extern "C" {
#endif

#include "main.h"
#include <stdint.h>

typedef struct
{
    uint8_t humidity;
    uint8_t temperature; /* Deprecated: DHT11-internal only. Use NTC (temperature_sensor.c) for compliance. */
} DHT11_Data_t;

void DHT11_Init(void);
uint8_t DHT11_Read(DHT11_Data_t *data);
uint8_t DHT11_GetLastData(DHT11_Data_t *data);
uint8_t DHT11_GetHumidity(void);

#ifdef __cplusplus
}
#endif

#endif
