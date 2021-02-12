# Target library
lib := libuthread.a


all: libuthread.a

libuthread.a: uthread.o queue.o preempt.o context.o
	gcc -Wall -Wextra -Werror -o $@ $^
uthread.o: uthread.c uthread.h private.h queue.h
	gcc -Wall -Wextra -Werror -c -o $@ $<
queue.o: queue.c queue.h
	gcc -Wall -Wextra -Werror -c -o $@ $<
preempt.o: preempt.c private.h
	gcc -Wall -Wextra -Werror -c -o $@ $<
context.o: context.c private.h
	gcc -Wall -Wextra -Werror -c -o $@ $<

clean:
	rm -f libuthread.a main.o uthread.o queue.o preempt.o context.o