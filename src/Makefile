PROJ_NAME = restful-daq

RTOS_SOURCE_DIR = freertos
HTTPD_SOURCE_DIR = webserver
STELLARIS_DRIVER_DIR = stellaris
LWIP_SOURCE_DIR = webserver/lwip

CC = arm-none-eabi-gcc
OBJCOPY = arm-none-eabi-objcopy
LDSCRIPT = standalone.ld

# should use --gc-sections but the debugger does not seem to be able to cope with the option.
LINKER_FLAGS = -nostartfiles -Xlinker -o$(PROJ_NAME).axf -Xlinker -M -Xlinker -Map=$(PROJ_NAME).map -Xlinker --no-gc-sections

DEBUG = -g3
OPTIM = -O0 -pipe

INCLUDE = \
		-I $(STELLARIS_DRIVER_DIR) -I $(LWIP_SOURCE_DIR)/include -I$(HTTPD_SOURCE_DIR) -I$(LWIP_SOURCE_DIR)/ports/stellaris/include -I$(LWIP_SOURCE_DIR)/include/ipv4 \
	  -I . -I $(RTOS_SOURCE_DIR)/include -I $(RTOS_SOURCE_DIR)/portable/GCC/ARM_CM3

CFLAGS = \
		$(DEBUG) $(OPTIM) $(INCLUDE) -std=c99 -pedantic -Wall -Wfatal-errors -Werror -mthumb -mcpu=cortex-m3 \
		-ffunction-sections -fdata-sections -T$(LDSCRIPT) \
		-D GCC_ARMCM3_LM3S102 -D printf=uipprintf -D UART_BUFFERED -D RTOS_FREERTOS \
		-D sprintf=usprintf -D snprintf=usnprintf -fno-asynchronous-unwind-tables\
		-D free=vPortFree -D malloc=pvPortMalloc -D gcc
#-D PACK_STRUCT_END=__attribute\(\(packed\)\) -D ALIGN_STRUCT_END=__attribute\(\(aligned\(4\)\)\)

SOURCE = \
		main.c \
		webserver/lwip_task.c \
		uart_task.c \
		timers.c \
		$(STELLARIS_DRIVER_DIR)/utils/uartstdio.c \
		$(STELLARIS_DRIVER_DIR)/driverlib/ethernet.c \
		$(STELLARIS_DRIVER_DIR)/utils/ustdlib.c \
		$(STELLARIS_DRIVER_DIR)/utils/lwiplib.c \
		$(RTOS_SOURCE_DIR)/list.c \
		$(RTOS_SOURCE_DIR)/queue.c \
		$(RTOS_SOURCE_DIR)/tasks.c \
		$(RTOS_SOURCE_DIR)/portable/GCC/ARM_CM3/port.c \
		$(RTOS_SOURCE_DIR)/portable/MemMang/heap_4.c \
		$(LWIP_SOURCE_DIR)/api/tcpip.c \
		$(LWIP_SOURCE_DIR)/api/sockets.c \
		$(LWIP_SOURCE_DIR)/api/api_lib.c \
		$(LWIP_SOURCE_DIR)/api/api_msg.c \
		$(LWIP_SOURCE_DIR)/api/netbuf.c \
		$(LWIP_SOURCE_DIR)/netif/etharp.c \
		$(LWIP_SOURCE_DIR)/core/init.c \
		$(LWIP_SOURCE_DIR)/core/udp.c \
		$(LWIP_SOURCE_DIR)/core/dhcp.c \
        $(LWIP_SOURCE_DIR)/core/tcp.c \
        $(LWIP_SOURCE_DIR)/core/tcp_in.c \
        $(LWIP_SOURCE_DIR)/core/tcp_out.c \
		$(LWIP_SOURCE_DIR)/core/pbuf.c \
		$(LWIP_SOURCE_DIR)/core/sys.c \
		$(LWIP_SOURCE_DIR)/core/mem.c \
		$(LWIP_SOURCE_DIR)/core/memp.c \
		$(LWIP_SOURCE_DIR)/core/timers.c \
		$(LWIP_SOURCE_DIR)/core/raw.c \
		$(LWIP_SOURCE_DIR)/core/netif.c \
		$(LWIP_SOURCE_DIR)/core/ipv4/icmp.c \
		$(LWIP_SOURCE_DIR)/core/ipv4/inet.c \
		$(LWIP_SOURCE_DIR)/core/ipv4/inet_chksum.c \
		$(LWIP_SOURCE_DIR)/core/ipv4/ip_addr.c \
		$(LWIP_SOURCE_DIR)/core/ipv4/ip.c \
		$(LWIP_SOURCE_DIR)/core/ipv4/ip_frag.c \
		$(LWIP_SOURCE_DIR)/core/stats.c \
		$(LWIP_SOURCE_DIR)/ports/stellaris/perf.c \
		$(LWIP_SOURCE_DIR)/ports/stellaris/sys_arch.c \
		$(LWIP_SOURCE_DIR)/ports/stellaris/netif/stellarisif.c \
		$(HTTPD_SOURCE_DIR)/fs.c \
		$(HTTPD_SOURCE_DIR)/rest.c \
		$(HTTPD_SOURCE_DIR)/httpd.c

LIBS = $(STELLARIS_DRIVER_DIR)/driverlib/gcc-cm3/libdriver-cm3.a

OBJS = $(SOURCE:.c=.o)

.PHONY: clean

all: $(PROJ_NAME).bin

$(PROJ_NAME).bin : $(PROJ_NAME).axf
	$(OBJCOPY) $< -O binary $@

$(PROJ_NAME).axf : $(OBJS) startup.o Makefile standalone.ld
	$(CC) $(CFLAGS) $(OBJS) startup.o $(LIBS) $(LINKER_FLAGS)

$(OBJS) : %.o : %.c Makefile FreeRTOSConfig.h
	$(CC) -c $(CFLAGS) $< -o $@

startup.o : startup.c Makefile
	$(CC) -c $(CFLAGS) -O1 $< -o $@

clean :
	rm -f $(OBJS) $(PROJ_NAME).bin $(PROJ_NAME).axf startup.o
