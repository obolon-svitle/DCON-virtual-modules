PROJ_NAME = dcon-virt-mod

RTOS_SOURCE_DIR = third_party/freertos
STELLARIS_DRIVER_DIR = third_party/stellaris
LWIP_SOURCE_DIR = third_party/lwip
COAP_SOURCE_DIR = third_party/coap

CC = arm-none-eabi-gcc

OBJCOPY = arm-none-eabi-objcopy
LDSCRIPT = ld/standalone.ld

PROJ_PATH := bin/$(PROJ_NAME)

LDFLAGS = -nostartfiles -Xlinker -Map=$(PROJ_PATH).map \
		-Xlinker --no-gc-sections -T $(LDSCRIPT)

DEBUG = -g3
OPTIM = -O0 -pipe

INCLUDE = -I include -I include/coap

#-D GCC_ARMCM3_LM3S9B95
#-D GCC_ARMCM3_LM3S102
CFLAGS = \
	$(DEBUG) $(OPTIM) $(INCLUDE) -std=c99 -pedantic -Wall -Wfatal-errors \
	-mthumb -mcpu=cortex-m3 -ffunction-sections -fdata-sections \
	-D GCC_ARMCM3_LM3S9B95 -DPART_LM3S9B95 -D gcc -D RTOS_FREERTOS -DIPv4 -DWITH_LWIP \
	-D UART_BUFFERED -Dsrand=usrand -D uipprintf=UARTprintf \
	-D printf=uipprintf -D sprintf=usprintf -D snprintf=usnprintf  \

SOURCE = \
	init/main.c \
	init/timers.c \
	dcon/dconmodules-core.c \
	dcon/devices/dcon_dev_common.c \
	dcon/devices/7050/7050.c \
	dcon/devices/7050/7050_hw.c \
	dcon/devices/7050/smbus/smbus.c \
	dcon/devices/7017/7017.c \
	dcon/devices/7017/7017_hw.c \
	dcon/devices/7024/7024.c \
	dcon/devices/7024/7024_hw.c \
	coapserver/server-coap.c \
	coapserver/coap_task.c \
	$(STELLARIS_DRIVER_DIR)/utils/uartstdio.c \
	$(wildcard $(STELLARIS_DRIVER_DIR)/driverlib/*.c) \
	$(STELLARIS_DRIVER_DIR)/EVB_9B95/drivers/set_pinout.c \
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
	$(COAP_SOURCE_DIR)/option.c \
	$(COAP_SOURCE_DIR)/hashkey.c \
	$(COAP_SOURCE_DIR)/encode.c \
	$(COAP_SOURCE_DIR)/coap_io_lwip.c \
	$(COAP_SOURCE_DIR)/block.c \
	$(COAP_SOURCE_DIR)/resource.c \
	$(COAP_SOURCE_DIR)/net.c \
	$(COAP_SOURCE_DIR)/uri.c \
	$(COAP_SOURCE_DIR)/pdu.c \
	$(COAP_SOURCE_DIR)/subscribe.c \
	$(COAP_SOURCE_DIR)/debug.c \
	$(COAP_SOURCE_DIR)/address.c

LIBS = 

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
