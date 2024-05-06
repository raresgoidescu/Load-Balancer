/**
 * @file queue.c
 * @author GOIDESCU Rares-Stefan (known.as.rares@gmail.com)
 * @brief implementare folosind o lista simplu inlantuita
 * @date 2024-05-01
 * 
 * @copyright Copyright (c) 2024
 * 
 */

#include "queue.h"
#include "simply_linked_list.h"
#include "utils.h"
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

queue_t *q_create(unsigned int data_size,
                  unsigned int max_size,
                  void (*destructor)(void **)) {
    queue_t *q = malloc(sizeof(*q));
    DIE(!q, "Malloc failed");

    q->front = NULL;
    q->rear = NULL;
    q->data_size = data_size;
    q->size = 0;
    q->max_size = max_size;
    q->destructor = *destructor;

    return q;
}

void q_enqueue(queue_t *q, void *new_data) {
    if (!q)
	    return;

    ll_node_t *new_node = calloc(1, sizeof(*new_node));
    DIE(!new_node, "Calloc failed");

    new_node->data = calloc(1, q->data_size);
    DIE(!new_node->data, "Calloc failed");

    memcpy(new_node->data, new_data, q->data_size);

    if (!q->rear) {
	    q->front = q->rear = new_node;
        q->size++;
	    return;
    }

    q->rear->next = new_node;
    q->rear = new_node;

    q->size++;
}

void q_dequeue(queue_t *q) {
    if (!q || !q->front)
	    return;

    ll_node_t *tmp = q->front;
    q->front = q->front->next;

    if (!q->front)
	    q->rear = q->front;
    q->size--;

    q->destructor(&tmp->data);
    free(tmp);
    tmp = NULL;
}

ll_node_t *q_front(queue_t *q) {
    if (!q || !q->front)
	    return NULL;

    return q->front;
}

bool q_is_empty(queue_t *q) { return (!q->size); }

void q_free(queue_t **queue_ref) {
    queue_t *queue = *queue_ref;
    if (!queue)
        return;

    while (q_is_empty(queue) > 0) {
        q_dequeue(queue);
    }

    free(queue);
    *queue_ref = NULL;
}
