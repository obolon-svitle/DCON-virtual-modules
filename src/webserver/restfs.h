#ifndef _RESTFS_H_
#define _RESTFS_H_

#include "FreeRTOS.h"
#include "semphr.h"

typedef int (*const params_handler_t)(const char *param,
									  const char** data); 
struct rest_dev {
	struct rest_dev *next;
	struct rest_dev *prev;
	xSemaphoreHandle semphr;
	const char* name;
	params_handler_t handler;
	const char **dev_params;
	int param_count;
};

extern struct rest_dev *rest_root;
extern xSemaphoreHandle rest_root_semphr;

#endif
