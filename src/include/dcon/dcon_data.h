#ifndef _DCON_DATA_H_
#define _DCON_DATA_H_

#include "dcon_types.h"
void dcon_list_devices(void (*action)(int addr, int type,
                                      void *data), void *data);
void dcon_data_send(const char *request, char *response);

#endif /* _DCON_DATA_H_ */
