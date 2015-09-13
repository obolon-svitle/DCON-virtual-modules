#include <stdio.h>
#include <string.h>

#include "inc/hw_types.h"
#include "utils/uartstdio.h"

#include "FreeRTOS.h"
#include "task.h"
#include "semphr.h"

#include "iom/iom.h"
#include "common.h"

#define BUFLEN 10

#define PIN_TYPE_PARAM_NAME "pin_type"
static char pin_type_buf[BUFLEN];

#define PIN_DATA_PARAM_NAME "pin_data"
static char pin_data_buf[BUFLEN];

static const char *parameters[] = {
	PIN_TYPE_PARAM_NAME,
	PIN_DATA_PARAM_NAME,
};

static struct iom_dev gpio = {
	.name = "GPIO",
	.parameters = parameters,
	.param_count = ARRAY_SIZE(parameters),		
};

static void perform_action(struct msg* msgbuf) {
	
	if (!strcmp(msgbuf->param, parameters[0])) {
		if (msgbuf->type == READ) {
			sprintf(pin_type_buf, "READ pin_type\n");
			msgbuf->data = pin_type_buf;
			msgbuf->status = 0;
		} else {
			UARTprintf("write to pin_type: %s\n", msgbuf->data);
			msgbuf->status = 0;
		}
	} else if (!strcmp(msgbuf->param, parameters[1])) {
		if (msgbuf->type == READ) {
			sprintf(pin_type_buf, "READ pin_data\n");
			msgbuf->data = pin_data_buf;
			msgbuf->status = 0;
		} else {
			UARTprintf("write to pin_type: %s\n", msgbuf->data);
			msgbuf->status = 0;
		}
	} else {
		msgbuf->status = -1;
	}
}


void TaskGPIOFunction(void *pvParameters) {

	struct msg msgbuf;
	
	iom_dev_register(&gpio);

	for (;;) {
		iom_dev_recvaction(&gpio, &msgbuf);
		perform_action(&msgbuf);
		iom_dev_sendaction(&gpio, &msgbuf);		
	}
}
