#include "stellaris/hw_types.h"
#include "stellaris/hw_memmap.h"
#include "stellaris/driverlib/gpio.h"

#include "smbus/smbus.h"

#define DIGITAL_GROUP_OUT GPIO_PORTD_BASE
#define DIGITAL_PIN_OUT (GPIO_PIN_4 | GPIO_PIN_5 | GPIO_PIN_6 | GPIO_PIN_7)

#define DIGITAL_GROUP_IN GPIO_PORTF_BASE
#define DIGITAL_PIN_IN (GPIO_PIN_4 | GPIO_PIN_5 | GPIO_PIN_6)

#define SA              0x5A	// Slave address
#define DEFAULT_SA		0x00	// Default Slave address
#define RAM_Access		0x00	// RAM access command
#define EEPROM_Access	0x20	// EEPROM access command
#define RAM_Tobj1		0x07	// To1 address in the eeprom

void dev_init(void) {
    GPIODirModeSet(DIGITAL_GROUP_OUT, DIGITAL_PIN_OUT,
                   GPIO_DIR_MODE_OUT);
    GPIODirModeSet(DIGITAL_GROUP_IN, DIGITAL_PIN_IN, GPIO_DIR_MODE_IN);

    GPIOPinTypeGPIOOutputOD(DIGITAL_GROUP_OUT, DIGITAL_PIN_OUT);
    GPIOPinTypeGPIOInput(DIGITAL_GROUP_IN, DIGITAL_PIN_IN);
}

float mlx90614_get_temperature(void) {
	uint32_t data;
	
	data = memRead(DEFAULT_SA, RAM_Access | RAM_Tobj1);
	
	return ((float)((float) data * 0.02 - 273.15));
}

int set_output(unsigned char group, unsigned char value) {
    if (group == 0x00 || group == 0x0A) {
        GPIOPinWrite(DIGITAL_GROUP_OUT, DIGITAL_PIN_OUT, value);
        return 0;
    } else {
        return -1;
    }
}

unsigned long get_input(void) {
    return GPIOPinRead(DIGITAL_GROUP_OUT, DIGITAL_PIN_OUT);
}

unsigned long int get_io_status(void) {
    return GPIODirModeGet(DIGITAL_GROUP_OUT, 0xff);
}
