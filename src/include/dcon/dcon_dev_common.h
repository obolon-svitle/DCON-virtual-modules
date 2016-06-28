#ifndef _DCON_COMMON_H_
#define _DCON_COMMON_H_

#include <stdio.h>
#include "dcon/dcon_dev.h"

typedef void (*cmd_handler_t)(const char *cmd, char *response);

struct cmd_t {
    const char *pattern;
    const char *check_bytes;
    cmd_handler_t handler;
};

int parse_command(const char *cmd, const struct cmd_t *cmds, size_t cmd_count);
void float2int(float fl, int *restrict intpart, int *restrict floatpart);

unsigned int hex_to_int(const char str[2]);

static inline void dcon_run_device(struct dcon_dev *dev,
		const struct cmd_t *cmds,
		size_t cmd_count) {
    struct msg msg;
    int result;

    for (;;) {
        dcon_dev_recv(dev, &msg);

        result = parse_command(msg.request, cmds, cmd_count);
        if (result != -1) {
            cmds[result].handler(msg.request, msg.response);
        } else {
            snprintf(msg.response, DCON_MAX_RESPONSE_SIZE, "?%02x\r", dev->addr);
        }

        dcon_dev_send(dev, &msg);
    }
}

#endif /* _DCON_COMMON_H_ */
