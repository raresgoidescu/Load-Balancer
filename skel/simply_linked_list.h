/*
 * Copyright (c) 2024, GOIDESCU Rares-Stefan 312CA <known.as.rares@gmail.com>
 */

#ifndef __SIMPLY_LL__
#define __SIMPLY_LL__

#include <stdlib.h>
#include <stdbool.h>

typedef struct ll_node_t {
    /* Pointer catre urmatorul nod */
    struct ll_node_t *next;
    void *data;
} ll_node_t;

typedef struct linked_list_t {
    /* Pointer catre inceputul listei */
    ll_node_t *head;
    /* Dimensiunea unui element in octeti */
    unsigned int data_size;
    /* Numarul de elemente curent din lista */
    unsigned int size;
} linked_list_t;

/**
 * @brief Functie care creeaza si initializeaza o lista simplu inlantuita
 * @return (linked_list_t *) - pointer catre noua lista
 */
linked_list_t *create_ll(unsigned int data_size);

/**
 * @brief Functie care adauga un nod pe pozitia n dintr-o lista
 * @param n - pozitia pe care vrem sa inseram
 */
void add_ll_nth_node(linked_list_t *list, unsigned int n, void *new_data);

/**
 * NOTE: Apelantul trebuie sa elibereze memoria ocupata de nodul extras
 * @brief Functie care elimina nodul de pe pozitia n dintr-o lista
 * @param n - pozitia nodului pe care vrem sa il stergem
 * @return (ll_node_t *) - pointer catre nodul sters
 */
ll_node_t *remove_ll_nth_node(linked_list_t *list, unsigned int n);

/**
 * @brief Functie care determina numarul de elemente dintr-o lista
 * @param fast - rapid sau reliable :)
 * @return (unsigned int) - numarul de elemente
 */
unsigned int get_ll_size(linked_list_t *list, bool fast);

/**
 * @brief Functie care elibereaza toata memoria folosita de o lista
 */
void free_ll(linked_list_t **list_ref);

#endif /* __SIMPLY_LL__ */