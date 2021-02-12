#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include "queue.h"

struct node {
	void *data;
	struct node *next;
};

struct queue {
	struct node * front;
	struct node * rear;
	int length;
};
queue_t queue_create(void)
{
	queue_t pt_queue;
	pt_queue = (queue_t)malloc(sizeof(struct queue));
	
	//Allocating space is successful.
	if(!pt_queue)	
		return NULL;
		
	pt_queue->front = (struct node *)malloc(sizeof(struct node));
	
	if(!pt_queue->front)
	{
		free(pt_queue);
		return NULL;
	}
	
	pt_queue->front->next = NULL;
	pt_queue->rear = pt_queue->front;
	pt_queue->length = 0;
	return pt_queue;
}

int queue_destroy(queue_t queue)
{
	if(queue == NULL || queue->length != 0)
		return -1;

	free(queue->front);
	free(queue);
	
	return 0;
}

int queue_enqueue(queue_t queue, void *data)
{

	if(!queue || !data)
    {
		return -1;
    }

	struct node* pt_node = (struct node*)malloc(sizeof(struct node));
	if(!pt_node)
		return -1;
	pt_node->data = data;
	pt_node->next = NULL;
    
	queue->rear->next = pt_node;
	queue->rear = pt_node;
	queue->length ++;

	return 0;
}

int queue_dequeue(queue_t queue, void **data)
{
	if(!queue || queue->length == 0)
		return -1;
		
	struct node* pt_node = queue->front->next;
	*data = pt_node->data;

	queue->front->next = queue->front->next->next;
    if(queue->rear == pt_node) 
			queue->rear = queue->front;
	queue->length --;
	free(pt_node);
	return 0;
}

int queue_delete(queue_t queue, void *data)
{
	if (!queue || !data || queue->length == 0)
		return -1;
	
	struct node* pt_fa = queue->front;
	struct node* pt_next = pt_fa->next;
	
	while (pt_next != NULL && pt_next->data != data)
	{
		pt_fa = pt_next;
		pt_next = pt_next->next;
	}
	
	if(pt_next != NULL)
	{
		pt_fa->next = pt_fa->next->next;
		if(pt_next == queue->rear)
			queue->rear = pt_fa;
		free(pt_next);
		queue->length --;
		return 0;
	}
	else
		return -1;
}

int queue_iterate(queue_t queue, queue_func_t func, void *arg, void **data)
{
	if(!queue || !func)
		return -1;
	
	//If the queue is empty, there is no need to do anything.
	if(queue_length(queue) == 0)
		return 0;

	struct node* pt_node = queue->front->next;
	struct node* pt_node_next = pt_node->next;

	while (pt_node != NULL)
	{
		int return_value = (*func)(queue, pt_node->data, arg);
		if(return_value == 1 && data != NULL)
		{
			*data = pt_node->data;
			return 0;
		}
		pt_node = pt_node_next;
		if(pt_node_next != NULL)
			pt_node_next = pt_node_next->next;
	}
	return 0;
}

int queue_length(queue_t queue)
{
	if(!queue)
		return -1;
	return queue->length;
}

