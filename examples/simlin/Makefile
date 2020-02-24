vpath %.c ../../src
vpath %.h ../..inc . ../../flsemu

xcp_sim_SRC = xcp.c xcp_checksum.c xcp_daq.c xcp_util.c hw/linux/hw.c  main.c #../flsemu/flsemu.c

INCLUDE= -I . -I ../../inc -I #../../flsemu
CPPFLAGS += $(INCLUDE)

CFLAGS +=  -Wall -Wextra -Wno-unused-parameter -Wno-unused-function  -ffunction-sections
LDFLAGS=  -Wl,-Map=$(xcp_sim_TARGET).map -Wl,--cref -Wl,-gc-sections -lpthread
ifeq ($(PROFILE),1)
CFLAGS += -pg -fprofile-arcs -ftest-coverage --coverage
endif
ifeq ($(DEBUG),1)
CFLAGS += -g -O0
else
CFLAGS += -O3
endif

ifeq ($(strip $(TARGET)), kvaser)
KV_BASE_DIR = f:/kvaser/Canlib
xcp_sim_SRC+=tl/can/kvaser.c
xcp_sim_TARGET=xcp_sim_kvaser
INCLUDE+=-I $(KV_BASE_DIR)/INC
LDFLAGS+=-L $(KV_BASE_DIR)/Lib/MS -l canlib32
CPPFLAGS += -DKVASER_CAN
else
xcp_sim_SRC+=tl/eth/linuxeth.c
#xcp_sim_SRC+=tl/eth/wineth_iocp.c
xcp_sim_TARGET=xcp_sim
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
	rm -f $(xcp_sim_OBJS) $(xcp_sim_TARGET).exe

.PHONY: clean all

