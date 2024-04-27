#include <stdio.h>
#include <errno.h>
#include <stdlib.h>
#include <string.h>

#define DIE(assertion, call_description)			\
do {								\
	if (assertion) {					\
		fprintf(stderr, "(%s, %d): ",			\
				__FILE__, __LINE__);		\
		perror(call_description);			\
		exit(errno);				        \
	}							\
} while (0)

#define MAX_STRING_SIZE 64

/* -------------------------- QUEUE IMPLEMENTATION ---------------------------*/

typedef struct queue_t queue_t;
struct queue_t {
	/* Dimensiunea maxima a cozii */
	unsigned int max_size;
	/* Dimensiunea cozii */
	unsigned int size;
	/* Dimensiunea in octeti a tipului de date stocat in coada */
	unsigned int data_size;
	/* Indexul de la care se vor efectua operatiile de front si dequeue */
	unsigned int read_idx;
	/* Indexul de la care se vor efectua operatiile de enqueue */
	unsigned int write_idx;
	/* Bufferul ce stocheaza elementele cozii */
	void **buff;
};

/*
 * @brief   Functia creeaza si intializeaza o coada goala
 * @return  (queue_t)ptr_to_the_new_empty_queue
 */
queue_t *
q_create(unsigned int data_size, unsigned int max_size)
{
	queue_t *queue = malloc(sizeof(queue_t));
	DIE(queue, "Malloc failed\n");

	queue->size = 0;
	queue->read_idx = -1; // U_INT_MAX
	queue->write_idx = -1;
	queue->max_size = max_size;
	queue->data_size = data_size;
	
	queue->buff = calloc(queue->max_size, sizeof(void *));
	DIE(queue->buff, "Calloc failed\n");

	return queue;
}

/*
 * @brief   Functia intoarce numarul de elemente din coada
 * @return  (unsigned int)no_elements
 */
unsigned int
q_get_size(queue_t *q)
{
	return q->size;
}

/*
 * @brief   Functia determina daca stiva este goala sau nu
 * @return  1 = goala / 0 = are elemente in ea
 */
unsigned int
q_is_empty(queue_t *q)
{
	return (q->size == 0);
}

/* 
 * @brief   Functia intoarce primul element din coada, fara sa il elimine.
 */
void *
q_front(queue_t *q)
{
	return (void *)((char *)q->buff + q->read_idx * q->data_size);
}

/*
 * @brief   Functia scoate un element din coada
 * @return  1 = success / 0 = fail
 */
int
q_dequeue(queue_t *q)
{
	if (q->read_idx == (unsigned int)-1) {
		printf("Queue is empty\n");
		return 0;
	}

	if (q->read_idx == q->write_idx) {
		// Stack is now empty
		q->read_idx = -1;
		q->write_idx = -1;
	} else {
		q->read_idx++;
		q->read_idx %= q->max_size;
	}

	q->size--;

	return 1;
}

/* 
 * @brief   Functia introduce un nou element in coada
 * @return  1 = success / 0 = fail
 */
int
q_enqueue(queue_t *q, void *new_data)
{
	if (q->size == q->max_size) {
		printf("Queue is full\n");
		return 0;
	}

	q->write_idx++;
	q->write_idx %= q->max_size;

	if (q->read_idx == (unsigned int)-1)
		q->read_idx = 0;

	void *elem_to_write = (char *)q->buff + q->write_idx * q->data_size;
	memcpy(elem_to_write, new_data, q->data_size);

	q->size++;

	return 1;
}

/*
 * @brief   Functia elimina toate elementele din coada primita ca parametru.
 */
void
q_clear(queue_t *q)
{
	for (unsigned int i = 0; i < q->max_size; ++i) {
		void __attribute__((unused)) *curr_elem =
			(char *)q->buff + i * q->data_size;
		curr_elem = NULL;
	}
}

/*
 * @brief   Functia elibereaza toata memoria ocupata de coada.
 */
void
q_free(queue_t *q)
{
	free(q->buff);
}

