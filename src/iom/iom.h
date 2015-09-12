#ifndef _IOM_H_
#define _IOM_H_

#include <stddef.h>

#include "FreeRTOS.h"
#include "semphr.h"

enum msg_type {
	READ,
	WRITE,
};

struct msg {
	const char *param;
	char *data;
	enum msg_type type;
	int status;
};

struct iom_dev {
	const char *name;
	const char **parameters;
	const int param_count;
	xSemaphoreHandle mutex;
	xQueueHandle dev_q;
	xQueueHandle data_q;
	int id;
};

int iom_dev_register(struct iom_dev *dev);
void iom_dev_unregister(struct iom_dev *dev);
void iom_dev_recvaction(struct iom_dev *dev, struct msg *msgbuf);
void iom_dev_sendaction(struct iom_dev *dev, const struct msg *msgbuf);

#endif /* _IOM_H_ */
