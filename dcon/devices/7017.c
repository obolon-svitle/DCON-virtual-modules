#include <stdio.h>
#include <stdarg.h>
#include <stdlib.h>
#include <string.h>

#include "stellaris/hw_types.h"
#include "stellaris/hw_memmap.h"
#include "stellaris/hw_ints.h"
#include "stellaris/driverlib/sysctl.h"
#include "stellaris/driverlib/adc.h"
#include "stellaris/driverlib/gpio.h"
#include "stellaris/driverlib/interrupt.h"

#include <common.h>

#include <dcon/dcon_dev.h>
#include "7017.h"
#include "dcon_common.h"

#define MODULE_7017_ADDR 0x02
#define MODULE_7017_TYPE 0x0A
#define MODULE_7017_NAME "7017"

/* Forward declarations */
static unsigned long get_ADC_data(void);
static void set_7017_config(const char *, char *);
static void get_7017_name(const char *, char *);
static void get_7017_config(const char *, char *);
static void set_7017_sync_data(const char *, char *);
static void get_7017_sync_data(const char *, char *);
static void get_7017_adc_data(const char *, char *);
static void get_7017_adc_channel_data(const char *, char *);

static const struct cmd_t cmds[] = {
	{"%AANN40CCFF", "056", get_7017_config},
	{"$AAM", "03", get_7017_name},
	{"$AA2", "03", get_7017_config},
	{"#**", "0", set_7017_sync_data},
	{"$AA4", "03", get_7017_sync_data},
	{"#AA", "0", get_7017_adc_data},
	{"#AAN", "03", get_7017_adc_channel_data},
};

static struct dcon_dev dev_7017 = {
	.addr = MODULE_7017_ADDR,
	.type = MODULE_7017_TYPE,
};

#define ADC_DIVIDER 170.5
#define MAX_VOLTAGE 5
#define ADC_CHANNEL_COUNT 8

static SemaphoreHandle_t adc_mutex;
static unsigned long adc_vals[ADC_CHANNEL_COUNT];
static unsigned long sync_adc_vals[ADC_CHANNEL_COUNT];

static void set_7017_config(const char *request, char *response) {
	char char_addr[3];

	strncpy(char_addr, request + 5, ARRAY_SIZE(char_addr) - 1);
	char_addr[2] = '\0';

	dev_7017.addr = hex_to_int(char_addr);
	
	snprintf(response, DCON_MAX_BUF, "!\r");
}

static void get_7017_name(const char *request, char *response) {
	snprintf(response, DCON_MAX_BUF, "!%02x%s\r", dev_7017.addr,
			 MODULE_7017_NAME);
}

static void get_7017_config(const char *request, char *response) {
	snprintf(response, DCON_MAX_BUF, "!%02x%02d0000\r", dev_7017.addr,
			 dev_7017.type);	
}

static void set_7017_sync_data(const char *request, char *response) {
	xSemaphoreTake(adc_mutex, portMAX_DELAY);

	for (size_t i = 0; i < ADC_CHANNEL_COUNT; i++) {
		sync_adc_vals[i] = adc_vals[i];
	}

	xSemaphoreGive(adc_mutex);
	
	snprintf(response, DCON_MAX_BUF, "\r");
}

static void get_7017_sync_data(const char *request, char *response) {
	snprintf(response, DCON_MAX_BUF,
			 "!S%+2.4f%+2.4f%+2.4f%+2.4f%+2.4f%+2.4f\r",
			 sync_adc_vals[0] / ADC_DIVIDER, sync_adc_vals[1] / ADC_DIVIDER,
			 sync_adc_vals[2] / ADC_DIVIDER, sync_adc_vals[3] / ADC_DIVIDER,
			 sync_adc_vals[4] / ADC_DIVIDER, sync_adc_vals[5] / ADC_DIVIDER);
}

static void get_7017_adc_data(const char *request, char *response) {
	char text[DCON_MAX_BUF];
	int char_count = 0;
	
	snprintf(response, DCON_MAX_BUF,
			 ">%+2.4f%+2.4f%+2.4f%+2.4f%+2.4f%+2.4f\r",
			 adc_vals[0] / ADC_DIVIDER, adc_vals[1] / ADC_DIVIDER,
			 adc_vals[2] / ADC_DIVIDER, adc_vals[3] / ADC_DIVIDER,
			 adc_vals[4] / ADC_DIVIDER, adc_vals[5] / ADC_DIVIDER);
}
static void get_7017_adc_channel_data(const char *request, char *response) {
	snprintf(response, DCON_MAX_BUF, ">%+2.4f\r", adc_vals[request[3]]);
}

void ADCIntHandler(void) {
	unsigned long adc_vals[ADC_CHANNEL_COUNT];
	long sample_count = 0;

	ADCIntClear(ADC0_BASE, 0);

	xSemaphoreTakeFromISR(adc_mutex, portMAX_DELAY);
	sample_count = ADCSequenceDataGet(ADC0_BASE, 0, &(adc_vals[0]));
	xSemaphoreGive(adc_mutex);
}

static void init_ADC(void) {
	unsigned ulDummy = 0;

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
	
	ADCSequenceDataGet(ADC0_BASE, 0, ulDummy);

	ADCIntEnable(ADC0_BASE, 0);
	IntEnable(INT_ADC0);
}


void Task7017Function(void *pvParameters){
	struct msg msg;
	int result;

	init_ADC();

	dcon_dev_register(&dev_7017);

	for (;;) {
		dcon_dev_recv(&dev_7017, &msg);

		result = parse_command(msg.request, cmds, ARRAY_SIZE(cmds));
		if (result != -1) {
			cmds[result].handler(msg.request, msg.response);
			msg.status = OK;
		} else {
			snprintf(msg.response, DCON_MAX_BUF, "?%02x\r", dev_7017.addr);
			msg.status = FAIL;
		}

		dcon_dev_send(&dev_7017, &msg);
	}
}
