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
#include "simply_linked_list.h"
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

    // printf("Caching in:\n");
    // puts((char *)key);
    // puts((char *)value);

    void *evict = *evicted_key;

    doc_data_t *doc_data = malloc(sizeof(*doc_data));
    doc_data->doc_name = strdup(key);
    doc_data->doc_content = strdup(value);

    if (!lru_cache_is_full(cache)) {
        // puts("Cache first branch");
        add_dll_nth_node(cache->data, 0, doc_data);
        // doc_data_t *test = cache->data->head->data;
        // printf("Cached content: ");
        // puts((char *)test->doc_content);
        // printf("cache_put: %p\n", cache->data->head);
        add_entry(cache->map, key, strlen(key) + 1, &cache->data->head, sizeof(dll_node_t *));
        evict = NULL;
    } else {
        // puts("Cache second branch");
        dll_node_t *lru_doc = remove_dll_nth_node(cache->data, UINT_MAX);
        doc_data_t *lru_doc_data = lru_doc->data;
        evict = strdup(lru_doc_data->doc_name);
        remove_entry(cache->map, evict);
        add_dll_nth_node(cache->data, 0, doc_data);
        add_entry(cache->map, key, strlen(key) + 1, &cache->data->head, sizeof(dll_node_t *));
        free(lru_doc_data->doc_content);
        free(lru_doc_data->doc_name);
        free(lru_doc_data);
        free(lru_doc);
    }

    *evicted_key = evict;

    free(doc_data);

    return true;
}

void *lru_cache_get(lru_cache *cache, void *key) {
    // puts("================== START =====================");
    // printf("Getting from cache: ");
    // puts((char *)key);

    // print_map(cache->map, stdout);

    unsigned int index = hash_string(key) % cache->capacity;
    // printf("index : %u\n", index);

    ll_node_t *curr = cache->map->buckets[index]->head;

    while (curr) {
        entry_t *entry = (entry_t *)curr->data;
        // printf("loop key :\t%s\n", (char *)entry->key);
        // printf("loop val :\t%p\n", entry->val);
        if (!compare_strings(entry->key, key))
            break;
        curr = curr->next;
    }

    // printf("1st node : %p\n", cache->data->head);
    // printf("2nd node : %p\n", cache->data->head->next);
    // printf("3rd node : %p\n", cache->data->head->next->next);

    if (!curr)
        return NULL;

    entry_t *entry = (entry_t *)curr->data;
    dll_node_t *dummy = *(dll_node_t **)entry->val;

    if (dummy == cache->data->tail) {
        cache->data->tail = dummy->prev;
        cache->data->head = dummy;
    } else if (dummy != dummy->next && dummy != cache->data->head) {
        dummy->prev->next = dummy->next;
        dummy->next->prev = dummy->prev;

        dummy->next = cache->data->head;
        cache->data->head->prev = dummy;
        dummy->prev = cache->data->tail;
        cache->data->tail->next = dummy;

        cache->data->head = dummy;
    }
    // if (dummy == cache->data->head)
    //     puts("Hello head");
    doc_data_t *doc_data = (doc_data_t *)dummy->data;

    // printf("Got from cache:\n");
    // printf("Should get: ");
    // puts((char *)entry->key);
    // printf("Got: ");
    // puts((char *)doc_data->doc_name);
    // puts((char *)doc_data->doc_content);

    // puts("================== STOP ======================");

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

    // puts((char *)key);
    if (!key)
        puts("Mars acasa");

    remove_entry(cache->map, key);
}
