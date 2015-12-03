#ifndef _DCON_DEV_H_
#define _DCON_DEV_H_

#include <stddef.h>

#include "freertos/FreeRTOS.h"
#include "freertos/semphr.h"

const int DCON_MAX_BUF = 30;

enum status {
	OK,
	FAIL,
};

struct msg {
	const char *request;
	const char *response;
	enum status status;
};

struct dcon_dev {
	char *addr;
	char *type;
	xSemaphoreHandle mutex;
	xQueueHandle dev_q;
	xQueueHandle data_q;
	struct dcon_dev *next;
	struct dcon_dev *prev;
};

int dcon_dev_register(struct dcon_dev *dev);
void dcon_dev_unregister(struct dcon_dev *dev);
void dcon_dev_recv(struct dcon_dev *dev, struct msg *msg);
void dcon_dev_send(struct dcon_dev *dev, const struct msg *msg);

#endif /* _DCON_H_ */
