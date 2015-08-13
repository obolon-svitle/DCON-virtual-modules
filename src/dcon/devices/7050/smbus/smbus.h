#ifndef _SMBUS_H_
#define _SMBUS_H_

#include "stellaris/hw_memmap.h"
#include "stellaris/hw_types.h"
#include "stellaris/hw_sysctl.h"
#include "stellaris/driverlib/sysctl.h"
#include "stellaris/driverlib/gpio.h"
#include <stdint.h>

#define SDA_PIN         GPIO_PORTH_BASE, GPIO_PIN_6
#define SCL_PIN         GPIO_PORTF_BASE, GPIO_PIN_1

uint16_t memRead(uint8_t SlaveAddress, uint8_t command);

#endif
