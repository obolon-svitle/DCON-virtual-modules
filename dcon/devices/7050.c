#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "common.h"
#include "dcon/dcon_dev.h"
#include "7050.h"

typedef void (*c_handler_t)(char *command, char *response);

static const struct dcon_dev dev_7050 = {
	.addr = MODULE_7050_ADDR,
	.type = MODULE_7050_TYPE,
};

/*
set_7050_config;
get_7050_digital_output;
get_7050_name;
get_7050_config;
get_7050_sync_data;
get_7050_digital_io_status;
*/

static void exec_command(char *command, char *response) {
	
}

void Task7050Function(void *pvParameters) {
	struct msg msg;
	int result;

	dcon_dev_register(&dev_7050);

	for (;;) {
		dcon_dev_recv(&dev_7050, &msg);
		
		exec_command(msg.request, msg.response);
		
		dcon_dev_send(&dev_7050, &msg);
	}
}
