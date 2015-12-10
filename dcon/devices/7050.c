#include <stdio.h>
#include <stdarg.h>
#include <stdlib.h>
#include <string.h>

#include "stellaris/hw_types.h"
#include "stellaris/hw_memmap.h"
#include "stellaris/driverlib/gpio.h"

#include "common.h"
#include "dcon/dcon_dev.h"
#include "7050.h"

#define DIGITAL_GROUP GPIO_PORTA_BASE

/* Forward declarations */
static void set_7050_config(const char *, char *);
static void set_7050_digital_output(const char *, char *);
static void get_7050_name(const char *, char *);
static void get_7050_config(const char *, char *);
static void set_7050_sync_data(const char *, char *);
static void get_7050_sync_data(const char *, char *);
static void get_7050_digital_io_status(const char *, char *);
static void get_7050_digital_io(const char *, char *);

typedef void (*cmd_handler_t)(const char *cmd, char *response);

struct cmd_t {
	const char *pattern;
	const char *check_bytes;
	cmd_handler_t handler;
};

static const struct cmd_t cmds[] = {
	{"%AANN40CCFF", "056", get_7050_config},
	{"#AABBDD", "0", set_7050_digital_output},
	{"$AAM", "03", get_7050_name},
	{"$AA2", "03", get_7050_config},
	{"#**", "0", set_7050_sync_data},
	{"$AA4", "03", get_7050_sync_data},
	{"$AA6", "03", get_7050_digital_io_status},
	{"@AA", "0", get_7050_digital_io},
};

typedef void (*c_handler_t)(char *command, char *response);

static struct dcon_dev dev_7050 = {
	.addr = MODULE_7050_ADDR,
	.type = MODULE_7050_TYPE,
};

static long int synchronized_data = 0;

static int parse_command(const char *cmd) {
	size_t j, len, check_byte;
	for (size_t i = 0; i < ARRAY_SIZE(cmds); i++) {

		if (strlen(cmds[i].pattern) != strlen(cmd)) {
			continue;
		}

		len = strlen(cmds[i].check_bytes);
		for (j = 0; j < len; j++) {
			check_byte = cmds[i].check_bytes[j] - '0';
			if (cmds[i].pattern[check_byte] !=
				cmd[check_byte])
				break;
		}

		if (j == len) {
			return i;
		} else {
			continue;
		}
	}
	
	return -1;
}

/* The str MUST have at least 2 symbols long */
static unsigned int hex_to_int(const char *str) {
	char addr_str[3];
	memcpy(addr_str, str, 2);
	addr_str[2] = '\0';

	return strtol(addr_str, NULL, 16);
}

static void set_7050_config(const char *request, char *response) {
	char char_addr[3];

	strncpy(char_addr, request + 5, ARRAY_SIZE(char_addr) - 1);
	char_addr[2] = '\0';

	dev_7050.addr = hex_to_int(char_addr);
	
	snprintf(response, DCON_MAX_BUF, "!\r");
}

static void set_7050_digital_output(const char *request, char *response) {
	unsigned char output_group = hex_to_int(request + 3);
	unsigned char output_value = hex_to_int(request + 5);

	if (output_group == 0x00 || output_group == 0x0A) {
		GPIOPinWrite(DIGITAL_GROUP, 0xff, output_value);
		snprintf(response, DCON_MAX_BUF, ">\r");
	} else {
		snprintf(response, DCON_MAX_BUF, "?\r");
	}

	return;
}

static void get_7050_name(const char *request, char *response) {
	snprintf(response, DCON_MAX_BUF, "!%02x%s\r", dev_7050.addr,
			 MODULE_7050_NAME);
}

static void get_7050_config(const char *request, char *response) {
	snprintf(response, DCON_MAX_BUF, "!%02x%02d0000\r", dev_7050.addr,
			 dev_7050.type);	
}

static void set_7050_sync_data(const char *request, char *response) {
	synchronized_data = GPIOPinRead(DIGITAL_GROUP, 0xff);
	snprintf(response, DCON_MAX_BUF, "\r");
}

static void get_7050_sync_data(const char *request, char *response) {
	snprintf(response, DCON_MAX_BUF, "!S%02x\r", synchronized_data);
}

static void get_7050_digital_io_status(const char *request, char *response) {
	long int pintype = GPIODirModeGet(DIGITAL_GROUP, 0xff);
	snprintf(response, DCON_MAX_BUF, "!%02x\r", pintype);
}

static void get_7050_digital_io(const char *request, char *response) {
	long int input;

	input = GPIOPinRead(DIGITAL_GROUP, 0xff);
	snprintf(response, DCON_MAX_BUF, ">%02x\r", input);
}


void Task7050Function(void *pvParameters) {
	struct msg msg;
	int result;

	dcon_dev_register(&dev_7050);

	for (;;) {
		dcon_dev_recv(&dev_7050, &msg);

		result = parse_command(msg.request);
		if (result != -1) {
			cmds[result].handler(msg.request, msg.response);
			msg.status = OK;
		} else {
			snprintf(msg.response, DCON_MAX_BUF, "?%02x\r", dev_7050.addr);
			msg.status = FAIL;
		}
		
		dcon_dev_send(&dev_7050, &msg);
	}
}
