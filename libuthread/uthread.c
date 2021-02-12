#include <assert.h>
#include <limits.h>
#include <signal.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/time.h>

#include "private.h"
#include "uthread.h"
#include "queue.h"

uthread_t cur_tid;
struct thread {
	uthread_t tid;
	int state; //0: ready, 1: running, 2: blocked, 3: zombie, 4: died
	void *stack;
	uthread_ctx_t context;
	int return_value; //return value of this thread
	int join_tid; //-1:no joined thread
	int *retval; //address of return value of its joined thread
};

int has_preempt = 0; //0: no preempt, 1: has preempt
queue_t ready_queue;
queue_t finish_queue;	//zombie and dead thread
queue_t blocked_queue;
struct thread* cur_thread;

//Find an item given tid and return its address
static struct thread* find_item(queue_t queue, uthread_t tid)
{
	int len = queue_length(queue);
	struct thread *find_pointer = NULL;
	while (len --)
	{
		struct thread *pointer;
		queue_dequeue(queue, (void**)&pointer);
		if(pointer->tid == tid && find_pointer == NULL)
        {
			find_pointer = pointer;
        }
		queue_enqueue(queue, pointer);
	}
	return find_pointer;
}

//Iterate through block_queue and unblock the items that are waiting for given thread to return.
static int unblock_thread(queue_t q, void *data, void *arg) 
{
	struct thread *a = (struct thread*) data;
	if (a->join_tid == ((struct thread *)(long) arg)->tid)
	{
		//Delete the item from block_queue and add it to ready_queue
		queue_delete(q, data);
		a->state = 0;
		a->join_tid = -1;
        if(a->retval != NULL)
            *(a->retval) = ((struct thread *)(long) arg)->return_value;
		queue_enqueue(ready_queue, data);
	}
	return 0;
}

static int destory_thread(struct thread* pt_thread)
{
	//Make sure that all threads finish regardless of whether return values are collected 
	if (pt_thread->state == 3 || pt_thread->state == 4)
	{
		uthread_ctx_destroy_stack(pt_thread->stack);
		free(pt_thread);
		return 0;
	}
	return -1;
}

int uthread_start(int preempt)
{	
	//Create the queues.
	ready_queue = queue_create();
	finish_queue = queue_create();
	blocked_queue = queue_create();	
	if (ready_queue == NULL || finish_queue == NULL || blocked_queue == NULL)
		return -1;
	
	//Create the pointer to main thread and initialize it.
	cur_tid = 0;
	struct thread *main_thread = (struct thread*)malloc(sizeof(struct thread));
	if (!main_thread)
		return -1;
	main_thread->tid = cur_tid;
	main_thread->state = 1;
	
	//Current thread is main thread.
	cur_thread = main_thread;
	
	if (preempt)
	{
		has_preempt = 1;
		preempt_start();
	}

	return 0;
}

int uthread_stop(void)
{	
	//Stop the preempt.
	if (has_preempt)
		preempt_stop();

	//Destory the queues.
	if (queue_destroy(ready_queue) == -1)
		return -1;
	if (queue_destroy(blocked_queue) == -1)
		return -1;
	cur_tid = 0;
	
	//Destroy each item in finish_queue and destroy the queue.
	while (queue_length(finish_queue) > 0)
	{
		struct thread *tcb;
		queue_dequeue(finish_queue, (void**)&tcb);
		destory_thread(tcb);
	}
	if (queue_destroy(finish_queue) == -1)
		return -1;
	destory_thread(cur_thread);
		
	return 0;
}

