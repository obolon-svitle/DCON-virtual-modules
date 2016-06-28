NAME := dcon-virt-mod

CC      ?= arm-none-eabi-gcc
OBJCOPY ?= arm-none-eabi-objcopy

RTOS_SOURCE_DIR      = third_party/freertos/
STELLARIS_DRIVER_DIR = third_party/stellaris/
LWIP_SOURCE_DIR      = third_party/lwip/
COAP_SOURCE_DIR      = third_party/coap/

BUILD_DIR = bin/

BIN_PATH = $(BUILD_DIR)/$(NAME)

LDSCRIPT = src/standalone.ld

LDFLAGS = -nostartfiles -Xlinker -Map=$(BIN_PATH).map -Xlinker --no-gc-sections -T $(LDSCRIPT)

DEBUG = -g3
OPTIM = -O0 -pipe

INCLUDE = -I src/include -I$(RTOS_SOURCE_DIR)/include -I$(COAP_SOURCE_DIR)/include/coap \
          -I$(COAP_SOURCE_DIR)/include -I$(STELLARIS_DRIVER_DIR)/include -I$(LWIP_SOURCE_DIR)/include

CFLAGS += $(DEBUG) $(OPTIM) $(INCLUDE)
CFLAGS += -std=gnu99 -pedantic -Wall -Wextra -Wfatal-errors 
CFLAGS += -mcpu=cortex-m3 -mthumb 
CFLAGS += -Dgcc -DRTOS_FREERTOS -DIPv4 -DWITH_LWIP
CFLAGS += -DUART_BUFFERED -Dsrand=usrand -Duipprintf=UARTprintf -Dprintf=uipprintf -Dsprintf=usprintf -Dsnprintf=usnprintf
#CFLAGS +=-ffunction-sections -fdata-sections

ifdef DEBUG_LOG
CFLAGS += -DDEBUG_LOG
endif

ifdef QEMU_BUILD
CFLAGS += -DPART_LM3S6965
else
CFLAGS += -DPART_LM3S9B95
endif

SOURCE = \
	src/main.c \
	src/timers.c \
	src/dcon/dconmodules-core.c \
	src/dcon/devices/dcon_dev_common.c \
	src/dcon/devices/7050/7050.c \
	src/dcon/devices/7050/7050_hw.c \
	src/dcon/devices/7050/smbus/smbus.c \
	src/dcon/devices/7017/7017.c \
	src/dcon/devices/7017/7017_hw.c \
	src/dcon/devices/7024/7024.c \
	src/dcon/devices/7024/7024_hw.c \
	src/server-coap.c \
	$(STELLARIS_DRIVER_DIR)/utils/uartstdio.c \
	$(wildcard $(STELLARIS_DRIVER_DIR)/driverlib/*.c) \
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
	$(COAP_SOURCE_DIR)/src/option.c \
	$(COAP_SOURCE_DIR)/src/hashkey.c \
	$(COAP_SOURCE_DIR)/src/encode.c \
	$(COAP_SOURCE_DIR)/src/coap_io_lwip.c \
	$(COAP_SOURCE_DIR)/src/block.c \
	$(COAP_SOURCE_DIR)/src/resource.c \
	$(COAP_SOURCE_DIR)/src/net.c \
	$(COAP_SOURCE_DIR)/src/uri.c \
	$(COAP_SOURCE_DIR)/src/pdu.c \
	$(COAP_SOURCE_DIR)/src/subscribe.c \
	$(COAP_SOURCE_DIR)/src/debug.c \
	$(COAP_SOURCE_DIR)/src/address.c

ifndef QEMU_BUILD
SOURCE += $(STELLARIS_DRIVER_DIR)/EVB_9B95/drivers/set_pinout.c
endif

LIBS = 

OBJS := $(addprefix $(BUILD_DIR)/, $(SOURCE:.c=.o))

DEPDIR = $(BUILD_DIR)/.d
$(shell mkdir -p $(DEPDIR) > /dev/null)
DEPFLAGS = -MT $@ -MMD -MP
POSTCOMPILE = mkdir -p $(DEPDIR)/$(dir $*.d) && mv -f $(BUILD_DIR)/$*.d $(DEPDIR)/$*.d

.PHONY: clean

all : $(BIN_PATH).bin
	
$(BIN_PATH).bin : $(BIN_PATH).axf
	@mkdir -p $(dir $@)
	$(OBJCOPY) $< -O binary $@

$(BIN_PATH).axf : $(OBJS) $(BUILD_DIR)/src/startup.o
	@mkdir -p $(dir $@)
	$(CC) $(CFLAGS) $^ $(LIBS) $(LDFLAGS) -o $@
	
$(BUILD_DIR)/src/startup.o : $(BUILD_DIR)/%.o : %.c $(DEPDIR)/%.d
	@mkdir -p $(dir $@)
	$(CC) $(CFLAGS) $(DEPFLAGS) -O1 -c $< -o $@
	$(POSTCOMPILE)

$(OBJS) : $(BUILD_DIR)/%.o : %.c $(DEPDIR)/%.d
	@mkdir -p $(dir $@)
	$(CC) $(CFLAGS) $(DEPFLAGS) -c $< -o $@
	$(POSTCOMPILE)

$(DEPDIR)/%.d: ;

-include $(patsubst %,$(DEPDIR)/%.d,$(basename $(SOURCE)))

clean :
	rm -rf $(DEPDIR) $(BUILD_DIR) $(OBJS) $(PROJ_PATH).bin $(PROJ_PATH).map \
		   $(PROJ_PATH).axf $(BUILD_DIR)/startup.o
