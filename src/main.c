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


/*
 * Creates all the demo application tasks, then starts the scheduler.  The WEB
 * documentation provides more details of the standard demo application tasks.
 * In addition to the standard demo tasks, the following tasks and tests are
 * defined and/or created within this file:
 *
 * "Fast Interrupt Test" - A high frequency periodic interrupt is generated
 * using a free running timer to demonstrate the use of the
 * configKERNEL_INTERRUPT_PRIORITY configuration constant.  The interrupt
 * service routine measures the number of processor clocks that occur between
 * each interrupt - and in so doing measures the jitter in the interrupt timing.
 * The maximum measured jitter time is latched in the ulMaxJitter variable, and
 * displayed on the OLED display by the 'OLED' task as described below.  The
 * fast interrupt is configured and handled in the timertest.c source file.
 *
 * "OLED" task - the OLED task is a 'gatekeeper' task.  It is the only task that
 * is permitted to access the display directly.  Other tasks wishing to write a
 * message to the OLED send the message on a queue to the OLED task instead of
 * accessing the OLED themselves.  The OLED task just blocks on the queue waiting
 * for messages - waking and displaying the messages as they arrive.
 *
 * "Check" hook -  This only executes every five seconds from the tick hook.
 * Its main function is to check that all the standard demo tasks are still
 * operational.  Should any unexpected behaviour within a demo task be discovered
 * the tick hook will write an error to the OLED (via the OLED task).  If all the
 * demo tasks are executing with their expected behaviour then the check task
 * writes PASS to the OLED (again via the OLED task), as described above.
 *
 * "uIP" task -  This is the task that handles the uIP stack.  All TCP/IP
 * processing is performed in this task.
 */




/*************************************************************************
 * Please ensure to read http://www.freertos.org/portLM3Sxxxx_Eclipse.html
 * which provides information on configuring and running this demo for the
 * various Luminary Micro EKs.
 *************************************************************************/

/* Standard includes. */
#include <stdio.h>

/* Scheduler includes. */
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "semphr.h"

/* Hardware library includes. */
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

/*-----------------------------------------------------------*/
#define SYSTICKHZ 100
#define SYSTICKMS (1000 / SYSTICKHZ)

#define TASK_STACK_SIZE 300

static void prvSetupHardware( void );

extern void setup_timers(void);

/*-----------------------------------------------------------*/

extern void TaskLWIPFunction(void *pvParameters);

extern void TaskSensorFunction(void* r);

int main( void ) {
	prvSetupHardware();

	UARTprintf("ALLO YOBA ETO TI\n");

	setup_timers();

	if( SysCtlPeripheralPresent( SYSCTL_PERIPH_ETH ) ) {
		xTaskCreate(TaskLWIPFunction, "lwip", TASK_STACK_SIZE, NULL, 1, NULL);
	}
	xTaskCreate(TaskSensorFunction, "sensor", TASK_STACK_SIZE, NULL, 1, NULL);

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
#include "set_pinout.h"
	PinoutSet();
	
	GPIOPinTypeUART(GPIO_PORTA_BASE, GPIO_PIN_0 | GPIO_PIN_1);
	
	GPIOPinTypeEthernetLED(GPIO_PORTF_BASE, GPIO_PIN_2 | GPIO_PIN_3);
	
	IntPrioritySet(INT_ETH, configKERNEL_INTERRUPT_PRIORITY);
	IntPrioritySet(INT_SYSCTL, configKERNEL_INTERRUPT_PRIORITY);
	IntPrioritySet(INT_UART0, configKERNEL_INTERRUPT_PRIORITY);

	#if 0
	SysTickPeriodSet(SysCtlClockGet() / 1000);
	SysTickEnable();
	SysTickIntEnable();

	IntMasterEnable();
	#endif

	UARTStdioInit(0);
}
/*-----------------------------------------------------------*/

void vApplicationStackOverflowHook( TaskHandle_t *pxTask, signed char *pcTaskName ) {
	( void ) pxTask;
	( void ) pcTaskName;
	UARTprintf("Stack Overflow\n");

	for( ;; );
}
/*-----------------------------------------------------------*/

void vAssertCalled( const char *pcFile, unsigned long ulLine ) {
volatile unsigned long ulSetTo1InDebuggerToExit = 0;

	taskENTER_CRITICAL();
	{
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
