#include "rest.h"
#include <stdlib.h>
#include <string.h>

extern xSemaphoreHandle rest_root_mutex;
extern struct rest_dev *rest_root;

/* Register and unregister devices. A thread can only control 
 * the devices, which it registers by itself
 */

int rest_dev_register(struct rest_dev *dev) {
	#if !NO_SYS
	if ((dev->semphr = xSemaphoreCreateMutex()) == NULL)
		return -1;
	xSemaphoreTake(rest_root_mutex, portMAX_DELAY);
	#endif
    dev->next = rest_root;
	dev->prev = rest_root->prev;
	rest_root->prev = dev;
	rest_root = dev;
	#if !NO_SYS
	xSemaphoreGive(rest_root_mutex);
	#endif
	return 0;
}

void rest_dev_unregister(struct rest_dev *dev) {
	#if !NO_SYS
	xSemaphoreTake(rest_root_mutex, portMAX_DELAY);
	xSemaphoreTake(dev->semphr, portMAX_DELAY);
	#endif
	dev->prev->next = dev->next;
	dev->next->prev = dev->prev;
	#if !NO_SYS
	xSemaphoreGive(dev->semphr);
	xSemaphoreGive(rest_root_mutex);
	#endif
}
