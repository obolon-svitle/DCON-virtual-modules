#include <stdio.h>
#include <stdarg.h>
#include <stdlib.h>
#include <string.h>

#include <common.h>
#include <dcon/dcon_dev.h>
#include "dcon/dcon_dev_common.h"

#include "7050.h"
#include "7050_hw.h"

#define MODULE_7050_ADDR 0x00
#define MODULE_7050_TYPE 0x40

#define MODULE_7050_NAME "7050"

/* Forward declarations */
static void set_7050_config(const char *, char *);
static void set_7050_digital_output(const char *, char *);
static void get_7050_name(const char *, char *);
static void get_7050_config(const char *, char *);
static void set_7050_sync_data(const char *, char *);
static void get_7050_sync_data(const char *, char *);
static void get_7050_digital_io_status(const char *, char *);
static void get_7050_digital_io(const char *, char *);
static void get_7050_temperature(const char *, char *);

static const struct cmd_t cmds[] = {
    {"%AANN40CCFF", "056", set_7050_config},
    {"#AABBDD", "0", set_7050_digital_output},
    {"$AAM", "03", get_7050_name},
    {"$AA2", "03", get_7050_config},
    {"#**", "0", set_7050_sync_data},
    {"$AA4", "03", get_7050_sync_data},
    {"$AA6", "03", get_7050_digital_io_status},
    {"@AA", "0", get_7050_digital_io},
    {"@AAI", "0", get_7050_temperature},
};

static struct dcon_dev dev_7050 = {
    .addr = MODULE_7050_ADDR,
    .type = MODULE_7050_TYPE,
};

static SemaphoreHandle_t sync_mutex;
static unsigned long int synchronized_data = 0;

static inline void set_7050_config(const char *request, char *response) {
    char char_addr[3];

    strncpy(char_addr, request + 5, ARRAY_SIZE(char_addr) - 1);
    char_addr[2] = '\0';

    dev_7050.addr = hex_to_int(char_addr);
    
    snprintf(response, DCON_MAX_BUF, "!\r");
}

static inline void set_7050_digital_output(const char *request, char *response) {
    unsigned char group = hex_to_int(request + 3);
    unsigned char value = hex_to_int(request + 5);

    if (!set_output(group, value)) {
        snprintf(response, DCON_MAX_BUF, ">\r");
    } else {
        snprintf(response, DCON_MAX_BUF, "?\r");
    }
}

static inline void get_7050_name(const char *request, char *response) {
    snprintf(response, DCON_MAX_BUF, "!%02x%s\r", dev_7050.addr,
             MODULE_7050_NAME);
}

static inline void get_7050_config(const char *request, char *response) {
    snprintf(response, DCON_MAX_BUF, "!%02x%02d0000\r", dev_7050.addr,
             dev_7050.type);    
}

static inline void set_7050_sync_data(const char *request, char *response) {
	xSemaphoreTake(sync_mutex, portMAX_DELAY);
    synchronized_data = get_input();
    snprintf(response, DCON_MAX_BUF, "\r");
	xSemaphoreGive(sync_mutex);
}

static inline void get_7050_sync_data(const char *request, char *response) {
	xSemaphoreTake(sync_mutex, portMAX_DELAY);
    snprintf(response, DCON_MAX_BUF, "!S%02x\r", synchronized_data);
	xSemaphoreGive(sync_mutex);
}

static inline void get_7050_digital_io_status(const char *request, char *response) {
    unsigned long int pintype = get_io_status();
    snprintf(response, DCON_MAX_BUF, "!%02x\r", pintype);
}

static inline void get_7050_digital_io(const char *request,
                       char *response) {
    long int input;

    input = get_input();
    snprintf(response, DCON_MAX_BUF, ">%02x\r", input);
}

static inline void get_7050_temperature(const char *request,
                                        char *response) {
    int intpart, floatpart;
    float temperature = mlx90614_get_temperature()
;
    intpart = temperature;
    floatpart = (temperature - intpart) * 100;
    floatpart = (floatpart >= 0) ? floatpart : -(floatpart);

    snprintf(response, DCON_MAX_BUF, ">%d.%02d\r", intpart, floatpart);
}

void Task7050Function(void *pvParameters) {
    struct msg msg;
    int result;

	sync_mutex = xSemaphoreCreateMutex();
	if (sync_mutex == NULL)
		return;

    dev_init();
    
    dcon_dev_register(&dev_7050);

    for (;;) {
        dcon_dev_recv(&dev_7050, &msg);

        result = parse_command(msg.request, cmds, ARRAY_SIZE(cmds));
        if (result != -1) {
            cmds[result].handler(msg.request, msg.response);
        } else {
            snprintf(msg.response, DCON_MAX_BUF, "?%02x\r",
                 dev_7050.addr);
        }
        
        dcon_dev_send(&dev_7050, &msg);
    }
}