/**
 * @file lru_cache.c
 * @author GOIDESCU Rares-Stefan (known.as.rares@gmail.com)
 * @date 2024-05-06
 * 
 * @copyright Copyright (c) 2024
 * 
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
    cache->map = create_hash_map(cache_capacity,
                                 hash_string,
                                 compare_strings,
                                 free_entry);
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
            if (doc_data) {
                free(doc_data->doc_name);
                free(doc_data->doc_content);
                free(doc_data);
            }
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

#ifdef DEBUG
    printf("Caching in:\n");
    puts((char *)key);
    puts((char *)value);
#endif

    void *evict = *evicted_key;

    long unsigned int key_len = strlen(key) + 1;

    doc_data_t *doc_data = malloc(sizeof(*doc_data));
    DIE(!doc_data, "Malloc failed");
    doc_data->doc_name = strdup(key);
    doc_data->doc_content = strdup(value);

    if (!lru_cache_is_full(cache)) {
        /* 
         * Daca cache-ul nu este plin, pur si simplu adaugam si
         * punem evict pe NULL, intrucat nu scoatem nimic
         */
        add_dll_nth_node(cache->data, 0, doc_data);
    #ifdef DEBUG
        doc_data_t *test = cache->data->head->data;
        printf("Cached content: ");
        puts((char *)test->doc_content);
        printf("cache_put: %p\n", cache->data->head);
    #endif
        dll_node_t *added_doc = cache->data->head;
        add_entry(cache->map, key, key_len, &added_doc, sizeof(dll_node_t *));
        evict = NULL;
    } else {
        /*
         * Daca cache-ul este plin, eliminam ultimul nod din lista,
         * cel mai vechi nod, in O(1), si adaugam noul document
         */
        dll_node_t *lru_doc = remove_dll_nth_node(cache->data, UINT_MAX);
        doc_data_t *lru_doc_data = lru_doc->data;
        evict = strdup(lru_doc_data->doc_name);
        remove_entry(cache->map, evict);
        add_dll_nth_node(cache->data, 0, doc_data);
        dll_node_t *added_doc = cache->data->head;
        add_entry(cache->map, key, key_len, &added_doc, sizeof(dll_node_t *));
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
#ifdef DEBUG
    puts("================== START =====================");
    printf("Getting from cache: ");
    puts((char *)key);

    print_map(cache->map, stdout);
#endif
    unsigned int index = hash_string(key) % cache->capacity;

    ll_node_t *curr = cache->map->buckets[index]->head;

    while (curr) {
        entry_t *entry = (entry_t *)curr->data;
    #ifdef DEBUG
        printf("loop key :\t%s\n", (char *)entry->key);
        printf("loop val :\t%p\n", entry->val);
    #endif
        if (!compare_strings(entry->key, key))
            break;
        curr = curr->next;
    }

    if (!curr)
        return NULL;

    entry_t *entry = (entry_t *)curr->data;
    dll_node_t *doc = *(dll_node_t **)entry->val;

    /*
     * Dupa ce am gasit documentul, il scoatem de unde era in cache
     * si il adaugam la inceputul listei
     * (locul celui mai recent accesat document)
     */
    if (doc == cache->data->tail) {
        cache->data->tail = doc->prev;
        cache->data->head = doc;
    } else if (doc != doc->next && doc != cache->data->head) {
        doc->prev->next = doc->next;
        doc->next->prev = doc->prev;

        doc->next = cache->data->head;
        cache->data->head->prev = doc;
        doc->prev = cache->data->tail;
        cache->data->tail->next = doc;

        cache->data->head = doc;
    }

#ifdef DEBUG
    doc_data_t *doc_data = (doc_data_t *)dummy->data;
    printf("Got from cache:\n");
    printf("Should get: ");
    puts((char *)entry->key);
    printf("Got: ");
    puts((char *)doc_data->doc_name);
    puts((char *)doc_data->doc_content);

    puts("================== STOP ======================");
#endif

    return doc;
}

void lru_cache_remove(lru_cache *cache, void *key) {
    dll_node_t *node;
    /* Gasim documentul in lista */
    if (has_key(cache->map, key))
        node = *(dll_node_t **)get_value(cache->map, key);
    else
        return;

    doc_data_t *doc_data = (doc_data_t *)node->data;
    free(doc_data->doc_content);
    free(doc_data->doc_name);
    free(doc_data);

    /* Scoatem nodul din lista */
    if (node == node->next) {
        free(node);
        cache->data->head = cache->data->tail = NULL;
        node = NULL;
    } else {
        if (node == cache->data->head)
            cache->data->head = node->next;
        if (node == cache->data->tail)
            cache->data->tail = node->prev;
        node->prev->next = node->next;
        node->next->prev = node->prev;
        free(node);
        node = NULL;
    }

    cache->data->size--;

    /* Scoatem documentul din dictionar */
    remove_entry(cache->map, key);
}
