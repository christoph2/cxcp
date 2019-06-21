vpath %.c ../../src

PROGRAMS = xcp_sim

xcp_sim_SRC = xcp.c xcp_checksum.c xcp_daq.c xcp_util.c hw/win/hw.c tl/eth/wineth.c main.c ../flsemu/flsemu.c
xcp_sim_OBJS = $(xcp_sim_SRC:%.c=%.o)


xcp_sim_TARGET=xcp_sim

INCLUDE= -I . -I ../../inc -I ../../flsemu
CPPFLAGS += $(INCLUDE)

CFLAGS += -m32 -Wall -Wextra -Wno-unused-parameter -Wno-unused-function  -ffunction-sections
LDFLAGS= -lws2_32 -Wl,-Map=xcp_sim.map -Wl,--cref -Wl,-gc-sections
ifeq ($(PROFILE),1)
CFLAGS += -fprofile-arcs -ftest-coverage --coverage
endif
ifeq ($(DEBUG),1)
CFLAGS += -g -O0
else
CFLAGS += -O3
endif


all: $(PROGRAMS)

%.o: %.c
	$(CC) $(CPPFLAGS) $(CFLAGS) -c $< -o $(notdir $@)




xcp_sim: $(xcp_sim_OBJS)
	@echo Linking $(xcp_sim_TARGET)
	$(CC) $(CFLAGS) -o $(xcp_sim_TARGET) $(notdir $(xcp_sim_OBJS)) $(LDFLAGS)

clean:
	rm -vf *o *exe
#	rm -f $(xcp_sim_objs) $(PROGRAMS)

.PHONY: clean all


