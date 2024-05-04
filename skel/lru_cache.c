/*
 * Copyright (c) 2024, <>
 */

#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "lru_cache.h"
#include "doubly_linked_list.h"
#include "hash_map.h"
#include "utils.h"

lru_cache *init_lru_cache(unsigned int cache_capacity) {
    lru_cache *cache = malloc(sizeof *cache);
    DIE(!cache, "Malloc failed");

    cache->capacity = cache_capacity;
    cache->map = create_hash_map(cache_capacity, hash_string, compare_strings, free_entry);
    cache->data = create_dll(sizeof(doc_data_t));

    return cache;
}

bool lru_cache_is_full(lru_cache *cache) {
    if (cache->capacity == cache->map->size)
        return true;
    return false;
}

void free_lru_cache(lru_cache **cache) {
    lru_cache *c = *cache;
    doubly_linked_list_t *list = c->data;
    if (list) {
        dll_node_t *curr = list->head;
        for (unsigned int i = 0; i < list->size; ++i) {
            list->head = curr->next;
            doc_data_t *doc_data = curr->data;
            free(doc_data->doc_name);
            free(doc_data->doc_content);
            free(doc_data);
            free(curr);
            curr = list->head;
        }
        free(list);
    }
    free_map(&c->map);
    free(c);
    *cache = NULL;
}

bool lru_cache_put(lru_cache *cache, void *key, void *value,
                   void **evicted_key) {
    if (has_key(cache->map, key))
        return false;

    void *evict = *evicted_key;

    doc_data_t *doc_data = malloc(sizeof(*doc_data));
    doc_data->doc_name = strdup(key);
    doc_data->doc_content = strdup(value);

    if (!lru_cache_is_full(cache)) {
        add_dll_nth_node(cache->data, 0, doc_data);
        add_entry(cache->map, key, strlen(key) + 1, cache->data->head, sizeof(dll_node_t *));
        evict = NULL;
    } else {
        dll_node_t *lru_doc = remove_dll_nth_node(cache->data, UINT_MAX);
        doc_data_t *lru_doc_data = lru_doc->data;
        evict = strdup(lru_doc_data->doc_name);
        free(lru_doc_data->doc_content);
        free(lru_doc_data->doc_name);
        free(lru_doc_data);
        free(lru_doc);
        remove_entry(cache->map, evict);
        add_dll_nth_node(cache->data, 0, doc_data);
        add_entry(cache->map, key, strlen(key) + 1, cache->data->head, sizeof(dll_node_t *));
    }

    *evicted_key = evict;

    free(doc_data);

    return true;
}

void *lru_cache_get(lru_cache *cache, void *key) {
    dll_node_t *node = *(dll_node_t **)get_value(cache->map, key);

    if (!node)
        return NULL;

    doc_data_t *doc_data = (doc_data_t *)node->data;

    return doc_data->doc_content;
}

void lru_cache_remove(lru_cache *cache, void *key) {
    dll_node_t *node = *(dll_node_t **)get_value(cache->map, key);
    if (!node) {
        puts("E NULL");
        return;
    }

    doc_data_t *doc_data = (doc_data_t *)node->data;
    // puts(doc_data->doc_name);
    // puts(doc_data->doc_content);
    free(doc_data->doc_content);
    free(doc_data->doc_name);
    free(doc_data);

    if (node == node->next) {
        free(node);
        cache->data->head = cache->data->tail = NULL;
        node = NULL;
    } else {
        node->prev->next = node->next;
        node->next->prev = node->prev;
        free(node);
        node = NULL;
    }

    cache->data->size--;
    /* Sterge nodul din lista circulara. Bafta! */

    // puts((char *)key);
    if (!key)
        puts("Mars acasa");

    remove_entry(cache->map, key);
}
