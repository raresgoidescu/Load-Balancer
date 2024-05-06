/**
 * @file doubly_linked_list.h
 * @author GOIDESCU Rares-Stefan (known.as.rares@gmail.com)
 * @date 2024-05-01
 * 
 * @copyright Copyright (c) 2024
 * 
 */

#ifndef __DOUBLY_LL__
#define __DOUBLY_LL__

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "utils.h"

typedef struct dll_node_t {
    struct dll_node_t *prev;
    void *data;
    struct dll_node_t *next;
} dll_node_t;

typedef struct doubly_linked_list_t {
    dll_node_t *head;
    dll_node_t *tail;
    unsigned int data_size;
    unsigned int size;
} doubly_linked_list_t;

/**
 * @brief Creeaza o lista dublu inlantuita circulara
 * 
 * @param data_size - dimensiunea elementelor
 * @return doubly_linked_list_t*
 */
doubly_linked_list_t *create_dll(unsigned int data_size);

/**
 * @brief Adauga un nod in lista dublu inlantuita
 * 
 * @param list - lista in care vrem sa adaugam nodul
 * @param n - pozitia pe care vrem sa adaugam nodul
 * @param new_data - informatia noului nod
 */
void add_dll_nth_node(doubly_linked_list_t *list,
                      unsigned int n,
                      void *new_data);

/**
 * @brief Elimina un nod dintr-o lista dublu inlantuita
 * 
 * @param list - lista din care vrem sa eliminam nodul
 * @param n - pozitia nodului pe care vrem sa il eliminam
 * @return dll_node_t* - nodul eliminat, pentru urmatoare operatii
 */
dll_node_t *remove_dll_nth_node(doubly_linked_list_t *list,
                                unsigned int n);


void free_dll(doubly_linked_list_t **list_ref);

#endif /* __DOUBLY_LL__ */
