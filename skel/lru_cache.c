/**
 * @file lru_cache.c
 * @author GOIDESCU Rares-Stefan (known.as.rares@gmail.com)
 * @date 2024-05-01
 * 
 * @copyright Copyright (c) 2024
 * 
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "lru_cache.h"
#include "doubly_linked_list.h"
#include "hash_map.h"
#include "utils.h"

lru_cache *init_lru_cache(unsigned int cache_capacity) {
    lru_cache *cache = malloc(sizeof(*cache));
    DIE(!cache, "Malloc failed");

    cache->cache_order = create_dll(sizeof(void *));
    cache->data = create_hash_map(cache_capacity, hash_string, compare_strings, free_entry);
    cache->capacity = cache_capacity;

    return cache;
}

bool lru_cache_is_full(lru_cache *cache) {
    if (cache->capacity == cache->cache_order->size)
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
    return true;
}

void *lru_cache_get(lru_cache *cache, void *key) {
    /* TODO */
    return NULL;
}

void lru_cache_remove(lru_cache *cache, void *key) {
    /* TODO */
}
