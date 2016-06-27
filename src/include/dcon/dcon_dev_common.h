#ifndef _DCON_COMMON_H_
#define _DCON_COMMON_H_

typedef void (*cmd_handler_t)(const char *cmd, char *response);

struct cmd_t {
    const char *pattern;
    const char *check_bytes;
    cmd_handler_t handler;
};

int parse_command(const char *cmd, const struct cmd_t *cmds, size_t cmd_count);
void float2int(float fl, int *restrict intpart, int *restrict floatpart);

unsigned int hex_to_int(const char str[2]);

#endif /* _DCON_COMMON_H_ */
