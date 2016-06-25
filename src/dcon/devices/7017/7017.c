#include <stdio.h>
#include <stdarg.h>
#include <stdlib.h>
#include <string.h>

#include <common.h>

#include <dcon/dcon_dev.h>
#include "dcon/dcon_dev_common.h"
#include "7017.h"
#include "7017_hw.h"

#define MODULE_7017_ADDR 0x02
#define MODULE_7017_TYPE 0x0A
#define MODULE_7017_NAME "7017"

/* Forward declarations */
static void set_7017_config(const char *, char *);
static void get_7017_name(const char *, char *);
static void get_7017_config(const char *, char *);
static void set_7017_sync_data(const char *, char *);
static void get_7017_sync_data(const char *, char *);
static void get_7017_adc_data(const char *, char *);
static void get_7017_adc_channel_data(const char *, char *);

static const struct cmd_t cmds[] = {
    {"%AANN40CCFF", "056", set_7017_config},
    {"$AAM", "03", get_7017_name},
    {"$AA2", "03", get_7017_config},
    {"#**", "012", set_7017_sync_data},
    {"$AA4", "03", get_7017_sync_data},
    {"#AA", "0", get_7017_adc_data},
    {"#AAN", "0", get_7017_adc_channel_data},
};

static struct dcon_dev dev_7017 = {
    .addr = MODULE_7017_ADDR,
    .type = MODULE_7017_TYPE,
};

static int sync_val_int = 0;
static int sync_val_float = 0;
static int readed = 0;

static void set_7017_config(const char *request, char *response) {
    char char_addr[3];

    strncpy(char_addr, request + 5, ARRAY_SIZE(char_addr) - 1);
    char_addr[2] = '\0';

    dev_7017.addr = hex_to_int(char_addr);
    
    snprintf(response, DCON_MAX_BUF, "!\r");
}

static void get_7017_name(const char *request, char *response) {
    UNUSED(request);
    snprintf(response, DCON_MAX_BUF, "!%02x%s\r", dev_7017.addr,
             MODULE_7017_NAME);
}

static void get_7017_config(const char *request, char *response) {
    UNUSED(request);
    snprintf(response, DCON_MAX_BUF, "!%02x%02d0000\r", dev_7017.addr,
             dev_7017.type);    
}

static void set_7017_sync_data(const char *request, char *response) {
    UNUSED(request);
    
    get_voltage(0, &sync_val_int, &sync_val_float);

    readed = 0;

    snprintf(response, DCON_MAX_BUF, "\r");
}

static void get_7017_sync_data(const char *request, char *response) {
    UNUSED(request);
    int intpart;
    int floatpart;
    char *pos = response;
    
    snprintf(response, DCON_MAX_BUF, ">%02x%d+%02d.%d",
             MODULE_7017_ADDR, readed, sync_val_int,
             sync_val_float);
    readed = 1;
}

static void get_7017_adc_data(const char *request, char *response) {
    int intpart;
    int floatpart;
    char *pos = response;
    ssize_t max_size;

    UNUSED(request);

    for (size_t i = 0; i < CHANNEL_COUNT; i++) {
        max_size = DCON_MAX_BUF - 2 - (pos - response);
        if (max_size < 0)
            break;
        get_voltage(i, &intpart, &floatpart);
        pos += snprintf(pos, max_size,
                        "+%d.%d", intpart, floatpart);
    }
    *(pos) = '\r';
    *(pos + 1) = '\0';
}
static void get_7017_adc_channel_data(const char *request, char *response) {
    int intpart, floatpart;

    get_voltage(request[3] - '0', &intpart, &floatpart);
    snprintf(response, DCON_MAX_BUF, ">+%01d.%d\r", intpart, floatpart);
}

void Task7017Function(void *pvParameters){
    struct msg msg;
    int result;

    UNUSED(pvParameters);

    init_7017();

    dcon_dev_register(&dev_7017);

    for (;;) {
        dcon_dev_recv(&dev_7017, &msg);

        result = parse_command(msg.request, cmds, ARRAY_SIZE(cmds));
        if (result != -1) {
            cmds[result].handler(msg.request, msg.response);
        } else {
            snprintf(msg.response, DCON_MAX_BUF, "?%02x\r", dev_7017.addr);
        }

        dcon_dev_send(&dev_7017, &msg);
    }
}
