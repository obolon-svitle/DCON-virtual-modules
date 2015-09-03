#include "FreeRTOS.h"
#include "semphr.h"
#include "restfs.h"

#include <string.h>

static int generic_html_handler(const char* param, const char** data);
static struct rest_dev dev_not_found_htm;
static struct rest_dev dev_index_htm;

static struct rest_dev dev_index_htm = {
	.next = &dev_not_found_htm,
	.prev = NULL,
	.name = "index.htm",
	.param_count = 0,
	.handler = generic_html_handler,
};

static struct rest_dev dev_not_found_htm = {
	.next = NULL,
	.prev = &dev_index_htm,
	.name = "404",
	.param_count = 0,
	.handler = generic_html_handler,
};

static char data_index[] = {"<html><head></head><body>restful-daq server</body></html>"};
static char data_404[] = {"<html><head></head><body>404 Not Found</body></html>"};

static int generic_html_handler(const char* param, const char** data) {
	if (!strcmp(param, dev_index_htm.name)) {
		*data = data_index;
		return sizeof(data_index) / sizeof(data_index[0]);
	}	
	*data = data_404;
	return sizeof(data_404) / sizeof(data_404[0]);
}

struct rest_dev *rest_root = &dev_index_htm;
