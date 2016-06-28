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

#define MODULE_ADC_PERIPH SYSCTL_PERIPH_ADC0
#define MODULE_ADC_BASE ADC0_BASE
#define MODULE_ADC_SEQUENCE_NUM 3
#define MODULE_ADC_SEQUENCE_0_CONFIG (ADC_CTL_CH4)
#define MODULE_ADC_SEQUENCE_1_CONFIG (ADC_CTL_CH5 | ADC_CTL_IE | ADC_CTL_END)

#define MODULE_CONFIG_GPIO() do { SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOD); \
	GPIOPinTypeADC(GPIO_PORTD_BASE, GPIO_PIN_6 | GPIO_PIN_7); } while (0)

static unsigned long adc_vals[CHANNEL_COUNT];
static xSemaphoreHandle adc_mutex;

void ADCIntHandler(void) {
    while(!ADCIntStatus(MODULE_ADC_BASE, MODULE_ADC_SEQUENCE_NUM, false));
    ADCIntClear(MODULE_ADC_BASE, MODULE_ADC_SEQUENCE_NUM);

    xSemaphoreTakeFromISR(adc_mutex, pdFALSE);
    ADCSequenceDataGet(MODULE_ADC_BASE, MODULE_ADC_SEQUENCE_NUM, adc_vals);
    xSemaphoreGiveFromISR(adc_mutex, pdTRUE);

    ADCProcessorTrigger(MODULE_ADC_BASE, MODULE_ADC_SEQUENCE_NUM);
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

    SysCtlPeripheralEnable(MODULE_ADC_PERIPH);

    MODULE_CONFIG_GPIO();

    ADCSequenceDisable(MODULE_ADC_BASE, MODULE_ADC_SEQUENCE_NUM);
    ADCSequenceConfigure(MODULE_ADC_BASE, MODULE_ADC_SEQUENCE_NUM, ADC_TRIGGER_PROCESSOR, 0);

    ADCSequenceStepConfigure(MODULE_ADC_BASE, MODULE_ADC_SEQUENCE_NUM, 0, MODULE_ADC_SEQUENCE_1_CONFIG);
    ADCSequenceStepConfigure(MODULE_ADC_BASE, MODULE_ADC_SEQUENCE_NUM, 1, MODULE_ADC_SEQUENCE_1_CONFIG);
    ADCIntRegister(MODULE_ADC_BASE, MODULE_ADC_SEQUENCE_NUM, ADCIntHandler);
    
    ADCIntEnable(MODULE_ADC_BASE, MODULE_ADC_SEQUENCE_NUM);
    ADCSequenceEnable(ADC_BASE, MODULE_ADC_SEQUENCE_NUM);
    ADCIntClear(MODULE_ADC_BASE, MODULE_ADC_SEQUENCE_NUM);

    ADCProcessorTrigger(MODULE_ADC_BASE, MODULE_ADC_SEQUENCE_NUM);
}
