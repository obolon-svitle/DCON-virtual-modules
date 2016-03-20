#include "lwip/lwiplib.h"
#include "server-coap.h"

#include <stellaris/hw_types.h>
#include <stellaris/driverlib/flash.h>
#include <stellaris/utils/uartstdio.h>

static unsigned char pucMACArray[8];

void TaskCoAPServerFunction(void *pvParameters) {
    unsigned long ulUser0, ulUser1;

    FlashUserGet(&ulUser0, &ulUser1);

    ulUser0 = 0x00123456;
    ulUser1 = 0x00987654;

    if ((ulUser0 == 0xffffffff) || (ulUser1 == 0xffffffff)) {
        while(1);            
    }

    pucMACArray[0] = ((ulUser0 >>  0) & 0xff);
    pucMACArray[1] = ((ulUser0 >>  8) & 0xff);
    pucMACArray[2] = ((ulUser0 >> 16) & 0xff);
    pucMACArray[3] = ((ulUser1 >>  0) & 0xff);
    pucMACArray[4] = ((ulUser1 >>  8) & 0xff);
    pucMACArray[5] = ((ulUser1 >> 16) & 0xff);

    lwIPInit(pucMACArray, 0, 0, 0, IPADDR_USE_DHCP);

    server_coap_serve();

    for (;;)
        ;
}
