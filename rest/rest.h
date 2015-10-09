#ifndef _REST_H_
#define _REST_H_

#include "freertos/FreeRTOS.h"
#include "freertos/semphr.h"
#include "restfs.h"

struct rest_param {
	const char *name;
	char *data;
};

struct rest_dev {
	struct rest_dev *next;
	struct rest_dev *prev;
	xSemaphoreHandle mutex;
	const char* name;
	const int param_count;
	struct rest_param *dev_params;
};

int rest_dev_register(struct rest_dev *dev);
void rest_dev_unregister(struct rest_dev *dev);

#endif
