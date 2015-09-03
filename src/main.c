/*
    FreeRTOS V8.0.0 - Copyright (C) 2014 Real Time Engineers Ltd.
    All rights reserved

    VISIT http://www.FreeRTOS.org TO ENSURE YOU ARE USING THE LATEST VERSION.

    ***************************************************************************
     *                                                                       *
     *    FreeRTOS provides completely free yet professionally developed,    *
     *    robust, strictly quality controlled, supported, and cross          *
     *    platform software that has become a de facto standard.             *
     *                                                                       *
     *    Help yourself get started quickly and support the FreeRTOS         *
     *    project by purchasing a FreeRTOS tutorial book, reference          *
     *    manual, or both from: http://www.FreeRTOS.org/Documentation        *
     *                                                                       *
     *    Thank you!                                                         *
     *                                                                       *
    ***************************************************************************

    This file is part of the FreeRTOS distribution.

    FreeRTOS is free software; you can redistribute it and/or modify it under
    the terms of the GNU General Public License (version 2) as published by the
    Free Software Foundation >>!AND MODIFIED BY!<< the FreeRTOS exception.

    >>! NOTE: The modification to the GPL is included to allow you to distribute
    >>! a combined work that includes FreeRTOS without being obliged to provide
    >>! the source code for proprietary components outside of the FreeRTOS
    >>! kernel.

    FreeRTOS is distributed in the hope that it will be useful, but WITHOUT ANY
    WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
    FOR A PARTICULAR PURPOSE.  Full license text is available from the following
    link: http://www.freertos.org/a00114.html

    1 tab == 4 spaces!

    ***************************************************************************
     *                                                                       *
     *    Having a problem?  Start by reading the FAQ "My application does   *
     *    not run, what could be wrong?"                                     *
     *                                                                       *
     *    http://www.FreeRTOS.org/FAQHelp.html                               *
     *                                                                       *
    ***************************************************************************

    http://www.FreeRTOS.org - Documentation, books, training, latest versions,
    license and Real Time Engineers Ltd. contact details.

    http://www.FreeRTOS.org/plus - A selection of FreeRTOS ecosystem products,
    including FreeRTOS+Trace - an indispensable productivity tool, a DOS
    compatible FAT file system, and our tiny thread aware UDP/IP stack.

    http://www.OpenRTOS.com - Real Time Engineers ltd license FreeRTOS to High
    Integrity Systems to sell under the OpenRTOS brand.  Low cost OpenRTOS
     licenses offer ticketed support, indemnification and middleware.

    http://www.SafeRTOS.com - High Integrity Systems also provide a safety
    engineered and independently SIL3 certified version for use in safety and
    mission critical applications that require provable dependability.

    1 tab == 4 spaces!
*/

#include <stdio.h>

#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "semphr.h"

#include <inc/hw_memmap.h>
#include <inc/hw_types.h>
#include <inc/hw_ints.h>
#include <inc/hw_sysctl.h>
#include <driverlib/sysctl.h>
#include <driverlib/gpio.h>
#include <driverlib/systick.h>
#include <driverlib/interrupt.h>
#include <utils/uartstdio.h>
#include <utils/lwiplib.h>
#include <stdio.h>

#define LWIP_STACK_SIZE 300

static void prvSetupHardware(void);
extern void setup_timers(void);

/*-----------------------------------------------------------*/

extern void TaskLWIPFunction(void *pvParameters);

int main( void ) {
	prvSetupHardware();
	setup_timers();
	
	if (SysCtlPeripheralPresent(SYSCTL_PERIPH_ETH)) {
		xTaskCreate(TaskLWIPFunction, "lwip", LWIP_STACK_SIZE, NULL, 1, NULL);
	}

	vTaskStartScheduler();
	
	for (; ;)
		;
	
	return 0;
}
/*-----------------------------------------------------------*/

void prvSetupHardware( void ) {
        
	/* If running on Rev A2 silicon, turn the LDO voltage up to 2.75V.  This is
	a workaround to allow the PLL to operate reliably. */
	if( DEVICE_IS_REVA2 ) {
		SysCtlLDOSet( SYSCTL_LDO_2_75V );
	}
	
	/* Set the clocking to run from the PLL at 50 MHz */
	SysCtlClockSet(SYSCTL_SYSDIV_4 | SYSCTL_OSC_MAIN | SYSCTL_USE_PLL |
                   SYSCTL_XTAL_16MHZ);
	GPIOPinTypeUART(GPIO_PORTA_BASE, GPIO_PIN_0 | GPIO_PIN_1);
	
	GPIOPinTypeEthernetLED(GPIO_PORTF_BASE, GPIO_PIN_2 | GPIO_PIN_3);
	
	IntPrioritySet(INT_ETH, configKERNEL_INTERRUPT_PRIORITY);
	IntPrioritySet(INT_UART0, configKERNEL_INTERRUPT_PRIORITY);

	SysTickPeriodSet(SysCtlClockGet() / configTICK_RATE_HZ);
	SysTickEnable();
	SysTickIntEnable();

	IntMasterEnable();

	UARTStdioInit(0);
}
/*-----------------------------------------------------------*/

void vApplicationStackOverflowHook(TaskHandle_t *pxTask, signed char *pcTaskName) {
	(void) pxTask;
	UARTprintf("Stack overflow in %s task", pcTaskName);

	for( ;; )
		;
}
/*-----------------------------------------------------------*/

void vAssertCalled( const char *pcFile, unsigned long ulLine ) {
volatile unsigned long ulSetTo1InDebuggerToExit = 0;

	taskENTER_CRITICAL(); {
		while( ulSetTo1InDebuggerToExit == 0 )
		{
			/* Nothing do do here.  Set the loop variable to a non zero value in
			the debugger to step out of this function to the point that caused
			the assertion. */
			( void ) pcFile;
			( void ) ulLine;
		}
	}
	taskEXIT_CRITICAL();
}
