#ifndef _IOM_DATA_H_
#define _IOM_DATA_H_

#include <stddef.h>

struct iom_dev;

void iom_init();
struct iom_dev* iom_data_open(const char *devname);
void iom_data_close(struct iom_dev *dev);
int iom_data_to_dev(struct iom_dev *dev, const char *param, char *data);
int iom_data_from_dev(struct iom_dev *dev, const char *param, char **data);

#endif /* _IOM_DATA_H_ */
