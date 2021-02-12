#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include "queue.h"

//Each item in the queue is a node that contains data and a pointer to the next item.
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
	
	//Allocating space for queue is unsuccessful.
	if (!pt_queue)	
		return NULL;
	
	//pt_queue->front points to the head node.
	pt_queue->front = (struct node *)malloc(sizeof(struct node));
	
	//Allocating space for head node is unsuccessful.
	if (!pt_queue->front)
	{
		free(pt_queue);
		return NULL;
	}
	
	//Initialize the queue and head node.
	pt_queue->front->next = NULL;
	pt_queue->rear = pt_queue->front;
	pt_queue->length = 0;
	
	return pt_queue;
}

int queue_destroy(queue_t queue)
{
	//The queue address does not exist or is not empty.
	if (queue == NULL || queue->length != 0)
		return -1;

	free(queue->front);
	free(queue);
	
	return 0;
}

int queue_enqueue(queue_t queue, void *data)
{
	//The queue or data address does not exist.
	if (!queue || !data)
		return -1;
	
	//Allocate space for the newly added node.
	struct node* pt_node = (struct node*)malloc(sizeof(struct node));
	if (!pt_node)
		return -1;

	//Add the node to the end of the queue.
	pt_node->data = data;
	pt_node->next = NULL;
	queue->rear->next = pt_node;
	queue->rear = pt_node;
	queue->length ++;

	return 0;
}

int queue_dequeue(queue_t queue, void **data)
{
	//The queue address does not exist or is empty.
	if (!queue || queue->length == 0)
		return -1;
	
	//Get the first (oldest) node of the queue.
	struct node* pt_node = queue->front->next;
	*data = pt_node->data;

	//Delete the node.
	queue->front->next = queue->front->next->next;
    if (queue->rear == pt_node) 
		queue->rear = queue->front;
	queue->length --;
	free(pt_node);
	
	return 0;
}

int queue_delete(queue_t queue, void *data)
{
	//The queue or data address does not exist or the queue is empty.
	if (!queue || !data || queue->length == 0)
		return -1;
	
	//Get the head and the first (oldest) node in the queue.
	struct node* pt_head = queue->front;
	struct node* pt_next = pt_head->next;
	
	//Iterate each node until data is found.
	while (pt_next != NULL && pt_next->data != data)
	{
		pt_head = pt_next;
		pt_next = pt_next->next;
	}

	//Data is found.	
	if (pt_next != NULL)
	{
		//Delete the node
		pt_head->next = pt_head->next->next;
		if (pt_next == queue->rear)
			queue->rear = pt_head;
		free (pt_next);
		queue->length --;
		
		return 0;
	}
	else
		return -1;
}

int queue_iterate(queue_t queue, queue_func_t func, void *arg, void **data)
{
	//The queue address or function does not exist
	if (!queue || !func)
		return -1;
	
	//If the queue is empty, there is no need to do anything.
	if (queue_length(queue) == 0)
		return 0;
	
	//Get the head and the first and second (oldest and second oldest) node in the queue.
	struct node* pt_node = queue->front->next;
	struct node* pt_node_next = pt_node->next;

	//Iterate each item.
	while (pt_node != NULL)
	{
		int return_value = (*func)(queue, pt_node->data, arg);
		if (return_value == 1 && data != NULL)
		{
			*data = pt_node->data;
			return 0;
		}
		pt_node = pt_node_next;
		if (pt_node_next != NULL)
			pt_node_next = pt_node_next->next;
	}
	return 0;
}

int queue_length(queue_t queue)
{
	//The queue address does not exist.
	if (!queue)
		return -1;

	return queue->length;
}

