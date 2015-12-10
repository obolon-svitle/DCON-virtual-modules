#ifndef _DCON_DATA_H_
#define _DCON_DATA_H_

#include <stddef.h>

struct dcon_dev;

extern const int DCON_MAX_BUF;

int dcon_init();

void dcon_data_send(const char *request, char *response);

#endif /* _DCON_DATA_H_ */
