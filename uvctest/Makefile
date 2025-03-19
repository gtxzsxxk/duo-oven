export CFLAGS="-mcpu=c906fdv -march=rv64imafdcv0p7xthead -mcmodel=medany -mabi=lp64d"

CROSS_COMPILE	?= $(CROSS_COMPILE)

CC := $(CROSS_COMPILE)gcc
SRC = $(wildcard *.c)
BIN:=$(SRC:.c=)

CFLAGS		:= -W -Wall -g
LDFLAGS		:= -static
# LDFLAGS		:= -lpthread

all: $(BIN)

$(BIN): % : %.c
	$(CC) $(LDFLAGS) $(CFLAGS) -o $@ $^
	cp uvctest ../device/milkv-duo-sd/overlay/root/oven_ctl/camera/uvcgrab

clean:
	rm -f *.o $(BIN)
