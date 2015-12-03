#ifndef _DCON_DATA_H_
#define _DCON_DATA_H_

#include <stddef.h>

struct dcon_dev;

extern const int BUFSIZE;

int dcon_init();

int dcon_data_send(const char *request, char *response);

#endif /* _DCON_DATA_H_ */
