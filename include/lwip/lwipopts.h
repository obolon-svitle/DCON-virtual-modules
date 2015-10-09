#include <stdint.h>
#include <stddef.h>
#include "freertos/portmacro.h"
#include "stellaris/hw_types.h"
#include "stellaris/driverlib/ethernet.h"

/*Stellaris*/
#define BYTE_ORDER LITTLE_ENDIAN
#define LWIP_PREFIX_BYTEORDER_FUNCS 1
#define LWIP_PLATFORM_BYTESWAP 1
#define LWIP_PLATFORM_HTONS(x) htons(x)
#define LWIP_PLATFORM_HTONL(x) htonl(x)

#define MEMP_MEM_MALLOC 1
#define MEM_LIBC_MALLOC 1
#define free vPortFree
#define malloc pvPortMalloc
#define mem_free free
#define mem_malloc malloc
#define mem_realloc(mem, size) mem

#define PBUF_LINK_HLEN 16

#define MEMP_NUM_PBUF                     48
#define MEMP_NUM_TCP_PCB                  16
#define MEMP_NUM_SYS_TIMEOUT              8
#define PBUF_POOL_SIZE                    48

#define SYS_LIGHTWEIGHT_PROT 1
#define LWIP_DHCP                       1
#define IP_REASSEMBLY 0
#define IP_FRAG 0

#define LWIP_SOCKET 0
#define TCPIP_MBOX_SIZE sizeof(void *)

#define ETH_PAD_SIZE 2

#define TCPIP_THREAD_NAME              "tcpip_thread"
#define TCPIP_THREAD_STACKSIZE          200
#define TCPIP_THREAD_PRIO               1

#define TCP_SND_BUF (2 * TCP_MSS)

#define DEFAULT_THREAD_NAME            "lwIP"
#define DEFAULT_THREAD_STACKSIZE        100
#define DEFAULT_THREAD_PRIO             1

#define LWIP_STATS                      0
#define LWIP_DEBUG
#define HTTPD_DEBUG LWIP_DBG_ON


