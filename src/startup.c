#include <stdint.h>

#ifndef STACK_SIZE
#define STACK_SIZE                              0xf00
#endif

void ResetISR(void);
static void NmiSR(void);
static void FaultISR(void);
static void IntDefaultHandler(void);

extern int main(void);
extern void xPortPendSVHandler(void);
extern void xPortSysTickHandler(void);
extern void vPortSVCHandler(void);
extern void Timer0IntHandler(void);
extern void UARTStdioIntHandler(void);
extern void lwIPEthernetIntHandler(void);
extern void SysTickIntHandler(void);

static unsigned long pulStack[STACK_SIZE];

__attribute__ ((section(".isr_vector")))
void (*const g_pfnVectors[]) (void) = {
    (void (*)(void)) ((unsigned long) pulStack + sizeof(pulStack)),    // The initial stack pointer
    ResetISR,             // The reset handler
    NmiSR,                // The NMI handler
    FaultISR,             // The hard fault handler
    IntDefaultHandler,    // The MPU fault handler
    IntDefaultHandler,    // The bus fault handler
    IntDefaultHandler,    // The usage fault handler
    0,                    // Reserved
    0,                    // Reserved
    0,                    // Reserved
    0,                    // Reserved
    vPortSVCHandler,      // SVCall handler
    IntDefaultHandler,    // Debug monitor handler
    0,                    // Reserved
    xPortPendSVHandler,   // The PendSV handler
    xPortSysTickHandler,  // The SysTick handler
    IntDefaultHandler,    // GPIO Port A
    IntDefaultHandler,    // GPIO Port B
    IntDefaultHandler,    // GPIO Port C
    IntDefaultHandler,    // GPIO Port D
    IntDefaultHandler,    // GPIO Port E
    UARTStdioIntHandler,  // UART0 Rx and Tx
    IntDefaultHandler,    // UART1 Rx and Tx
    IntDefaultHandler,    // SSI Rx and Tx
    IntDefaultHandler,    // I2C Master and Slave
    IntDefaultHandler,    // PWM Fault
    IntDefaultHandler,    // PWM Generator 0
    IntDefaultHandler,    // PWM Generator 1
    IntDefaultHandler,    // PWM Generator 2
    IntDefaultHandler,    // Quadrature Encoder
    IntDefaultHandler,    // ADC Sequence 0
    IntDefaultHandler,    // ADC Sequence 1
    IntDefaultHandler,    // ADC Sequence 2
    IntDefaultHandler,    // ADC Sequence 3
    IntDefaultHandler,    // Watchdog timer
    Timer0IntHandler,     // Timer 0 subtimer A
    IntDefaultHandler,    // Timer 0 subtimer B
    IntDefaultHandler,    // Timer 1 subtimer A
    IntDefaultHandler,    // Timer 1 subtimer B
    IntDefaultHandler,    // Timer 2 subtimer A
    IntDefaultHandler,    // Timer 2 subtimer B
    IntDefaultHandler,    // Analog Comparator 0
    IntDefaultHandler,    // Analog Comparator 1
    IntDefaultHandler,    // Analog Comparator 2
    IntDefaultHandler,    // System Control (PLL, OSC, BO)
    IntDefaultHandler,    // FLASH Control
    IntDefaultHandler,    // GPIO Port F
    IntDefaultHandler,    // GPIO Port G
    IntDefaultHandler,    // GPIO Port H
    IntDefaultHandler,    // UART2 Rx and Tx
    IntDefaultHandler,    // SSI1 Rx and Tx
    IntDefaultHandler,    // Timer 3 subtimer A
    IntDefaultHandler,    // Timer 3 subtimer B
    IntDefaultHandler,    // I2C1 Master and Slave
    IntDefaultHandler,    // Quadrature Encoder 1
    IntDefaultHandler,    // CAN0
    IntDefaultHandler,    // CAN1
    0,                    // Reserved
    lwIPEthernetIntHandler,      // Ethernet
    IntDefaultHandler     // Hibernate
};

extern unsigned long _etext;
extern unsigned long _data;
extern unsigned long _edata;
extern unsigned long _bss;
extern unsigned long _ebss;

//the variables will contain register values when a fault occured
volatile uint32_t r0;
volatile uint32_t r1;
volatile uint32_t r2;
volatile uint32_t r3;
volatile uint32_t r12;
volatile uint32_t lr;
volatile uint32_t pc;
volatile uint32_t psr;

void ResetISR(void) {
    unsigned long *pulSrc, *pulDest;

    pulSrc = &_etext;
    for (pulDest = &_data; pulDest < &_edata;) {
        *pulDest++ = *pulSrc++;
    }

    for (pulDest = &_bss; pulDest < &_ebss;) {
        *pulDest++ = 0;
    }

    main();
}

static void NmiSR(void) {
    for (;;);
}

void prvGetRegistersFromStack( uint32_t *pulFaultStackAddress ) {
    r0 = pulFaultStackAddress[0];
    r1 = pulFaultStackAddress[1];
    r2 = pulFaultStackAddress[2];
    r3 = pulFaultStackAddress[3];
    r12 = pulFaultStackAddress[4];
    lr = pulFaultStackAddress[5];
    pc = pulFaultStackAddress[6];
    psr = pulFaultStackAddress[7];
    for (;;) ;
}

static void HardFault_Handler(void) __attribute__((naked));
static void HardFault_Handler(void) {
    __asm volatile
    (
        " tst lr, #4                                                \n"
        " ite eq                                                    \n"
        " mrseq r0, msp                                             \n"
        " mrsne r0, psp                                             \n"
        " ldr r1, [r0, #24]                                         \n"
        " ldr r2, handler2_address_const                            \n"
        " bx r2                                                     \n"
        " handler2_address_const: .word prvGetRegistersFromStack    \n"
    );
}

static void FaultISR(void) {
    HardFault_Handler();
}

static void IntDefaultHandler(void) {
    for (;;) ;
}
