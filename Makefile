NAME := dcon-virt-mod

CC      ?= arm-none-eabi-gcc
OBJCOPY ?= arm-none-eabi-objcopy

RTOS_SOURCE_DIR      = third_party/freertos/
STELLARIS_DRIVER_DIR = third_party/stellaris/
LWIP_SOURCE_DIR      = third_party/lwip/
COAP_SOURCE_DIR      = third_party/coap/
DCON_PROJ_DIR        = src/

BUILD_DIR = bin/
BIN_NAME = $(BUILD_DIR)/$(NAME)

LDSCRIPT = src/linker.ld
LDFLAGS = -nostartfiles -Xlinker -Map=$(BIN_NAME).map -Xlinker --no-gc-sections -T $(LDSCRIPT)

DEBUG = -g3
OPTIM = -O0 -pipe

CFLAGS += $(DEBUG) $(OPTIM)
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

INC += src/include
INC += $(RTOS_SOURCE_DIR)/include
INC += $(COAP_SOURCE_DIR)/include/coap
INC += $(COAP_SOURCE_DIR)/include
INC += $(STELLARIS_DRIVER_DIR)/include
INC += $(LWIP_SOURCE_DIR)/include

SRC += $(DCON_PROJ_DIR)/main.c
SRC += $(DCON_PROJ_DIR)/timers.c
SRC += $(DCON_PROJ_DIR)/dcon/dconmodules-core.c
SRC += $(DCON_PROJ_DIR)/dcon/devices/dcon_dev_common.c
SRC += $(DCON_PROJ_DIR)/dcon/devices/7050/7050.c
SRC += $(DCON_PROJ_DIR)/dcon/devices/7050/7050_hw.c
SRC += $(DCON_PROJ_DIR)/dcon/devices/7050/smbus/smbus.c
SRC += $(DCON_PROJ_DIR)/dcon/devices/7017/7017.c
SRC += $(DCON_PROJ_DIR)/dcon/devices/7017/7017_hw.c
SRC += $(DCON_PROJ_DIR)/dcon/devices/7024/7024.c
SRC += $(DCON_PROJ_DIR)/dcon/devices/7024/7024_hw.c
SRC += $(DCON_PROJ_DIR)/server-coap.c
SRC += $(STELLARIS_DRIVER_DIR)/utils/uartstdio.c
SRC += $(wildcard $(STELLARIS_DRIVER_DIR)/driverlib/*.c)
SRC += $(STELLARIS_DRIVER_DIR)/utils/ustdlib.c
SRC += $(STELLARIS_DRIVER_DIR)/utils/lwiplib.c
SRC += $(RTOS_SOURCE_DIR)/list.c
SRC += $(RTOS_SOURCE_DIR)/queue.c
SRC += $(RTOS_SOURCE_DIR)/tasks.c
SRC += $(RTOS_SOURCE_DIR)/portable/GCC/ARM_CM3/port.c
SRC += $(RTOS_SOURCE_DIR)/portable/MemMang/heap_4.c
SRC += $(LWIP_SOURCE_DIR)/api/tcpip.c
SRC += $(LWIP_SOURCE_DIR)/api/sockets.c
SRC += $(LWIP_SOURCE_DIR)/api/api_lib.c
SRC += $(LWIP_SOURCE_DIR)/api/api_msg.c
SRC += $(LWIP_SOURCE_DIR)/api/netbuf.c
SRC += $(LWIP_SOURCE_DIR)/netif/etharp.c
SRC += $(LWIP_SOURCE_DIR)/core/init.c
SRC += $(LWIP_SOURCE_DIR)/core/udp.c
SRC += $(LWIP_SOURCE_DIR)/core/dhcp.c
SRC += $(LWIP_SOURCE_DIR)/core/tcp.c
SRC += $(LWIP_SOURCE_DIR)/core/tcp_in.c
SRC += $(LWIP_SOURCE_DIR)/core/tcp_out.c
SRC += $(LWIP_SOURCE_DIR)/core/timers.c
SRC += $(LWIP_SOURCE_DIR)/core/pbuf.c
SRC += $(LWIP_SOURCE_DIR)/core/sys.c
SRC += $(LWIP_SOURCE_DIR)/core/mem.c
SRC += $(LWIP_SOURCE_DIR)/core/memp.c
SRC += $(LWIP_SOURCE_DIR)/core/raw.c
SRC += $(LWIP_SOURCE_DIR)/core/netif.c
SRC += $(LWIP_SOURCE_DIR)/core/ipv4/icmp.c
SRC += $(LWIP_SOURCE_DIR)/core/ipv4/inet.c
SRC += $(LWIP_SOURCE_DIR)/core/ipv4/inet_chksum.c
SRC += $(LWIP_SOURCE_DIR)/core/ipv4/ip_addr.c
SRC += $(LWIP_SOURCE_DIR)/core/ipv4/ip.c
SRC += $(LWIP_SOURCE_DIR)/core/ipv4/ip_frag.c
SRC += $(LWIP_SOURCE_DIR)/core/stats.c
SRC += $(LWIP_SOURCE_DIR)/ports/stellaris/perf.c
SRC += $(LWIP_SOURCE_DIR)/ports/stellaris/sys_arch.c
SRC += $(LWIP_SOURCE_DIR)/ports/stellaris/netif/stellarisif.c
SRC += $(COAP_SOURCE_DIR)/src/option.c
SRC += $(COAP_SOURCE_DIR)/src/hashkey.c
SRC += $(COAP_SOURCE_DIR)/src/encode.c
SRC += $(COAP_SOURCE_DIR)/src/coap_io_lwip.c
SRC += $(COAP_SOURCE_DIR)/src/block.c
SRC += $(COAP_SOURCE_DIR)/src/resource.c
SRC += $(COAP_SOURCE_DIR)/src/net.c
SRC += $(COAP_SOURCE_DIR)/src/uri.c
SRC += $(COAP_SOURCE_DIR)/src/pdu.c
SRC += $(COAP_SOURCE_DIR)/src/subscribe.c
SRC += $(COAP_SOURCE_DIR)/src/debug.c
SRC += $(COAP_SOURCE_DIR)/src/address.c
ifndef QEMU_BUILD
SRC += $(STELLARIS_DRIVER_DIR)/EVB_9B95/drivers/set_pinout.c
endif

LIBS =

#####################################################################

OBJS := $(addprefix $(BUILD_DIR)/, $(SRC:.c=.o))
CFLAGS += $(addprefix -I, $(INC))

DEPDIR = $(BUILD_DIR)/.d
$(shell mkdir -p $(DEPDIR) > /dev/null)
DEPFLAGS = -MT $@ -MMD -MP
POSTCOMPILE = mkdir -p $(DEPDIR)/$(dir $*.d) && mv -f $(BUILD_DIR)/$*.d $(DEPDIR)/$*.d

.PHONY: clean astyle

all : $(BIN_NAME).bin ;

astyle :
	astyle --options=astyle.conf "$(DCON_PROJ_DIR)/*.c, *.h"

$(BIN_NAME).bin : $(BIN_NAME).axf
	@mkdir -p $(dir $@)
	$(OBJCOPY) $< -O binary $@

$(BIN_NAME).axf : $(LDSCRIPT)

$(BIN_NAME).axf : $(OBJS) $(BUILD_DIR)/src/startup.o
	@mkdir -p $(dir $@)
	$(CC) $(CFLAGS) $^ $(LIBS) $(LDFLAGS) -o $@
	
$(BUILD_DIR)/src/startup.o : $(BUILD_DIR)/%.o : %.c $(DEPDIR)/%.d
	@mkdir -p $(dir $@)
	$(CC) $(CFLAGS) $(DEPFLAGS) -O1 -c $< -o $@
	$(POSTCOMPILE)

$(OBJS) : $(BUILD_DIR)/%.o : %.c
	@mkdir -p $(dir $@)
	$(CC) $(CFLAGS) $(DEPFLAGS) -c $< -o $@
	$(POSTCOMPILE)

$(DEPDIR)/%.d : ;

-include $(patsubst %,$(DEPDIR)/%.d,$(basename $(SRC)))

clean :
	rm -rf $(DEPDIR) $(BUILD_DIR) $(OBJS) $(PROJ_PATH).bin $(PROJ_PATH).map \
		   $(PROJ_PATH).axf $(BUILD_DIR)/startup.o
