/**
 * @file lru_cache.c
 * @author GOIDESCU Rares-Stefan (known.as.rares@gmail.com)
 * @date 2024-05-01
 * 
 * @copyright Copyright (c) 2024
 * 
 */

#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "lru_cache.h"
#include "constants.h"
#include "doubly_linked_list.h"
#include "hash_map.h"
#include "utils.h"

typedef struct node_data_t {
    char *doc_name;
    char *doc_content;
} node_data_t;

lru_cache *init_lru_cache(unsigned int cache_capacity) {
    lru_cache *cache = malloc(sizeof(*cache));
    DIE(!cache, "Malloc failed");

    cache->cache_order = create_dll(sizeof(node_data_t));
    cache->data = create_hash_map(cache_capacity, hash_string, compare_strings, free_entry);
    cache->capacity = cache_capacity;

    return cache;
}

bool lru_cache_is_full(lru_cache *cache) {
    if (cache->capacity == cache->data->size)
        return true;
    return false;
}

void free_lru_cache(lru_cache **cache) {
    free_dll(&(*cache)->cache_order);
    free_map(&(*cache)->data);
    free(*cache);
    *cache = NULL;
}

bool lru_cache_put(lru_cache *cache, void *key, void *value,
                   void **evicted_key) {
    if (has_key(cache->data, key))
        return false;

    void *evict = *evicted_key;

    /* Iau o structura auxiliara */
    node_data_t doc_data;
    doc_data.doc_name = key;
    doc_data.doc_content = value;

    /* Daca e full cache-ul scot LRU doc din cache si adaug noul document */
    if (lru_cache_is_full(cache)) {
        dll_node_t *lru_doc =
            remove_dll_nth_node(cache->cache_order, UINT_MAX);

        node_data_t *lru_doc_data = (node_data_t *)lru_doc->data;

        /* Tin minte numele documentului scos din cache */
        evict = calloc(1, DOC_NAME_LENGTH + 1);
        DIE(!evict, "Calloc failed");
        memcpy(evict, lru_doc_data->doc_name, DOC_NAME_LENGTH + 1);
        
        free(lru_doc_data->doc_content);
        free(lru_doc_data->doc_name);
        free(lru_doc->data);
        free(lru_doc);

        remove_entry(cache->data, evict);
        /* Adaug in cache continutul structurii auxiliare */
        add_dll_nth_node(cache->cache_order, 0, &doc_data);

        add_entry(cache->data,
                  key, DOC_NAME_LENGTH + 1,
                  cache->cache_order->head, sizeof(dll_node_t *));
    } else {
        add_dll_nth_node(cache->cache_order, 0, &doc_data);
        add_entry(cache->data,
                  key, DOC_NAME_LENGTH + 1,
                  cache->cache_order->head, sizeof(dll_node_t *));
        evict = NULL;
    }

    /* Aici este buba :D */
    //free(node_data->doc_content);
    //free(node_data->doc_name);
    //free(node_data);

    *evicted_key = evict;
    return true;
}

void *lru_cache_get(lru_cache *cache, void *key) {
    dll_node_t *node = *(dll_node_t **)get_value(cache->data, key);
    if (!node)
        return NULL;

    node_data_t *node_data = (node_data_t *)node->data;

    return node_data->doc_content;
}

void lru_cache_remove(lru_cache *cache, void *key) {
    remove_entry(cache->data, key);
}
