#ifndef _DCON_DATA_H_
#define _DCON_DATA_H_

#include <stddef.h>

extern const int DCON_MAX_BUF;

void dcon_data_send(const char *request, char *response);

#endif /* _DCON_DATA_H_ */
