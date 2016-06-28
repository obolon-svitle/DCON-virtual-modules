#include "stellaris/hw_types.h"
#include "stellaris/hw_memmap.h"
#include "stellaris/hw_gpio.h"
#include "stellaris/driverlib/gpio.h"
#include "stellaris/driverlib/pwm.h"
#include "stellaris/driverlib/sysctl.h"

#include "7024_hw.h"

void init_7024(void) {
#ifndef PART_LM3S6965
    SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOE);
    GPIOPinTypePWM(GPIO_PORTE_BASE, GPIO_PIN_7);
    GPIOPinConfigure(GPIO_PE7_PWM5);

    PWMGenConfigure(PWM_BASE, PWM_GEN_2,
                    PWM_GEN_MODE_DOWN | PWM_GEN_MODE_NO_SYNC);
    PWMGenPeriodSet(PWM_BASE, PWM_GEN_2, PWM_PERIOD);
    PWMPulseWidthSet(PWM_BASE, PWM_OUT_5, 1);
    PWMGenEnable(PWM_BASE, PWM_GEN_2);
    PWMOutputState(PWM_BASE, PWM_OUT_5_BIT, true);
#else
    SysCtlPeripheralEnable(SYSCTL_PERIPH_PWM0);
    SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOF);
    GPIOPinTypePWM(GPIO_PORTF_BASE, GPIO_PIN_0);

    PWMGenConfigure(PWM0_BASE, PWM_GEN_0,
                       PWM_GEN_MODE_UP_DOWN | PWM_GEN_MODE_NO_SYNC);
    PWMGenPeriodSet(PWM0_BASE, PWM_GEN_0, PWM_PERIOD);
    PWMPulseWidthSet(PWM0_BASE, PWM_OUT_0, PWM_PERIOD / 4);
    PWMOutputState(PWM0_BASE, PWM_OUT_0_BIT | PWM_OUT_1_BIT, true);
    PWMGenEnable(PWM0_BASE, PWM_GEN_0);
#endif

}

int set_analog_output(unsigned int value) {
    PWMPulseWidthSet(PWM_BASE, PWM_OUT_5, value);
    return 0;
}
