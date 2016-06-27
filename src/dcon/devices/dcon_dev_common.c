#include <stddef.h>
#include <string.h>
#include <stdlib.h>
#include "common.h"

#include "dcon/dcon_dev_common.h"

void float2int(float fl, int *restrict intpart, int *restrict floatpart) {
	*intpart = fl;
	*floatpart = (fl - *intpart ) * 100;
	*floatpart = (*floatpart >= 0) ? *floatpart : -(*floatpart);
}

int parse_command(const char *cmd, const struct cmd_t *cmds,
                      size_t cmd_count) {
    size_t j, len, check_byte;
    for (size_t i = 0; i < cmd_count; i++) {

    	if (strlen(cmds[i].pattern) == strlen(cmd)) {
    		len = strlen(cmds[i].check_bytes);
    		for (j = 0; j < len; j++) {
    			check_byte = cmds[i].check_bytes[j] - '0';
    			if (cmds[i].pattern[check_byte] !=
    					cmd[check_byte])
    				break;
    		}

    		if (j == len) {
    			return i;
    		}
    	}
    }
    
    return -1;
}

/* The str MUST have at least 2 symbols long */
unsigned int hex_to_int(const char str[2]) {
    char addr_str[3];
    memcpy(addr_str, str, 2);
    addr_str[2] = '\0';

    return strtol(addr_str, NULL, 16);
}
