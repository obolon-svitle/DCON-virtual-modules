#ifndef _DCON_DEV_H_
#define _DCON_DEV_H_

#include <stddef.h>

#include "freertos/FreeRTOS.h"
#include "freertos/semphr.h"

extern const int DCON_MAX_BUF;

enum status {
	OK,
	FAIL,
};

struct msg {
	const char *request;
	char *response;
	enum status status;
};

struct dcon_dev {
	unsigned int addr;
	unsigned int type;
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
