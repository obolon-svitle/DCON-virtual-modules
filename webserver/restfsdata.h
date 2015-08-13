#ifndef _RESTFSDATA_H_
#define _RESTFSDATA_H_

#include <FreeRTOS.h>
#include <semphr.h>

xSemaphoreHandle rest_root_mutex;
struct rest_dev *rest_root_dev;

#endif
