#include <string.h>

#include "freertos/FreeRTOS.h"
#include "freertos/semphr.h"

#include "common.h"

#include "dcon_common.h"
#include "dcon/dcon_dev.h"
#include "dcon/dcon_data.h"

xSemaphoreHandle dcon_root_mutex;
static struct dcon_dev* dcon_root = NULL;

int dcon_dev_register(struct dcon_dev *dev) {
	xSemaphoreTake(dcon_root_mutex, portMAX_DELAY);

	if (dev->mutex == NULL) {
		xSemaphoreGive(dcon_root_mutex);
		return -1;
	}

	dev->mutex = xSemaphoreCreateMutex();
			
	dev->dev_q = xQueueCreate(1, sizeof(struct msg));
	dev->data_q = xQueueCreate(1, sizeof(struct msg));
	if (dev->dev_q == 0 || dev->data_q == 0) {
		xSemaphoreGive(dcon_root_mutex);
		return -1;
	}

	dev->prev = NULL;
	dev->next = dcon_root;
	if (dcon_root != NULL)
		dcon_root->prev = dev;
	dcon_root = dev;
  
	xSemaphoreGive(dcon_root_mutex);
	return 0;
}

void dcon_dev_unregister(struct dcon_dev *dev) {	
	xSemaphoreTake(dcon_root_mutex, portMAX_DELAY);
	xSemaphoreTake(dev->mutex, portMAX_DELAY);

	dev->prev->next = dev->next;
	dev->next->prev = dev->prev;

	xSemaphoreGive(dev->mutex);
	xSemaphoreGive(dcon_root_mutex);
}

void dcon_dev_recv(struct dcon_dev *dev, struct msg *msg) {
	xQueueReceive(dev->data_q, msg, portMAX_DELAY);
}

void dcon_dev_send(struct dcon_dev *dev, const struct msg *msgbuf) {
	xQueueSend(dev->dev_q, msgbuf, portMAX_DELAY);
}

static struct dcon_dev* dcon_find(const char *request) {
	if (strlen(request) < 3)
		return NULL;
	
	xSemaphoreTake(dcon_root_mutex, portMAX_DELAY);

	for (struct dcon_dev *p = dcon_root; p != NULL; p = p->next) {
		xSemaphoreTake(p->mutex, portMAX_DELAY);
		if (!strncmp(p->addr + 1, request, 2)) {
			xSemaphoreGive(dcon_root_mutex);
			return p;
		}
		xSemaphoreGive(p->mutex);
	}

	xSemaphoreGive(dcon_root_mutex);

	return NULL;
}

static void dcon_close(struct dcon_dev *dev) {
	xSemaphoreGive(dev->mutex);
}

static void dcon_call_dev(struct dcon_dev *dev, struct msg *msg) {
	if (xQueueSend(dev->data_q, msg, portMAX_DELAY) != pdTRUE) {
		msg->status = FAIL;
		return;
	}

	if (xQueueReceive(dev->dev_q, msg, portMAX_DELAY) != pdTRUE) {
		msg->status = FAIL;
		return;
	}	
}

static void dcon_call_all(struct msg *msg) {
	xSemaphoreTake(dcon_root_mutex, portMAX_DELAY);
	
	for (struct dcon_dev *p = dcon_root; p != NULL; p = p->next) {
		dcon_call_dev(p, msg);
	}
	
	xSemaphoreGive(dcon_root_mutex);
}

int dcon_data_send(const char *request, char *response) {
	struct msg msg;
	struct dcon_dev *dev;

	msg.request = request;
	msg.response = response;

	if (!strcmp(request, CMD_SYNC_SAMPLING) ||
		!strcmp(request, CMD_HOST_OK)) {
		
		dcon_call_all(&msg);
		return 0;
	}
	
	if ((dev = dcon_find(request)) != NULL) {
		dcon_call_dev(dev, &msg);
		if (msg.status == OK) {
			dcon_close(dev);
			return 0;
		}
		dcon_close(dev);
	}	

	return -1;
}
