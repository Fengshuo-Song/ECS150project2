#include <stdio.h>
#include <stdlib.h>

#include "uthread.h"

/***********************test_hello***************************/
int hello(void)
{
	printf("Hello world!\n");
	return 0;
}

void test_hello(void)
{
	printf("*** TEST hello ***\n");
	printf("==Expected Output:\n");
	printf("Hello world!\n");
	printf("==Actual Output:\n");

	uthread_t tid;

	uthread_start(0);
	tid = uthread_create(hello);
	uthread_join(tid, NULL);
	uthread_stop();
}
/***********************test_hello***************************/

/***********************test_yield***************************/
int thread3(void)
{
	uthread_yield();
	printf("thread%d\n", uthread_self());
	return 0;
}

int thread2(void)
{
	uthread_create(thread3);
	uthread_yield();
	printf("thread%d\n", uthread_self());
	return 0;
}

int thread1(void)
{
	uthread_create(thread2);
	uthread_yield();
	printf("thread%d\n", uthread_self());
	uthread_yield();
	return 0;
}

void test_yield(void)
{
	printf("*** TEST yield ***\n");
	printf("==Expected Output:\n");
	printf("thread1\nthread2\nthread3\n");
	printf("==Actual Output:\n");
	
	uthread_start(0);
	uthread_join(uthread_create(thread1), NULL);
	uthread_stop();

}
/***********************test_yield***************************/

/***********************test_return1*************************/
//This part tests whether one thread can collect the return value of another thread.
//When one thread joins another thread, the thread that is being joined has finished. 
int thread_A(void)
{
	printf("Thread A ends and returns 123.\n");
	return 123;
}

void test_return1(void)
{
	printf("*** TEST simple return 1 ***\n");
	printf("==Expected Output:\n");
	printf("Thread A ends and returns 123.\n");
	printf("Main thread joins A.\n");
	printf("Thread A returns 123.\n");
	printf("==Actual Output:\n");
	
	uthread_t tid;
	int ret_A = 100;
    
	uthread_start(0);
	uthread_yield();
	tid = uthread_create(thread_A);
	uthread_yield();
	printf("Main thread joins A.\n");
	uthread_join(tid, &ret_A);
	uthread_stop();
	printf("Thread A returns %d.\n",ret_A);

}
/***********************test_return1*************************/

/***********************test_return2*************************/
//This part tests another scenario of joining and the return value.
//When one thread joins another thread, the thread that is being joined has finished. 
void test_return2(void)
{
	printf("*** TEST simple return 2 ***\n");
	printf("==Expected Output:\n");
	printf("Main thread joins A.\n");
	printf("Thread A ends and returns 123.\n");
	printf("Thread A returns 123.\n");
	printf("==Actual Output:\n");
	
	uthread_t tid;
	int ret_A = 100;
    
	uthread_start(0);
	uthread_yield();
	tid = uthread_create(thread_A);
	printf("Main thread joins A.\n");
	uthread_join(tid, &ret_A);
	uthread_yield();
	uthread_stop();
	printf("Thread A returns %d.\n",ret_A);

}
/***********************test_return2*************************/

/*********************test_return_complex********************/
//This tests joining and return values with more threads.
int helloA(void)
{
	printf("A: Hello world!\n");
	return 1;
}

int helloB(void)
{
	printf("B1: Hello world!\n");
	uthread_yield();
	printf("B2: Hello world!\n");
	uthread_yield();
	return 2;
}

int helloC(void)
{
	printf("C1: Hello world!\n");
	uthread_yield();
	printf("C2: Hello world!\n");
	uthread_yield();
	uthread_exit(666);
	return 3;
}

void test_return_complex(void)
{
	printf("*** TEST complex return ***\n");
	printf("==Expected Output:\n");
	printf("A: Hello world!\nB1: Hello world!\nC1: Hello world!\n");
	printf("B2: Hello world!\nC2: Hello world!\n");
	printf("Thread helloA returns 1.\n");
	printf("Thread helloC returns 666.\n");
	printf("==Actual Output:\n");
	
	uthread_t tidA;
	uthread_t tidC;
	
	int ret_A = 100;
	int ret_C = 200;
	uthread_start(0);
	uthread_yield();
	tidA = uthread_create(helloA);
	uthread_yield();
	uthread_create(helloB);
	tidC = uthread_create(helloC);
	
	uthread_join(tidA, &ret_A);
	uthread_yield();
	uthread_join(tidC, &ret_C);
	
	uthread_stop();
	
	printf("Thread helloA returns %d.\n",ret_A);
	printf("Thread helloC returns %d.\n",ret_C);

}
/*********************test_return_complex********************/

int main(void)
{
	test_hello();
	printf("********************\n");
	test_yield();
	printf("********************\n");
	test_return1();
	printf("********************\n");
	test_return2();
	printf("********************\n");
	test_return_complex();

	return 0;
}
