targets := libuthread.a
objs    := uthread.o queue.o preempt.o context.o

CC      := gcc
CFLAGS  := -Wall -Wextra -Werror -MMD
CFLAGS  += -g

ifneq ($(V)),1)
Q = @
endif

all: $(targets)

deps := $(patsubst %.o, %.d, $(objs))
-include $(deps)

libuthread.a: $(objs)
	@echo "CC $@"
	$(Q)$(CC) $(CFLAGS) -o $@ $^
%.o: %.c
	@echo "CC $@"
	$(Q)$(CC) $(CFLAGS) -c -o $@ $<

clean:
	@echo "clean"
	$(Q)rm -f $(targets) $(objs) $(deps)
