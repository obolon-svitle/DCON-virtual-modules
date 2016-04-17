#include "stellaris/hw_types.h"
#include "stellaris/hw_memmap.h"
#include "stellaris/hw_ints.h"
#include "stellaris/driverlib/sysctl.h"
#include "stellaris/driverlib/adc.h"
#include "stellaris/driverlib/gpio.h"
#include "stellaris/driverlib/interrupt.h"

#include "7017_hw.h"

#define ADC_DIVIDER 170.5
#define MAX_VOLTAGE 5
#define ADC_CHANNEL_COUNT 8

void ADCIntHandler(void) {
    unsigned long adc_vals[ADC_CHANNEL_COUNT];

    ADCIntClear(ADC0_BASE, 0);

    xSemaphoreTakeFromISR(adc_mutex, NULL);
    ADCSequenceDataGet(ADC0_BASE, 0, &(adc_vals[0]));
    xSemaphoreGiveFromISR(adc_mutex, NULL);
}

void dev_init(void) {
    unsigned long ulDummy = 0;

    adc_mutex = xSemaphoreCreateMutex();
    if (adc_mutex == NULL) {
        return;
    }        

    SysCtlPeripheralEnable(SYSCTL_PERIPH_ADC0);
    SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOD);
    SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOB);
    GPIOPinTypeADC(GPIO_PORTC_BASE,
                   GPIO_PIN_7| GPIO_PIN_6| GPIO_PIN_5| GPIO_PIN_4);
    GPIOPinTypeADC(GPIO_PORTE_BASE, GPIO_PIN_1 | GPIO_PIN_2);

    ADCSequenceDisable(ADC0_BASE, 0);
    ADCSequenceConfigure(ADC0_BASE, 0, ADC_TRIGGER_PROCESSOR, 0);

    ADCSequenceStepConfigure(ADC0_BASE, 0, 1, ADC_CTL_CH11);
    ADCSequenceStepConfigure(ADC0_BASE, 0, 2, ADC_CTL_CH4);                   
    ADCSequenceStepConfigure(ADC0_BASE, 0, 3, ADC_CTL_CH5);                   
    ADCSequenceStepConfigure(ADC0_BASE, 0, 4, ADC_CTL_CH6);                   
    ADCSequenceStepConfigure(ADC0_BASE, 0, 5, ADC_CTL_CH7 | ADC_CTL_IE | ADC_CTL_END);
    ADCSequenceEnable(ADC0_BASE, 0);
    
    ADCSequenceDataGet(ADC0_BASE, 0, &ulDummy);

    ADCIntEnable(ADC0_BASE, 0);
    IntEnable(INT_ADC0);
}
