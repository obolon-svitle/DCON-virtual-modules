#ifndef _IOM_DATA_H_
#define _IOM_DATA_H_

#include <stddef.h>

struct iom_dev;

int iom_init();
struct iom_dev* iom_data_open(const char *devname);
void iom_data_close(struct iom_dev *dev);
int iom_data_read(struct iom_dev *dev, const char *request[],
					  size_t request_len, const char **response);
int iom_data_write(struct iom_dev *dev, const char *request[],
				   size_t request_len, const char *data);

#endif /* _IOM_DATA_H_ */
