#include <string.h>

#include "freertos/FreeRTOS.h"
#include "freertos/semphr.h"

#include "common.h"

#include "iom/iom.h"
#include "iom/iom_data.h"

xSemaphoreHandle iom_mutex;
static struct iom_dev* iom_root = NULL;

int iom_dev_register(struct iom_dev *dev) {
	xSemaphoreTake(iom_mutex, portMAX_DELAY);

	dev->mutex = xSemaphoreCreateMutex();
	if (dev->mutex == NULL) {
		xSemaphoreGive(iom_mutex);
		return -1;
	}
			
	dev->dev_q = xQueueCreate(1, sizeof(struct msg));
	dev->data_q = xQueueCreate(1, sizeof(struct msg));
	if (dev->dev_q == 0 || dev->data_q == 0) {
		xSemaphoreGive(iom_mutex);
		return -1;
	}

	dev->prev = NULL;
	dev->next = iom_root;
	if (iom_root != NULL)
		iom_root->prev = dev;
	iom_root = dev;
  
	xSemaphoreGive(iom_mutex);
	return 0;
}

void iom_dev_unregister(struct iom_dev *dev) {
	
	xSemaphoreTake(iom_mutex, portMAX_DELAY);
	xSemaphoreTake(dev->mutex, portMAX_DELAY);

	dev->prev->next = dev->next;
	dev->next->prev = dev->prev;
	
	xSemaphoreGive(dev->mutex);
	xSemaphoreGive(iom_mutex);
}

void iom_dev_recv(struct iom_dev *dev, struct msg *msg) {
	xQueueReceive(dev->data_q, msg, portMAX_DELAY);
}

void iom_dev_send(struct iom_dev *dev, const struct msg *msgbuf) {
	xQueueSend(dev->dev_q, msgbuf, portMAX_DELAY);
}

struct iom_dev* iom_data_open(const char* name) {

	xSemaphoreTake(iom_mutex, portMAX_DELAY);

	for (struct iom_dev *p = iom_root; p != NULL; p = p->next) {	
		if (!strcmp(name, p->name)) {
			xSemaphoreTake(p->mutex, portMAX_DELAY);
			xSemaphoreGive(iom_mutex);
			return p;
		}
	}

	
	xSemaphoreGive(iom_mutex);
	return NULL;
}

void iom_data_close(struct iom_dev *dev) {
	
	if (xSemaphoreGetMutexHolder(dev->mutex) != NULL) {
		xSemaphoreGive(dev->mutex);
	}
}

static void iom_call_dev(struct iom_dev *dev, struct msg *msg ) {
	if (xSemaphoreGetMutexHolder(dev->mutex) == NULL) {
		msg->status = FAIL;
		return;
	}	

	if (xQueueSend(dev->data_q, msg, portMAX_DELAY) != pdTRUE) {
		msg->status = FAIL;
		return;
	}

	if (xQueueReceive(dev->dev_q, msg, portMAX_DELAY) != pdTRUE) {
		msg->status = FAIL;
		return;
	}	
}

int iom_data_read(struct iom_dev *dev, const char *request[],
					  size_t request_len, const char **response) {
	struct msg msg;
	msg.destination = FROM_DEV;
	msg.request = request;
	msg.request_len = request_len;

	iom_call_dev(dev, &msg);
	
	if (msg.status != OK) {
		return -1;
	}

	*response = msg.response;
			
	return 0;
}

int iom_data_write(struct iom_dev *dev, const char *request[],
				   size_t request_len, const char *data) {
	struct msg msg;
	msg.destination = TO_DEV;
	msg.request = request;
	msg.request_len = request_len;
	msg.data = data;

	iom_call_dev(dev, &msg);

	if (msg.status != OK) {
		return -1;	
	}

	return 0;	
}
