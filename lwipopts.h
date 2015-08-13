#include <stdint.h>
#include <stddef.h>
#include "portmacro.h"
#include "inc/hw_types.h"
#include "driverlib/ethernet.h"

/*Stellaris*/
#define BYTE_ORDER LITTLE_ENDIAN
#define LWIP_PREFIX_BYTEORDER_FUNCS
#define LWIP_PLATFORM_BYTESWAP 1

#define LWIP_PLATFORM_HTONS(x) htons(x)
#define LWIP_PLATFORM_HTONL(x) htonl(x)

#define MEMP_MEM_MALLOC 1
#define MEM_LIBC_MALLOC 1
#define MEMP_NUM_PBUF                     48
#define MEMP_NUM_TCP_PCB                  16
#define MEMP_NUM_SYS_TIMEOUT              8
#define PBUF_POOL_SIZE                    48

#define SYS_LIGHTWEIGHT_PROT 1
#define LWIP_DHCP                       1
#define IP_REASSEMBLY 0
#define IP_FRAG 0


#define ETH_PAD_SIZE 2

#define TCPIP_THREAD_NAME              "tcpip_thread"
#define TCPIP_THREAD_STACKSIZE          300
#define TCPIP_THREAD_PRIO               1

#define DEFAULT_THREAD_NAME            "lwIP"
#define DEFAULT_THREAD_STACKSIZE        300
#define DEFAULT_THREAD_PRIO             1

#define LWIP_STATS                      0
#define LWIP_DEBUG
#define ICMP_DEBUG LWIP_DBG_ON
#define DHCP_DEBUG                      LWIP_DBG_ON
#define ICMP_DEBUG                      LWIP_DBG_ON
#define INET_DEBUG                      LWIP_DBG_ON
#define PBUF_DEBUG                      LWIP_DBG_ON

