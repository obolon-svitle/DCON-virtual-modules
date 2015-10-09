#ifndef _RESTFS_H_
#define _RESTFS_H_

int rest_lookup(const char *url, char **data, xSemaphoreHandle *mutex);
void rest_close(char *url);

#endif
