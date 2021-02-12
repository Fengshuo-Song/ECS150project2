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
	int state; //0:ready, 1:running, 2:blocked, 3:zombie, 4:died
	void *stack;
	uthread_ctx_t context;
	int return_value;
	int join_tid; //-1:no join thread
	int *retval;
};

int has_preempt = 0; //0: no preempt; 1: has preempt
queue_t ready_queue;
queue_t finish_queue;	//Zombie and dead thread
queue_t blocked_queue;
struct thread* cur_thread;

static struct thread* find_item(queue_t queue, uthread_t tid)
{
	int len = queue_length(queue);
	struct thread *find_pointer = NULL;
	while(len --)
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

static int unblock_thread(queue_t q, void *data, void *arg) 
{
	struct thread *a = (struct thread*) data;
	if(a->join_tid == ((struct thread *)(long) arg)->tid) //////!!!!!!!!!!!!!!!!
	{
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
	if(pt_thread->state == 4)
	{
		uthread_ctx_destroy_stack(pt_thread->stack);
		free(pt_thread);
		return 0;
	}
	return -1;
}

int uthread_start(int preempt)
{
	
	if(preempt)
	{
		has_preempt = 1;
		preempt_start();
	}

	ready_queue = queue_create();
	finish_queue = queue_create();
	blocked_queue = queue_create();	
	if(ready_queue == NULL || finish_queue == NULL || blocked_queue == NULL)
		return -1;

	cur_tid = 0;
	
	struct thread *main_thread = (struct thread*)malloc(sizeof(struct thread));
	if(!main_thread)
		return -1;

	main_thread->tid = cur_tid;
	main_thread->state = 1;

	cur_thread = main_thread;
	
	return 0;
}

int uthread_stop(void)
{
	if(queue_destroy(ready_queue) == -1)
		return -1;
	if(queue_destroy(blocked_queue) == -1)
		return -1;
	cur_tid = 0;

	while(queue_length(finish_queue) > 0)
	{
		struct thread *tcb;
		queue_dequeue(finish_queue, (void**)&tcb);
		destory_thread(tcb);
	}
	if(queue_destroy(finish_queue) == -1)
		return -1;
	
	destory_thread(cur_thread);
	
	if(has_preempt)
		preempt_stop();
		
	return 0;
}

int uthread_create(uthread_func_t func)
{
	if(cur_tid == USHRT_MAX)
		return -1;

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
	
	if(queue_enqueue(ready_queue, new_thread) == -1)
		return -1;

	return new_thread->tid;
}

void uthread_yield(void)
{
	cur_thread->state = 0;
	queue_enqueue(ready_queue, cur_thread);
	
	struct thread *next_tcb;
	queue_dequeue(ready_queue, (void**)&next_tcb);
	
	struct thread *prev_tcb = cur_thread;
	cur_thread = next_tcb;
    cur_thread->state = 1;

	uthread_ctx_switch(&(prev_tcb->context), &(next_tcb->context));
}

uthread_t uthread_self(void)
{
	return cur_thread->tid;
}

void uthread_exit(int retval)
{
	cur_thread->state = 3;
	cur_thread->return_value = retval;
    
	int blocked_queue_length = queue_length(blocked_queue);
	if(blocked_queue_length > 0)
	{
		queue_iterate(blocked_queue, unblock_thread, (void*)(&(cur_thread->tid)), NULL);

	}

	queue_enqueue(finish_queue, cur_thread);
	
	//Schedule new thread
	//assert ready_queue not empty
	struct thread *next_tcb;
	queue_dequeue(ready_queue, (void**)&next_tcb);
	
	struct thread *prev_tcb = cur_thread;
	cur_thread = next_tcb;
	cur_thread->state = 1;
	uthread_ctx_switch(&(prev_tcb->context), &(next_tcb->context));
}

int uthread_join(uthread_t tid, int *retval)
{
	cur_thread->state = 2;
	cur_thread->join_tid = tid;
	
	struct thread *find_pointer = find_item(finish_queue, tid);
	
	if(find_pointer) //uthread_join,finish_queue
	{
		cur_thread->retval = retval;

        if(retval != NULL)
            *retval = find_pointer->return_value;
		cur_thread->state = 0;
		cur_thread->join_tid = -1;

		queue_enqueue(ready_queue, cur_thread);
		struct thread *next_tcb;
		queue_dequeue(ready_queue, (void**)&next_tcb);
	
		struct thread *prev_tcb = cur_thread;
		cur_thread = next_tcb;
		cur_thread->state = 1;
		uthread_ctx_switch(&(prev_tcb->context), &(next_tcb->context));
		
		return 0;
	}
	else
		queue_enqueue(blocked_queue, cur_thread);
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
		uthread_ctx_switch(&(prev_tcb->context), &(next_tcb->context));
		return 0;
	}
	return -1;
}
