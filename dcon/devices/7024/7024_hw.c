#include "stellaris/hw_types.h"
#include "stellaris/hw_memmap.h"
#include "stellaris/hw_gpio.h"
#include "stellaris/driverlib/gpio.h"
#include "stellaris/driverlib/pwm.h"
#include "stellaris/driverlib/sysctl.h"
#include "stellaris/pin_map.h"

#include "7024_hw.h"

void init_7024(void) {
    SysCtlPeripheralEnable(SYSCTL_PERIPH_PWM);
    SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOE);
    GPIOPinTypePWM(GPIO_PORTE_BASE, GPIO_PIN_7);
    
    GPIOPinConfigure(GPIO_PE7_PWM5);
        
    PWMGenConfigure(PWM_BASE, PWM_GEN_2,
                    PWM_GEN_MODE_DOWN | PWM_GEN_MODE_NO_SYNC);
    PWMGenPeriodSet(PWM_BASE, PWM_GEN_2, PWM_PERIOD);
    PWMPulseWidthSet(PWM_BASE, PWM_OUT_5, 1);
    PWMGenEnable(PWM_BASE, PWM_GEN_2);
    PWMOutputState(PWM_BASE, PWM_OUT_5_BIT, true);
}

int set_analog_output(unsigned int value) {
    PWMPulseWidthSet(PWM_BASE, PWM_OUT_5, value);
    return 0;
}
