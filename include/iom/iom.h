#ifndef _IOM_H_
#define _IOM_H_

#include <stddef.h>

#include "freertos/FreeRTOS.h"
#include "freertos/semphr.h"

enum status {
	OK,
	FAIL,
};

enum destination {
	TO_DEV,
	FROM_DEV,
};

struct msg {
	const char **request;
	size_t request_len;
	const char *response;
	const char *data;
	enum status status;
	enum destination destination;
};

struct iom_dev {
	const char *name;
	xSemaphoreHandle mutex;
	xQueueHandle dev_q;
	xQueueHandle data_q;
	struct iom_dev *next;
	struct iom_dev *prev;
};

int iom_dev_register(struct iom_dev *dev);
void iom_dev_unregister(struct iom_dev *dev);
void iom_dev_recv(struct iom_dev *dev, struct msg *msgbuf);
void iom_dev_send(struct iom_dev *dev, const struct msg *msgbuf);

#endif /* _IOM_H_ */
