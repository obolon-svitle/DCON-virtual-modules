#ifndef _COMMON_H_
#define _COMMON_H_

#include "stellaris/hw_types.h"
#include "stellaris/utils/uartstdio.h"

#ifndef ARRAY_SIZE
#define ARRAY_SIZE(x) (sizeof(x) / sizeof(x[0]))
#endif

#ifndef UNUSED
#define UNUSED(x) ((void)(x))
#endif

#ifndef DEBUG_LOG
#define DVM_LOG_I(...)
#define DVM_LOG_W(...)
#define DVM_LOG_E(...)
#define DVM_LOG_D(...)
#else
#define DVM_LOG_I(fmt, ...) UARTprintf("I: " fmt "\n", ##__VA_ARGS__)
#define DVM_LOG_W(fmt, ...) UARTprintf("W: " fmt "\n", ##__VA_ARGS__)
#define DVM_LOG_E(fmt, ...) UARTprintf("E: " fmt "\n", ##__VA_ARGS__)
#define DVM_LOG_D(fmt, ...) UARTprintf("D: " fmt "\n", ##__VA_ARGS__)
#endif

#endif
