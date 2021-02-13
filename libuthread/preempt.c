#include <signal.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/time.h>

#include "private.h"
#include "uthread.h"

/*
 * Frequency of preemption
 * 100Hz is 100 times per second
 */
#define HZ 100

struct sigaction sa, original_sa;
struct itimerval new_time, original_time;
sigset_t sset;

void handler(int sig)
{
    switch(sig)
    {
        case SIGVTALRM:
            uthread_yield();
            break;
    }
}

void preempt_start(void)
{
	sigemptyset(&sset);
	
	//Install handler
	sa.sa_handler = handler;
	sigemptyset(&sa.sa_mask);
	sa.sa_flags = 0;
	sigaction(SIGVTALRM, &sa, &original_sa);
	sigaddset(&sset,SIGVTALRM);
	
	//Install timer
	new_time.it_interval.tv_usec = 10000;
	new_time.it_interval.tv_sec = 0;
	new_time.it_value.tv_usec = 10000;
	new_time.it_value.tv_sec = 0;
	setitimer(ITIMER_VIRTUAL, &new_time, &original_time);
}

void preempt_stop(void)
{
	sigaction(SIGVTALRM, &original_sa, NULL);
	setitimer(ITIMER_VIRTUAL, &original_time, NULL);
}

void preempt_enable(void)
{
	sigprocmask(SIG_UNBLOCK, &sset, NULL);
}

void preempt_disable(void)
{
	sigprocmask(SIG_BLOCK, &sset, NULL);
}

