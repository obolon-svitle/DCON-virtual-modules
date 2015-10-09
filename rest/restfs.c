#include "freertos/FreeRTOS.h"
#include "freertos/semphr.h"
#include "restfs.h"

#include <string.h>
#include "rest.h"

static struct rest_dev dev_404_htm;
static struct rest_dev dev_index_htm;

static char data_index[] = {"<html><head></head><body>restful-daq server</body></html>"};
static char data_404[] = {"<html><head></head><body>404 Not Found</body></html>"};

static struct rest_param dev_index_param[] = {
	{
		.name = "",
		.data = data_index,
	},
};

static struct rest_param dev_404_param[] = {
	{
		.name = "",
		.data = data_404,
	},
};

static struct rest_dev dev_index_htm = {
	.next = &dev_404_htm,
	.prev = NULL,
	.name = "index.htm",
	.param_count = 1,
	.dev_params = dev_index_param,
};

static struct rest_dev dev_404_htm = {
	.next = NULL,
	.prev = &dev_index_htm,
	.name = "404.html",
	.param_count = 1,
	.dev_params = dev_404_param,
};

struct rest_dev *rest_root = &dev_index_htm;

int rest_dev_register(struct rest_dev *dev) {
	dev->mutex = xSemaphoreCreateMutex();	
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

int rest_lookup(const char *url, char **data, xSemaphoreHandle *datamutex) {
	struct rest_dev *d;
	char* slash_ptr;
	int i, param_pos;

	slash_ptr = strchr(url + 1, '/');
	if (slash_ptr == NULL) {
		param_pos = strlen(url);
	} else {
		param_pos = slash_ptr - url;
	}
	
	for (d = rest_root; d != NULL; d = d->next) {
		if (!strncmp(url + 1, d->name, param_pos - 1)) {
			for (i = 0; i < d->param_count; i++) {
				if (!strcmp(url + param_pos + 1, d->dev_params[i].name)) {
					*datamutex = d->mutex;
					*data = d->dev_params[i].data;
					return strlen(d->dev_params[i].data);
				}
			}
		}
	}
	
	return -1;
}

void rest_close(char *url) {
	
}
