#ifndef __QUEUE__
#define __QUEUE__

#include <stdbool.h>

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
 * @return - (q_node_t *) catre primul element din coada
 */
q_node_t *q_front(queue_t *q);

/**
 * @brief Functie care determina daca o coada este goala sau nu
 * @return - true if empty, false otherwise
 */
bool q_is_empty(queue_t *q);

#endif /* __QUEUE__ */
