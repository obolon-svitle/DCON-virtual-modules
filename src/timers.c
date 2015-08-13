#include "freertos/FreeRTOS.h"
#include "freertos/FreeRTOSConfig.h"

#include "stellaris/hw_ints.h"
#include "stellaris/hw_memmap.h"
#include "stellaris/hw_types.h"
#include "stellaris/driverlib/interrupt.h"
#include "stellaris/driverlib/sysctl.h"
#include "stellaris/driverlib/timer.h"

#define timerINTERRUPT_FREQUENCY        (20000UL)
#define timerHIGHEST_PRIORITY            0

volatile unsigned long ulHighFrequencyTimerTicks = 0UL;

void setup_timers(void) {
    unsigned long ulFrequency;
    SysCtlPeripheralEnable(SYSCTL_PERIPH_TIMER0);
    TimerConfigure(TIMER0_BASE, TIMER_CFG_32_BIT_PER);

    IntPrioritySet(INT_TIMER0A, configKERNEL_INTERRUPT_PRIORITY);

    portDISABLE_INTERRUPTS();
    ulFrequency = configCPU_CLOCK_HZ / timerINTERRUPT_FREQUENCY;
    TimerLoadSet(TIMER0_BASE, TIMER_A, ulFrequency);
    IntEnable(INT_TIMER0A);
    TimerIntEnable(TIMER0_BASE, TIMER_TIMA_TIMEOUT);
    TimerEnable(TIMER0_BASE, TIMER_A);
    portENABLE_INTERRUPTS();
}

void Timer0IntHandler(void) {
    TimerIntClear(TIMER0_BASE, TIMER_TIMA_TIMEOUT);
    ulHighFrequencyTimerTicks++;
}
