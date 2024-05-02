/**
 * @file doubly_linked_list.c
 * @author GOIDESCU Rares-Stefan (known.as.rares@gmail.com)
 * @date 2024-05-01
 * 
 * @copyright Copyright (c) 2024
 * 
 */

#include "doubly_linked_list.h"

doubly_linked_list_t *create_dll(unsigned int data_size) {
    doubly_linked_list_t *list = malloc(sizeof(*list));
    DIE(!list, "Malloc failed");

    list->head = NULL;
    list->tail = NULL;
    list->data_size = data_size;
    list->size = 0;

    return list;
}

void add_dll_nth_node(doubly_linked_list_t *list,
                      unsigned int n,
                      void *new_data) {
    dll_node_t *new_node = malloc(sizeof(*new_node));
    DIE(!new_node, "Malloc failed");

    new_node->data = malloc(list->data_size);
    DIE(!new_node->data, "Malloc failed");

    memcpy(new_node->data, new_data, list->data_size);

    if (list->size == 0) {
        new_node->next = list->head;
        new_node->prev = new_node;
        list->head = new_node;
        list->tail = new_node;
        list->head->next = new_node;
        list->tail->next = new_node;
        list->tail->prev = new_node;
        list->head->prev = new_node;
        goto done_adding_dll_node;
    }

    if (n >= list->size) {
        list->tail->next = new_node;
        new_node->prev = list->tail;
        new_node->next = list->head;
        list->head->prev = new_node;
        list->tail = new_node;
        goto done_adding_dll_node;
    }

    if (n == 0) {
        new_node->next = list->head;
        new_node->prev = list->tail;
        list->tail->next = new_node;
        list->head->prev = new_node;
        list->head = new_node;
        goto done_adding_dll_node;
    }

    dll_node_t *curr = list->head;
    for (unsigned int i = 0; i < n && curr; ++i)
        curr = curr->next;

    new_node->next = curr;
    curr->prev->next = new_node;
    new_node->prev = curr->prev;
    curr->prev = new_node;

done_adding_dll_node:
    list->size++;
}

dll_node_t *remove_dll_nth_node(doubly_linked_list_t *list,
                                unsigned int n) {
    if (!list)
        return NULL;

    dll_node_t *curr = list->head;

    if (n == 0) {
        list->tail->next = list->head->next;
        list->head->next->prev = list->tail;
        list->head = list->head->next;
        goto done_removing_dll_node;
    }
    if (n >= list->size - 1) {
        list->tail->prev->next = list->head;
        list->head->prev = list->tail->prev;
        curr = list->tail;
        list->tail = list->tail->prev;
        goto done_removing_dll_node;
    }

    for (unsigned int i = 0; i < n && i < list->size - 1; ++i)
        curr = curr->next;

    curr->prev->next = curr->next;
    curr->next->prev = curr->prev;

done_removing_dll_node:
    list->size--;
    return curr;
}

void free_dll(doubly_linked_list_t **list_ref) {
    if (!*list_ref || !list_ref)
        return;

    dll_node_t *curr = (*list_ref)->head;

    for (unsigned int i = 0; i < (*list_ref)->size; ++i) {
        (*list_ref)->head = curr->next;
        free(curr->data);
        free(curr);
        curr = (*list_ref)->head;
    }

    free(*list_ref);
}