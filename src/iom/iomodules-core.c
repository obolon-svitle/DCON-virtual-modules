#include <string.h>

#include "FreeRTOS.h"
#include "semphr.h"

#include "common.h"

#include "iom/iom.h"
#include "iom/iom_data.h"

#define IOM_TABLE_SIZE 30
static struct iom_dev* iom_table[IOM_TABLE_SIZE];
static xSemaphoreHandle iom_dev_mutex;

void iom_init() {
	memset(iom_table, sizeof(iom_table), 0);
	iom_dev_mutex = xSemaphoreCreateMutex();
}

int iom_dev_register(struct iom_dev *dev) {
	
	xSemaphoreTake(iom_dev_mutex, portMAX_DELAY);

	for (int i = 0; i < ARRAY_SIZE(iom_table); i++) {
		if (iom_table[i] == NULL) {
			dev->mutex = xSemaphoreCreateMutex();
			dev->dev_q = xQueueCreate(1, sizeof(struct msg));
			dev->data_q = xQueueCreate(1, sizeof(struct msg));
			dev->id = i;
			iom_table[i] = dev;
			xSemaphoreGive(iom_dev_mutex);
			return 0;
		}
	}
	
	xSemaphoreGive(iom_dev_mutex);
	return -1;
}

void iom_dev_unregister(struct iom_dev *dev) {
	
	xSemaphoreTake(iom_dev_mutex, portMAX_DELAY);
	xSemaphoreTake(dev->mutex, portMAX_DELAY);
	
	iom_table[dev->id] = NULL;
	
	xSemaphoreGive(dev->mutex);
	xSemaphoreGive(iom_dev_mutex);
}

void iom_dev_recvaction(struct iom_dev *dev, struct msg *msgbuf) {
	xQueueReceive(dev->data_q, msgbuf, portMAX_DELAY);
}

void iom_dev_sendaction(struct iom_dev *dev, const struct msg *msgbuf) {
	xQueueSend(dev->dev_q, msgbuf, portMAX_DELAY);
}

struct iom_dev* iom_data_open(const char* name) {

	xSemaphoreTake(iom_dev_mutex, portMAX_DELAY);
	
	for (int i = 0; i < ARRAY_SIZE(iom_table); i++) {
		if (!strcmp(name, iom_table[i]->name)) {
			xSemaphoreTake(iom_table[i]->mutex, portMAX_DELAY);
			xSemaphoreGive(iom_dev_mutex);
			return iom_table[i];
		}
	}

	xSemaphoreGive(iom_dev_mutex);
	return NULL;
}

void iom_data_close(struct iom_dev *dev) {
	
	if (xSemaphoreGetMutexHolder(dev->mutex) != NULL) {
		xSemaphoreGive(dev->mutex);
	}
}

int iom_data_to_dev(struct iom_dev *dev, const char *param, char *data) {

	struct msg msgbuf;
	
	if (xSemaphoreGetMutexHolder(dev->mutex) == NULL)
		return -1;

	msgbuf.param = param;
	msgbuf.data = data;
	msgbuf.type = WRITE;
			
	xQueueSend(dev->data_q, &msgbuf, portMAX_DELAY);
	xQueueReceive(dev->dev_q, &msgbuf, portMAX_DELAY);
			
	return msgbuf.status;
}

int iom_data_from_dev(struct iom_dev *dev, const char *param, char **data) {

	struct msg msgbuf;

	if (xSemaphoreGetMutexHolder(dev->mutex) == NULL) {
		return -1;
	}

	msgbuf.data = *data;
	msgbuf.param = param;
	msgbuf.type = READ;
		
	xQueueSend(dev->data_q, &msgbuf, portMAX_DELAY);
	xQueueReceive(dev->dev_q, &msgbuf, portMAX_DELAY);
		
	return msgbuf.status;
}