int uthread_create(uthread_func_t func)
{
	if(cur_tid == USHRT_MAX)
		return -1;
	
	//Create the new thread and initialize it.
	struct thread *new_thread = (struct thread*)malloc(sizeof(struct thread));
	if(!new_thread)
		return -1;
	new_thread->tid = ++ cur_tid;
	new_thread->state = 0;
	new_thread->stack = uthread_ctx_alloc_stack();
	new_thread->return_value = 0;
	new_thread->join_tid = -1;		
	if(uthread_ctx_init(&new_thread->context, new_thread->stack, func) == -1)
	{
		cur_tid --;
		return -1;
	}
	
	//Put the new thread into the ready_queue.
	if(queue_enqueue(ready_queue, new_thread) == -1)
		return -1;

	return new_thread->tid;
}

void uthread_yield(void)
{
	//Change the current thread state to ready and add it to the ready_queue.
	cur_thread->state = 0;
	queue_enqueue(ready_queue, cur_thread);
	
	//Current thread becomes the first (oldest) item in the ready_queue.
	struct thread *next_tcb;
	queue_dequeue(ready_queue, (void**)&next_tcb);
	struct thread *prev_tcb = cur_thread;
	cur_thread = next_tcb;
    cur_thread->state = 1;
	
	if (has_preempt)
		preempt_disable();
	uthread_ctx_switch(&(prev_tcb->context), &(next_tcb->context));
}

uthread_t uthread_self(void)
{
	return cur_thread->tid;
}

void uthread_exit(int retval)
{
	//Change the current thread state to zombie and store its return value.
	cur_thread->state = 3;
	cur_thread->return_value = retval;
    
    //If blocked_queue is empty, no thread is waiting. Then just put it into the finish queue.
	int blocked_queue_length = queue_length(blocked_queue);
	if(blocked_queue_length > 0)
		queue_iterate(blocked_queue, unblock_thread, (void*)(&(cur_thread->tid)), NULL);

	queue_enqueue(finish_queue, cur_thread);
	
	//Schedule new thread.
	struct thread *next_tcb;
	queue_dequeue(ready_queue, (void**)&next_tcb);
	
	struct thread *prev_tcb = cur_thread;
	cur_thread = next_tcb;
	cur_thread->state = 1;
	
	if (has_preempt)
		preempt_disable();
	uthread_ctx_switch(&(prev_tcb->context), &(next_tcb->context));
}

int uthread_join(uthread_t tid, int *retval)
{
	//Change the current thread state to zombie and update join_tid.
	cur_thread->state = 2;
	cur_thread->join_tid = tid;
	
	//Find whether the thread it is waiting for has finished.
	struct thread *find_pointer = find_item(finish_queue, tid);
	
	//If the thread is finished, then collect its return value;
	//Otherwise, add it to the blocked_queue.
	if(find_pointer)
	{
		cur_thread->retval = retval;

        if(retval != NULL)
            *retval = find_pointer->return_value;
		cur_thread->state = 0;
		
		//Update join_tid, since it does not need to wait.
		cur_thread->join_tid = -1;

		queue_enqueue(ready_queue, cur_thread);
		struct thread *next_tcb;
		queue_dequeue(ready_queue, (void**)&next_tcb);
	
		struct thread *prev_tcb = cur_thread;
		cur_thread = next_tcb;
		cur_thread->state = 1;
		
		if (has_preempt)
			preempt_disable();
		uthread_ctx_switch(&(prev_tcb->context), &(next_tcb->context));
		
		return 0;
	}
	else
		queue_enqueue(blocked_queue, cur_thread);
	
	//Search the thread in the ready_queue. If not found, then there is an error.
	find_pointer = find_item(ready_queue, tid);
	if(find_pointer)
	{
		cur_thread->join_tid = (int) find_pointer->tid;
		cur_thread->retval = retval;
		
		struct thread *next_tcb;

		queue_dequeue(ready_queue, (void**)&next_tcb);
		
		struct thread *prev_tcb = cur_thread;
		cur_thread = next_tcb;
		cur_thread->state = 1;
		
		if (has_preempt)
			preempt_disable();
		uthread_ctx_switch(&(prev_tcb->context), &(next_tcb->context));
		return 0;
	}
	return -1;
}
