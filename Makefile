PROJ_NAME = dcon-virt-mod

RTOS_SOURCE_DIR = third_party/freertos
STELLARIS_DRIVER_DIR = stellaris
LWIP_SOURCE_DIR = third_party/lwip
COAP_SOURCE_DIR = third_party/coap

CC = arm-none-eabi-gcc

OBJCOPY = arm-none-eabi-objcopy
LDSCRIPT = ld/standalone.ld

PROJ_PATH := bin/$(PROJ_NAME)

# should use --gc-sections but the debugger does not seem to be able to cope with the option.
LDFLAGS = -nostartfiles -Xlinker -Map=$(PROJ_PATH).map -Xlinker --no-gc-sections -T $(LDSCRIPT)

DEBUG = -g3
OPTIM = -O0 -pipe

INCLUDE = -I include -I include/coap

CFLAGS = \
		$(DEBUG) $(OPTIM) $(INCLUDE) -std=c99 -pedantic -Wall -Wfatal-errors  \
		-mthumb -mcpu=cortex-m3\
		-ffunction-sections -fdata-sections\
		-D GCC_ARMCM3_LM3S102 -D gcc -D RTOS_FREERTOS -DIPv4 -DWITH_LWIP -D UART_BUFFERED -Dsrand=usrand \
		-D uipprintf=UARTprintf -D printf=uipprintf -D sprintf=usprintf -D snprintf=usnprintf  \

SOURCE = \
		init/main.c \
		init/timers.c \
		dcon/dconmodules-core.c \
		dcon/devices/7050.c \
		webserver/server-coap.c \
		webserver/lwip_task.c \
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
		$(LWIP_SOURCE_DIR)/core/timers.c \
		$(LWIP_SOURCE_DIR)/core/pbuf.c \
		$(LWIP_SOURCE_DIR)/core/sys.c \
		$(LWIP_SOURCE_DIR)/core/mem.c \
		$(LWIP_SOURCE_DIR)/core/memp.c \
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
		$(COAP_SOURCE_DIR)/option.o \
		$(COAP_SOURCE_DIR)/hashkey.o \
		$(COAP_SOURCE_DIR)/encode.o \
		$(COAP_SOURCE_DIR)/coap_io_lwip.o \
		$(COAP_SOURCE_DIR)/block.o \
		$(COAP_SOURCE_DIR)/resource.o \
		$(COAP_SOURCE_DIR)/net.o \
		$(COAP_SOURCE_DIR)/uri.o \
		$(COAP_SOURCE_DIR)/pdu.o \
		$(COAP_SOURCE_DIR)/subscribe.o \
		$(COAP_SOURCE_DIR)/debug.o \
		$(COAP_SOURCE_DIR)/address.c

LIBS = lib/gcc-cm3/libdriver-cm3.a

OBJS = $(SOURCE:.c=.o)

DEPDIR = .d
$(shell mkdir -p $(DEPDIR) > /dev/null)
DEPFLAGS = -MT $@ -MMD -MP
POSTCOMPILE = mkdir -p $(DEPDIR)/$(dir $*.d) && \
			  mv -f $*.d $(DEPDIR)/$*.d

.PHONY: clean

all : $(PROJ_PATH).bin Makefile

$(PROJ_PATH).bin : $(PROJ_PATH).axf
	$(OBJCOPY) $< -O binary $@

$(PROJ_PATH).axf : $(OBJS) init/startup.o
	$(CC) $(CFLAGS) $^ $(LIBS) $(LDFLAGS) -o $@

$(OBJS) : %.o : %.c $(DEPDIR)/%.d
	$(CC) $(CFLAGS) $(DEPFLAGS) -c $< -o $@
	$(POSTCOMPILE)

init/startup.o : %.o : %.c $(DEPDIR)/%.d
	$(CC) $(CFLAGS) $(DEPFLAGS) -O1 -c $< -o $@
	$(POSTCOMPILE)

$(DEPDIR)/%.d: ;

-include $(patsubst %,$(DEPDIR)/%.d,$(basename $(SOURCE)))

clean :
	rm -f $(OBJS) $(PROJ_PATH).bin $(PROJ_PATH).map $(PROJ_PATH).axf startup.o
