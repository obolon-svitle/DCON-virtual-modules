#include <stdlib.h>
#include <string.h>
#include "restfs.h"

/* Register and unregister devices. A thread can only control 
 * the devices, which it registers by itself
 */

int rest_dev_register(struct rest_dev *dev, const char* name,
					  int (*handler) (
						  const char *param,
						  const char** data),
					  const char **dev_params,
					  int param_count) {
    dev->next = rest_root;
	dev->prev = rest_root->prev;
	rest_root->prev = dev;
	rest_root = dev;
	return 0;
}

void rest_dev_unregister(struct rest_dev *dev) {
	dev->prev->next = dev->next;
	dev->next->prev = dev->prev;
}
