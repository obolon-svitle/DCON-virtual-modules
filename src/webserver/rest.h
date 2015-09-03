#ifndef _REST_H_
#define _REST_H_

#include "FreeRTOS.h"
#include "semphr.h"
#include "restfs.h"

struct rest_dev;

int rest_dev_register(struct rest_dev *dev, const char* name,
					  int (*handler) (
						  const char *param,
						  const char** data),
					  const char **dev_params,
					  int param_count);
void rest_dev_unregister(struct rest_dev *dev);

#endif
