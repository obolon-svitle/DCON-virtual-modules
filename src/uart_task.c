#include <inc/hw_types.h>
#include <utils/uartstdio.h>
#include "FreeRTOS.h"
#include "task.h"

void TaskUART(void *pvParameters) {
	for (;;) {
		UARTprintf("YOBA\n");
		vTaskDelay(500 / portTICK_RATE_MS);
	}
}
