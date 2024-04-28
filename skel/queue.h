#ifndef __QUEUE__
#define __QUEUE__

#include <stdbool.h>
#include "simply_linked_list.h"

typedef struct queue_t {
    /* Pointer catre primul element al listei */
    ll_node_t *front;
    /* Pointer catre ultimul element al listei */
    ll_node_t *rear;
    /* Numarul de octeti al unui element */
    unsigned int data_size;
    /* Numarul de elemente din coada */
    unsigned int size;
} queue_t;

/**
 * @brief Functie care creeaza si initializeaza o coada
 * @return - (queue_t *) catre lista noii cozi
 */
queue_t *q_create(unsigned int data_size);

/**
 * @brief Functie care adauga un element in coada
 */
void q_enqueue(queue_t *q, void *new_data);

/**
 * @brief Functie care elimina (si sterge) un element in coada
 */
void q_dequeue(queue_t *q);

/**
 * @brief Functie care determina primul element din coada
 * @return - (ll_node_t *) catre primul element din coada
 */
ll_node_t *q_front(queue_t *q);

/**
 * @brief Functie care determina daca o coada este goala sau nu
 * @return - true if empty, false otherwise
 */
bool q_is_empty(queue_t *q);

/**
 * @brief Functie care elibereaza memoria folosita de o coada
 */
void q_free(queue_t *queue);

#endif /* __QUEUE__ */
