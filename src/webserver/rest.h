#ifndef _REST_H_
#define _REST_H_

#include <FreeRTOS.h>
#include <semphr.h>

typedef int (*const params_handler_t)(const char *param,
									  const char** data); 

struct rest_dev {
	struct rest_dev *next;
	struct rest_dev *prev;
	xSemaphoreHandle semphr;
	const char* name;
	params_handler_t handler;
	const int param_count;
	const char **dev_params;
};

int rest_dev_register(struct rest_dev *dev);
void rest_dev_unregister(struct rest_dev *dev);

#endif
