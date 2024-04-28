/*
 * Copyright (c) 2024, GOIDESCU Rares-Stefan 312CA <known.as.rares@gmail.com>
 */

#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include "utils.h"

#include "simply_linked_list.h"
#include "hash_map.h"

bool compare_strings(void *ptr1, void *ptr2) {
    char *str1 = (char *)ptr1;
    char *str2 = (char *)ptr2;

    return strcmp(str1, str2);
}

void free_entry(void *entry_ptr) {
    entry_t *entry = entry_ptr;
    free(entry->key);
    free(entry->val);
}

hash_map_t *create_hash_map(unsigned int n_buckets,
                            unsigned int (*hash_func)(void *),
                            bool (*compare_func)(void *, void *),
                            void (*free_key_val)(void *)) {
    hash_map_t *map = malloc(sizeof(*map));
    DIE(!map, "Malloc failed");

    map->buckets = calloc(n_buckets, sizeof(linked_list_t *));
    DIE(!map->buckets, "Calloc failed");

    for (int i = 0; i < n_buckets; ++i)
        map->buckets[i] = create_ll(sizeof(entry_t));

    map->size = 0;
    map->map_max_size = n_buckets;

    map->hash = *hash_func;
    map->compare = *compare_func;
    map->free_entry = *free_key_val;

    return map;
}

bool has_key(hash_map_t *map, void *key) {
    unsigned int index = map->hash(key ) % map->map_max_size;

    ll_node_t *curr = map->buckets[index]->head;
    while (curr) {
        if (!map->compare(key, ((entry_t *)curr->data)->key))
            return true;
        curr = curr->next;
    }

    return false;
}

void *get_value(hash_map_t *map, void *key) {
    unsigned int index = map->hash(key) % map->map_max_size;
    ll_node_t *curr = map->buckets[index]->head;
    
    while (curr) {
        if (!map->compare(key, ((entry_t *)curr->data)->key))
            return ((entry_t*)curr->data)->val;
        curr = curr->next;
    }

    return NULL;
}

void add_entry(hash_map_t *map,
               void *key, unsigned int key_size,
               void *val, unsigned int val_size) {
    unsigned int index = map->hash(key) % map->map_max_size;

    entry_t *new_entry = malloc(sizeof(entry_t));
    DIE(!new_entry, "Malloc failed");

    new_entry->key = calloc(1, key_size);
    DIE(!new_entry->key, "Calloc failed");

    new_entry->val = calloc(1, val_size);
    DIE(!new_entry->val, "Calloc failed");

    memcpy(new_entry->key, key, key_size);
    memcpy(new_entry->val, val, val_size);

    add_ll_nth_node(map->buckets[index], map->buckets[index]->size, new_entry);
}

void remove_entry(hash_map_t *map, void *key) {
    unsigned int index = map->hash(key) % map->map_max_size;

    ll_node_t *curr = map->buckets[index]->head;

    int position_in_bucket = 0;

    while (curr) {
        entry_t *curr_entry = curr->data;
        if (!map->compare(key, curr_entry->key))
            break;
        curr = curr->next;
        ++position_in_bucket;
    }

    if (!curr) {
        return;
    } else {
        map->free_entry(curr->data);
        ll_node_t *_to_free = remove_ll_nth_node(map->buckets[index],
                                                    position_in_bucket);
        free(_to_free->data);
        _to_free->data = NULL;
        free(_to_free);
        _to_free = NULL;
    }
}