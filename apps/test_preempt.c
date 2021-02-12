/*
The output of this program is as follows:
Begin thread 1.
Begin thread 2.
Thread 1:1
Thread 2:1
Thread 1:2
Thread 1:3
Thread 2:2
Thread 1:4
Thread 2:3
Thread 1:5
Thread 1:6
Thread 2:4
Thread 1:7
Thread 2:5
Thread 1:8
Thread 1:9
Thread 2:6
Thread 1:10
End thread 1
*/

//The whole program does not call uthread_yield function, but the output 
//shows that it keeps switching between different threads. These switches 
//are completed by the timer interrupt, which is preemptive. Also, since 
//only thread 1 is joined by the main thread, thread 2 may not terminate
//when the main thread terminates.

#include <stdio.h>
#include <stdlib.h>
#include "uthread.h"

//The purpose to allow the thread run slower, so that it can be interrupted by the timer.
#define DELAY 100000000

//Both threads prints from 1 to 10 slowly.
int test_thread_1(void);
int test_thread_2(void);

int test_thread_1(void)
{
	printf("Begin thread 1.\n");
	uthread_create(test_thread_2);

	int i,j;
	for(i = 1;i <= 10; i++)
	{
		for(j = 1;j < DELAY; j++);
		printf("Thread 1:%d\n",i);
	}
	
    printf("End thread 1\n");
    return 0;
}

int test_thread_2(void)
{
	printf("Begin thread 2.\n");
	int i,j;
	for(i = 1;i <= 10;i++)
	{
		for(j = 1;j < DELAY; j++);
		printf("Thread 2:%d\n",i);
	}
	
    printf("End thread 2\n");
    return 0;
}

int main(void)
{
	uthread_t tid;

	uthread_start(1);
	tid = uthread_create(test_thread_1);
	uthread_join(tid, NULL);
	uthread_stop();

	return 0;
}
