#include "stellaris/hw_types.h"
#include "stellaris/hw_memmap.h"
#include "stellaris/driverlib/gpio.h"

#define DIGITAL_GROUP_OUT GPIO_PORTD_BASE
#define DIGITAL_PIN_OUT (GPIO_PIN_4 | GPIO_PIN_5 | GPIO_PIN_6 | GPIO_PIN_7)

#define DIGITAL_GROUP_IN GPIO_PORTF_BASE
#define DIGITAL_PIN_IN (GPIO_PIN_4 | GPIO_PIN_5 | GPIO_PIN_6)

void dev_init(void) {
    GPIODirModeSet(DIGITAL_GROUP_OUT, DIGITAL_PIN_OUT,
                   GPIO_DIR_MODE_OUT);
    GPIODirModeSet(DIGITAL_GROUP_IN, DIGITAL_PIN_IN, GPIO_DIR_MODE_IN);

    GPIOPinTypeGPIOOutputOD(DIGITAL_GROUP_OUT, DIGITAL_PIN_OUT);
    GPIOPinTypeGPIOInput(DIGITAL_GROUP_IN, DIGITAL_PIN_IN);

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
