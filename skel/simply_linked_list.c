/*
 * Copyright (c) 2024, GOIDESCU Rares-Stefan 312CA <known.as.rares@gmail.com>
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "utils.h"
#include "simply_linked_list.h"

linked_list_t *create_list(unsigned int data_size) {
    linked_list_t *list = malloc(sizeof(*list));
    DIE(!list, "Malloc failed");
    list->data_size = data_size;
    list->size = 0;
    list->head = NULL;
    return list;
}

void add_ll_nth_node(linked_list_t *list, unsigned int n, void *new_data) {
    if (!list)
        return;

    n = (n > list->size) ? list->size : n;

    ll_node_t *curr = list->head;
    ll_node_t *prev = NULL;

    while (n) {
        n--;
        prev = curr;
        curr = curr->next;
    }

    ll_node_t *new_node = malloc(sizeof(*new_node));
    DIE(!new_node, "Malloc failed");
    new_node->data = calloc(1, list->data_size);
    DIE(!new_node->data, "Calloc failed\n");

    memcpy(new_node->data, new_data, list->data_size);

    new_node->next = curr;
    (!prev) ? (list->head = new_node) : (prev->next = new_node);

    list->size++;
}

ll_node_t *remove_ll_nth_node(linked_list_t *list, unsigned int n) {
    if (!list || !list->head)
        return NULL;

    n = (n > list->size - 1) ? list->size - 1 : n;

    ll_node_t *curr = list->head;
    ll_node_t *prev = NULL;

    while (n) {
        n--;
        prev = curr;
        curr = curr->next;
    }

    (!prev) ? (list->head = curr->next) : (prev->next = curr->next);

    list->size--;

    return curr;
}

unsigned int get_ll_size(linked_list_t *list, bool fast) {
    if (!list)
        return -1;

    if (fast)
        return list->size;

    ll_node_t *curr = list->head;
    unsigned int count = 0;
    while (curr) {
      count++;
      curr = curr->next;
    }
    return count;
}

void free_ll(linked_list_t **list_ref) {
    if (!list_ref || !*list_ref)
        return;

    ll_node_t *tmp;
    while (get_ll_size(*list_ref, 1)) {
        tmp = remove_ll_nth_node(*list_ref, 0);
        free(tmp->data);
        tmp->data = NULL;
        free(tmp);
        tmp = NULL;
    }

    free(*list_ref);
    *list_ref = NULL;
}
