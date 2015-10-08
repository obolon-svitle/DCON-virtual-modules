#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#include "inc/hw_types.h"
#include "inc/hw_memmap.h"
#include "driverlib/gpio.h"
#include "utils/uartstdio.h"

#include "FreeRTOS.h"
#include "task.h"
#include "semphr.h"

#include "common.h"

#include "iom/iom.h"

static long int ports[] = {
	GPIO_PORTA_BASE,
	GPIO_PORTB_BASE,
	GPIO_PORTC_BASE,
	GPIO_PORTD_BASE,
	GPIO_PORTE_BASE,
	GPIO_PORTF_BASE,
};

static unsigned char pins[] = {
	GPIO_PIN_0,
	GPIO_PIN_1,
	GPIO_PIN_2,
	GPIO_PIN_3,
	GPIO_PIN_4,
	GPIO_PIN_5,
	GPIO_PIN_6,
	GPIO_PIN_7,
};

/* Forward declarations */
static const char* pin_type_read(const char **request, size_t len);
static int pin_data_write(const char **request, size_t request_len,
						  const char *data);

static const char* pin_data_read(const char **request, size_t len);
static int pin_type_write(const char **request, size_t request_len,
						  const char *data);

#define PINDATA_BUFSIZE 3
static char pindata[PINDATA_BUFSIZE];

static const char *write_func_names[] =
{"pin_type", "pin_data"};
static const char *read_func_names[] =
{"pin_type", "pin_data"};

static const char *(* const read_funcs[])(const char **, size_t) =
{pin_type_read, pin_data_read};
static int (* const write_funcs[])(const char **, size_t, const char*) =
{pin_type_write, pin_data_write};

static struct iom_dev gpio = {
	.name = "gpio",
};

static int parse_pin(const char *portstr, const char *pinstr,
					 long int *port, unsigned char *pin) {
	int pinstrlen;
	
	if (portstr[0] >= 'a' && portstr[0] <= 'f') {
		*port = ports[portstr[0] - 'a'];
	} else if (portstr[0] >= 'A' && portstr[0] <= 'F') {
		*port = ports[portstr[0] - 'A'];
	} else {
		return FAIL;
	}

	pinstrlen = strlen(pinstr);
	*pin = 0;
	
	for (size_t i = 0; i < pinstrlen; i += 2) {
		if ((*pinstr - '0' < 0) || (*pinstr - '0') > 7)
			return FAIL;
		*pin |= pins[pinstr[i] - '0'];

		if (pinstr[i + 1] != '|' && pinstr[i + 1] != '\0')
			return FAIL;
	}

	return 0;
}

static const char *pin_type_read(const char **request, size_t len) {
	unsigned char pin;
	long int port;
	long int mode;

	/* Port name + Pin name */
	if (len != 2) {
		return 0;
	}

	if (parse_pin(request[0], request[1], &port, &pin)) {
		return 0;
	}

	mode = GPIODirModeGet(port, pin);
	if (mode == GPIO_DIR_MODE_OUT)
		return "out";
	else if (mode == GPIO_DIR_MODE_IN)
		return "in";
	else if (mode == GPIO_DIR_MODE_HW)
		return "hw";
	else
		return 0;
}

static int pin_type_write(const char **request, size_t request_len,
						  const char *data) {
	unsigned char pin;
	long int port;
	long int mode;

	/* Port + Pin + mode */
	if (request_len != 2)
		return -1;

	if (parse_pin(request[0], request[1], &port, &pin)) {
		return -1;
	}

	if (!strcmp(data, "in")) {
			mode = GPIO_DIR_MODE_IN;			
	} else if (!strcmp(data, "out")) {
		mode = GPIO_DIR_MODE_OUT;
	} else if (!strcmp(data, "hw")) {
		mode = GPIO_DIR_MODE_HW;
	} else {
		return -1;
	}

	GPIODirModeSet(port, pin, mode);
	return OK;
}

static int pin_data_write(const char **request, size_t request_len,
						  const char *data) {
	unsigned char pin;
	long int port;
	unsigned char val;

	if (request_len < 2)
		return -1;

	if (!parse_pin(request[0], request[1], &port, &pin)) {
		return FAIL;
	}

	val = atoi(data);
	GPIOPinWrite(port, pin, val);
	return OK;	
}

static const char* pin_data_read(const char **request, size_t len) {
	unsigned char pin;
	long int port;
	char val;

	if (len < 2)
		return NULL;

	if (!parse_pin(request[0], request[1], &port, &pin)) {
		return NULL;;
	}

	val = GPIOPinRead(port, pin);
	snprintf(pindata, ARRAY_SIZE(pindata), "%d", val);
	return pindata;
}

void TaskGPIOFunction(void *pvParameters) {
	struct msg msg;
	
	iom_dev_register(&gpio);

	for (;;) {
		iom_dev_recv(&gpio, &msg);

		if (msg.destination == FROM_DEV && msg.request_len > 0) {
			for (size_t i = 0; i < ARRAY_SIZE(read_func_names); i++) {
				if (!strcmp(read_func_names[i], msg.request[0])) {
					msg.response = read_funcs[i](msg.request + 1,
												 msg.request_len - 1);
					if (msg.response == NULL) {
						msg.status = FAIL;
					} else {
						msg.status = OK;
					}
				}
			}
		} else if (msg.destination == TO_DEV) {
			for (size_t i = 0; i < ARRAY_SIZE(write_func_names); i++) {
				if (!strcmp(write_func_names[i], msg.request[0])) {
					msg.status = write_funcs[i](msg.request + 1,
												msg.request_len - 1,
												msg.data);
				}
			}			
		}

		iom_dev_send(&gpio, &msg);
	}
}
