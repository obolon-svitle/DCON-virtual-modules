#include <stdio.h>

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"
#include "freertos/semphr.h"

#include "stellaris/hw_memmap.h"
#include "stellaris/hw_types.h"
#include "stellaris/hw_ints.h"
#include "stellaris/hw_sysctl.h"
#include "stellaris/driverlib/sysctl.h"
#include "stellaris/driverlib/gpio.h"
#include "stellaris/driverlib/systick.h"
#include "stellaris/driverlib/interrupt.h"
#include "stellaris/utils/uartstdio.h"

#ifdef DPART_LM3S9B95
#include "stellaris/EVB_9B95/drivers/set_pinout.h"
#endif

#include "common.h"

#include "coap/coap_task.h"
#include "dcon/dcon_init.h"

#define COAP_STACK_SIZE 200
#define COAP_TASK_PRIORITY 1

static void setup_hardware(void);
extern void setup_timers(void);

int main(void) {
    setup_hardware();
    setup_timers();

    dcon_init();

    if (SysCtlPeripheralPresent(SYSCTL_PERIPH_ETH)) {
        xTaskCreate(TaskCoAPServerFunction, "lwip", COAP_STACK_SIZE,
                    NULL, COAP_TASK_PRIORITY, NULL);
    }

    vTaskStartScheduler();
    
    for (; ;)
        ;
    
    return 0;
}
/*-----------------------------------------------------------*/

static void setup_hardware(void) {
        
    /* If running on Rev A2 silicon, turn the LDO voltage up to 2.75V.  This is
    a workaround to allow the PLL to operate reliably. */
    if (DEVICE_IS_REVA2) {
        SysCtlLDOSet(SYSCTL_LDO_2_75V);
    }
    
    /* Set the clocking to run from the PLL at 50 MHz */
    SysCtlClockSet(SYSCTL_SYSDIV_4 | SYSCTL_OSC_MAIN | SYSCTL_USE_PLL |
                   SYSCTL_XTAL_16MHZ);

#ifdef DPART_LM3S9B95
    PinoutSet();
#endif

    GPIOPinTypeUART(GPIO_PORTA_BASE, GPIO_PIN_0 | GPIO_PIN_1);
    
    GPIOPinTypeEthernetLED(GPIO_PORTF_BASE, GPIO_PIN_2 | GPIO_PIN_3);
    
    IntPrioritySet(INT_ETH, configKERNEL_INTERRUPT_PRIORITY);
    IntPrioritySet(INT_UART0, configMAX_SYSCALL_INTERRUPT_PRIORITY);

    UARTStdioInit(0);
}

void vApplicationStackOverflowHook(TaskHandle_t *pxTask, signed char *pcTaskName) {
    (void) pxTask;
    UARTprintf("Stack overflow in %s task\n", pcTaskName);

    for( ;; )
        ;
}

void vAssertCalled( const char *pcFile, unsigned long ulLine ) {
volatile unsigned long ulSetTo1InDebuggerToExit = 0;

    taskENTER_CRITICAL(); {
        while (ulSetTo1InDebuggerToExit == 0) {
            ( void ) pcFile;
            ( void ) ulLine;
        }
    }
    taskEXIT_CRITICAL();
}
