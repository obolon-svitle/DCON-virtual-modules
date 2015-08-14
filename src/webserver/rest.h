#ifndef _REST_H_
#define _REST_H_

#include <FreeRTOS.h>
#include <semphr.h>

struct rest_dev {
	xSemaphoreHandle semphr;
	struct rest_dev *next;
	struct rest_dev *prev;
	const char* name;
	int (*param_func)(const char *param,
					  char **data);
	const char **dev_params;
	const int param_count;
};

int rest_dev_register(struct rest_dev *dev);
void rest_dev_unregister(struct rest_dev *dev);

#endif
