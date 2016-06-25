#include "stellaris/hw_types.h"
#include "stellaris/hw_memmap.h"
#include "stellaris/hw_ints.h"
#include "stellaris/driverlib/sysctl.h"
#include "stellaris/driverlib/adc.h"
#include "stellaris/driverlib/gpio.h"
#include "stellaris/driverlib/interrupt.h"

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/semphr.h"

#include "7017_hw.h"

#include "dcon/dcon_dev_common.h"

#define MAX_VOLTAGE 3.3
#define RESOLUTION 1024

static unsigned long adc_vals[CHANNEL_COUNT];
static SemaphoreHandle_t adc_mutex;

void ADCIntHandler(void) {
    while(!ADCIntStatus(ADC0_BASE, 3, false));
    ADCIntClear(ADC0_BASE, 3);
    ADCSequenceDataGet(ADC0_BASE, 3, adc_vals);
    ADCProcessorTrigger(ADC0_BASE, 3);
}

void get_voltage(int channel, int *intpart, int *floatpart) {
    xSemaphoreTake(adc_mutex, portMAX_DELAY);

    float2int((float) adc_vals[channel] / RESOLUTION * MAX_VOLTAGE, intpart, floatpart);
    xSemaphoreGive(adc_mutex);
}

void init_7017(void) {
    adc_mutex = xSemaphoreCreateMutex();
    if (adc_mutex == NULL) {
        return;
    }        

    SysCtlPeripheralEnable(SYSCTL_PERIPH_ADC0);
    SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOD);
    GPIOPinTypeADC(GPIO_PORTD_BASE, GPIO_PIN_6 | GPIO_PIN_7);
    
    ADCSequenceDisable(ADC0_BASE, 3);
    ADCSequenceConfigure(ADC0_BASE, 3, ADC_TRIGGER_PROCESSOR, 0);

    ADCSequenceStepConfigure(ADC0_BASE, 3, 0, ADC_CTL_CH4);
    ADCSequenceStepConfigure(ADC0_BASE, 3, 1, ADC_CTL_CH5 | ADC_CTL_IE | ADC_CTL_END);
    ADCIntRegister(ADC0_BASE, 3, ADCIntHandler);
    
    ADCIntEnable(ADC0_BASE, 3);
    ADCSequenceEnable(ADC_BASE, 3);
    ADCIntClear(ADC0_BASE, 3);

    ADCProcessorTrigger(ADC0_BASE, 3);


}
