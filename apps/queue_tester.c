#include <assert.h>
#include <stdio.h>
#include <stdlib.h>

#include "queue.h"

#define TEST_ASSERT(assert)				\
do {									\
	printf("ASSERT: " #assert " ... ");	\
	if (assert) {						\
		printf("PASS\n");				\
	} else	{							\
		printf("FAIL\n");				\
		exit(1);						\
	}									\
} while(0)

/* Create*/
void test_create(void)
{
	fprintf(stderr, "*** TEST create ***\n");
	queue_t q = queue_create();
	TEST_ASSERT(q != NULL);
	TEST_ASSERT(queue_destroy(q) == 0);
}

/* Enqueue/Dequeue simple */
void test_queue_simple(void)
{
	int data = 3, *ptr;
	queue_t q;

	fprintf(stderr, "*** TEST queue_simple ***\n");

	q = queue_create();
	queue_enqueue(q, &data);
	queue_dequeue(q, (void**)&ptr);
	TEST_ASSERT(ptr == &data);
	TEST_ASSERT(queue_destroy(q) == 0);
}

/* Enqueue/Dequeue complex */
void test_queue_complex(void)
{
	int data[] = {1,2,3,4,5}, *ptr = data, i;
	queue_t q;

	fprintf(stderr, "*** TEST queue_complex ***\n");

	q = queue_create();
	for (i = 0; i < 5; i++)
		queue_enqueue(q, &data[i]);

	for (i = 0; i < 5; i++)
		queue_dequeue(q, (void**)&ptr);

	TEST_ASSERT(ptr == &data[4]);
	TEST_ASSERT(queue_destroy(q) == 0);
}

/* Destroy 1 */
void test_destroy(void)
{
	queue_t q;

	fprintf(stderr, "*** TEST destroy ***\n");

	q = queue_create();
	int status = queue_destroy(q);
	TEST_ASSERT(status == 0);
}

/* Destroy 2 */
void test_destroy_abnormal(void)
{
	queue_t q;

	fprintf(stderr, "*** TEST fail to destroy ***\n");
	
	//Destroy a non-empty queue.
	q = queue_create();
	int data = 16;
	queue_enqueue(q, &data);
	TEST_ASSERT(queue_destroy(q) == -1);
	
	int *ptr;
	queue_dequeue(q, (void**)&ptr);
	TEST_ASSERT(queue_destroy(q) == 0);
}

/* Delete Front */
void test_delete_front(void)
{
	int data[] = {1,2,3,4,5}, i, status;
	queue_t q;

	fprintf(stderr, "*** TEST delete front ***\n");

	q = queue_create();
	for (i = 0; i < 5; i++)
		queue_enqueue(q, &data[i]);
	status = queue_delete(q, &data[0]);

	TEST_ASSERT(status == 0);
	
	int *ptr;
	while(queue_length(q) > 0)
		queue_dequeue(q, (void**)&ptr);
	TEST_ASSERT(queue_destroy(q) == 0);
}

/* Delete Interior */
void test_delete_interior(void)
{
	int data[] = {1,2,3,4,5}, i, status;
	queue_t q;

	fprintf(stderr, "*** TEST delete interior ***\n");

	q = queue_create();
	for (i = 0; i < 5; i++)
	{
		queue_enqueue(q, &data[i]);
	}
	status = queue_delete(q, &data[3]);
	TEST_ASSERT(status == 0);
	
	int *ptr;
	while(queue_length(q) > 0)
		queue_dequeue(q, (void**)&ptr);
	TEST_ASSERT(queue_destroy(q) == 0);
}

/* Delete Rear */
void test_delete_rear(void)
{
	int data[] = {1,2,3,4,5}, i, status;
	queue_t q;

	fprintf(stderr, "*** TEST delete rear ***\n");

	q = queue_create();
	for (i = 0; i < 5; i++)
	{
		queue_enqueue(q, &data[i]);
	}
	status = queue_delete(q, &data[4]);
	TEST_ASSERT(status == 0);
	
	int *ptr;
	while(queue_length(q) > 0)
		queue_dequeue(q, (void**)&ptr);
	TEST_ASSERT(queue_destroy(q) == 0);
}

/* Enqueue and dequeue with items of different types */
void test_different_types(void)
{
	queue_t q;

	fprintf(stderr, "*** TEST different types ***\n");

	q = queue_create();
	double data1 = 9.0;
	unsigned short data2 = 26;
	char data3 = 'c';
	
	queue_enqueue(q, &data1);
	queue_enqueue(q, &data2);
	queue_enqueue(q, &data3);
	
	TEST_ASSERT(queue_length(q) == 3);
	
	double *ptr1;
	queue_dequeue(q, (void**)&ptr1);
	TEST_ASSERT(*ptr1 == 9.0);
	
	unsigned short *ptr2;
	queue_dequeue(q, (void**)&ptr2);
	TEST_ASSERT(*ptr2 == 26);
	
	char *ptr3;
	queue_dequeue(q, (void**)&ptr3);
	TEST_ASSERT(*ptr3 == 'c');
	TEST_ASSERT(queue_destroy(q) == 0);
}

/* Callback function that increments integer items by a certain value (or delete
 * item if item is value 42) */
static int inc_item(queue_t q, void *data, void *arg)
{
	int *a = (int*)data;
	int inc = (int)(long)arg;

	if (*a == 42)
		queue_delete(q, data);
	else
		*a += inc;

	return 0;
}

/* Callback function that finds a certain item according to its value */
static int find_item(queue_t q, void *data, void *arg)
{
	int *a = (int*)data;
	int match = (int)(long)arg;
	(void)q;

	if (*a == match)
		return 1;

	return 0;
}

/* Iterate */
void test_iterator(void)
{
	queue_t q;
	int data[] = {1, 2, 3, 4, 5, 42, 6, 7, 8, 9};
	size_t i;
	int *ptr;

	fprintf(stderr, "*** TEST iterator ***\n");
	/* Initialize the queue and enqueue items */
	q = queue_create();
	for (i = 0; i < sizeof(data) / sizeof(data[0]); i++)
		queue_enqueue(q, &data[i]);

	/* Add value '1' to every item of the queue, delete item '42' */
	queue_iterate(q, inc_item, (void*)1, NULL);
	//queue_print(q);
	TEST_ASSERT(data[0] == 2);
	TEST_ASSERT(data[9] == 10);
	TEST_ASSERT(queue_length(q) == 9);

	/* Find and get the item which is equal to value '5' */
	ptr = NULL;     // result pointer *must* be reset first
	queue_iterate(q, find_item, (void*)5, (void**)&ptr);
	TEST_ASSERT(ptr != NULL);
	TEST_ASSERT(*ptr == 5);
	TEST_ASSERT(ptr == &data[3]);
	
	while(queue_length(q) > 0)
		queue_dequeue(q, (void**)&ptr);
	TEST_ASSERT(queue_destroy(q) == 0);
}

/* Iterate on an empty queue*/
void test_iterator_empty_queue(void)
{
	queue_t q;

	fprintf(stderr, "*** TEST iterator empty ***\n");
	/* Initialize the queue and enqueue items */
	q = queue_create();

	queue_iterate(q, inc_item, (void*)1, NULL);
	TEST_ASSERT(queue_length(q) == 0);
	TEST_ASSERT(queue_destroy(q) == 0);
}

int main(void)
{
	test_create();
	test_queue_simple();
	test_queue_complex();
	test_destroy();
	test_destroy_abnormal();
	test_delete_front();
	test_delete_interior();
	test_delete_rear();
	test_different_types();
	test_iterator();
	test_iterator_empty_queue();
	return 0;
}
