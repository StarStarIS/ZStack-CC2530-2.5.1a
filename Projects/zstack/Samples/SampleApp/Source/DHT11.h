#ifndef DHT11_H
#define DHT11_H

#include "hal_types.h"

extern uint8 DHT11_LastErr;
extern uint8 DHT11_LastRaw[5];

void DHT11_Init(void);
uint8 DHT11_Read(uint8 *temperature, uint8 *humidity);

#endif