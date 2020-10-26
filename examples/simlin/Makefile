vpath %.c ../../src
vpath %.h ../..inc . ../../flsemu

xcp_sim_SRC = xcp.c xcp_checksum.c xcp_daq.c xcp_util.c hw/linux/hw.c hw/linux/tui.c app_config.c main.c ../flsemu/posix/flsemu.c ../flsemu/common.c

INCLUDE= -I . -I ../../inc -I ../../flsemu
CPPFLAGS += $(INCLUDE)

CFLAGS +=  -std=gnu11 -Wall -Wextra -Wno-unused-parameter -Wno-unused-function  -ffunction-sections
LDFLAGS=  -Wl,-Map=$(xcp_sim_TARGET).map -Wl,--cref -Wl,-gc-sections -lpthread -lrt -lncurses
ifeq ($(PROFILE),1)
CFLAGS += -pg -fprofile-arcs -ftest-coverage --coverage
endif
ifeq ($(DEBUG),1)
CFLAGS += -ggdb -O2 -D_FORTIFY_SOURCE=2 -fstack-protector-all --param=ssp-buffer-size=4 -fstack-check
else
CFLAGS += -O3
endif

#ifeq ($(strip $(TARGET)), kvaser)
#KV_BASE_DIR = f:/kvaser/Canlib
#xcp_sim_SRC+=tl/can/kvaser.c
#xcp_sim_TARGET=xcp_sim_kvaser
#INCLUDE+=-I $(KV_BASE_DIR)/INC
#LDFLAGS+=-L $(KV_BASE_DIR)/Lib/MS -l canlib32
#CPPFLAGS += -D KVASER_CAN
ifeq ($(strip $(TARGET)), socket_can)
xcp_sim_SRC+=tl/can/linux_socket_can.c
xcp_sim_TARGET=xcp_sim_socket_can
CPPFLAGS += -D SOCKET_CAN
else
CPPFLAGS += -D ETHER
xcp_sim_SRC+=tl/eth/linuxeth.c
xcp_sim_TARGET=xcp_sim_eth
endif

xcp_sim_OBJS = $(xcp_sim_SRC:%.c=%.o)

all: $(xcp_sim_TARGET)

xcp_sim_OBJS: xcp.h xcp_hw.h xcp_macros.h xcp_types.h xcp_ow.h xcp_util.h xcp_config.h #flsemu.h

%.o: %.c
	$(CC) $(CPPFLAGS) $(CFLAGS) -c $< -o $(notdir $@)

$(xcp_sim_TARGET): $(xcp_sim_OBJS)
	@echo Linking $(xcp_sim_TARGET)
	$(CC) $(CFLAGS) -o $(xcp_sim_TARGET) $(notdir $(xcp_sim_OBJS)) $(LDFLAGS)

clean:
	@rm -f $(xcp_sim_OBJS) $(xcp_sim_TARGET).exe

.PHONY: clean all

