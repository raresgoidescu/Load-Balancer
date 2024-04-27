/*
 * Copyright (c) 2024, GOIDESCU Rares-Stefan 312CA <known.as.rares@gmail.com>
 */

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "utils.h"

typedef struct q_node_t {
    /* Pointer catre urmatorul nod din lista */
    struct q_node_t *next;
    void *data;
} q_node_t;

typedef struct queue_t {
    /* Pointer catre primul element al listei */
    q_node_t *front;
    /* Pointer catre ultimul element al listei */
    q_node_t *rear;
    /* Numarul de octeti al unui element */
    unsigned int data_size;
    /* Numarul de elemente din coada */
    unsigned int size;
} queue_t;

queue_t *q_create(unsigned int data_size)
{
    queue_t *q = malloc(sizeof(*q));
    DIE(!q, "Malloc failed");

    q->front = NULL;
    q->rear = NULL;
    q->data_size = data_size;
    q->size = 0;

    return q;
}

void q_enqueue(queue_t *q, void *new_data)
{
    if (!q) return;
    
    q_node_t *new_node = calloc(1, sizeof(*new_node));
    DIE(!new_node, "Calloc failed");

    new_node->data = calloc(1, q->data_size);
    DIE(!new_node->data, "Calloc failed");

    memcpy(new_node->data, new_data, q->data_size);

    if (!q->rear) {
	    q->front = q->rear = new_node;
	    return;
    }

    q->rear->next = new_node;
    q->rear = new_node;

    q->size++;
}

void q_dequeue(queue_t *q)
{
    if (!q || !q->front) return;

    q_node_t *tmp = q->front;
    q->front = q->front->next;

    if (!q->front)
	    q->rear = q->front;
    q->size--;

    free(tmp);
}

q_node_t *q_front(queue_t *q)
{
    if (!q || !q->front)
        return NULL;

    return q->front;
}

int q_is_empty(queue_t *q)
{
    return (!q->size);
}
