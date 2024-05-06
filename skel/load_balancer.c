/*
 * Copyright (c) 2024, <>
 */

#include "load_balancer.h"
#include "constants.h"
#include "hash_map.h"
#include "lru_cache.h"
#include "queue.h"
#include "server.h"
#include "simply_linked_list.h"
#include "utils.h"
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static
void insert_keep_sorted(server **arr, int *size,
                        server *to_add) {
    unsigned int i = *size;
    while (i > 0 && hash_uint(&arr[i - 1]->id) > hash_uint(&to_add->id)) {
        arr[i] = arr[i - 1];
        --i;
    }
    arr[i] = to_add;
    ++(*size);
}

static
void send_empty_request(server *s) {
    request *dummy_get = malloc(sizeof(*dummy_get));
    dummy_get->type = GET_DOCUMENT;
    dummy_get->doc_name = NULL;
    dummy_get->doc_content = NULL;

    if (!q_is_empty(s->requests)) {
        response *dummy = server_handle_request(s, dummy_get);
        free(dummy->server_response);
        free(dummy->server_log);
        free(dummy);
    }

    free(dummy_get);
}

load_balancer *init_load_balancer(bool enable_vnodes) {
    enable_vnodes = 0;
    load_balancer *load_balancer = malloc(sizeof(*load_balancer));
    DIE(!load_balancer, "Malloc failed");

    load_balancer->server_ring = calloc(MAX_SERVERS, sizeof(server *));
    DIE(!load_balancer->server_ring, "Calloc failed");
    for (int i = 0; i < MAX_SERVERS; ++i)
        load_balancer->server_ring[i] = NULL;

    load_balancer->number_of_servers = 0;

    load_balancer->hash_function_servers = *hash_uint;
    load_balancer->hash_function_docs = *hash_string;

    return load_balancer;
}

void loader_add_server(load_balancer* main, int server_id, int cache_size) {
    server *new_server = init_server(cache_size);
    new_server->id = server_id;

    insert_keep_sorted(main->server_ring, &main->number_of_servers, new_server);

    int index = -1;
    for (int i = 0; i < main->number_of_servers; ++i) {
        if (main->server_ring[i]->id == server_id) {
            index = i;
            break;
        }
    }
    if (index == -1 || main->number_of_servers == 1)
        return;

    int next = (index == main->number_of_servers - 1) ? 0 : index + 1;

    if (index == next)
        return;

    unsigned int server_hash = hash_uint(&server_id);
    unsigned int last_server_hash = hash_uint(&main->server_ring[main->number_of_servers - 1]->id);

    hash_map_t *next_server_db = main->server_ring[next]->data_base;

    send_empty_request(main->server_ring[next]);

    // Daca adaug pe ultimul index, atunci trebuie sa ma uit pe primul index
    for (unsigned int i = 0; i < next_server_db->max_size; ++i) {
        ll_node_t *curr = next_server_db->buckets[i]->head;
        while (curr) {
            entry_t *curr_entry = curr->data;
            ll_node_t *spare_pointer = curr->next;

            if (next == 0 && hash_string(curr_entry->key) > hash_uint(&main->server_ring[next]->id) && hash_string(curr_entry->key) <= server_hash) {
                add_entry(main->server_ring[index]->data_base, curr_entry->key, strlen(curr_entry->key) + 1, curr_entry->val, strlen(curr_entry->val) + 1);
                lru_cache_remove(main->server_ring[next]->cache, curr_entry->key);
                remove_entry(next_server_db, curr_entry->key);
                curr = spare_pointer;
                continue;
            }
            else if (hash_string(curr_entry->key) > last_server_hash && index == 0) {
                add_entry(main->server_ring[index]->data_base, curr_entry->key, strlen(curr_entry->key) + 1, curr_entry->val, strlen(curr_entry->val) + 1);
                lru_cache_remove(main->server_ring[next]->cache, curr_entry->key);                
                remove_entry(next_server_db, curr_entry->key);
                curr = spare_pointer;
                continue;
            }
            else if (next && hash_string(curr_entry->key) <= server_hash) {
                add_entry(main->server_ring[index]->data_base, curr_entry->key, strlen(curr_entry->key) + 1, curr_entry->val, strlen(curr_entry->val) + 1);
                lru_cache_remove(main->server_ring[next]->cache, curr_entry->key);                
                remove_entry(next_server_db, curr_entry->key);
            }
            curr = spare_pointer;
        }
    }
}

void loader_remove_server(load_balancer* main, int server_id) {
    if (main->number_of_servers == 0)
        return;

    int index = -1;
    for (int i = 0; i < main->number_of_servers; ++i)
        if (main->server_ring[i]->id == server_id) {
            index = i;
            break;
        }

    if (index == -1)
        return;

    int successor = (index == main->number_of_servers - 1) ? 0 : index + 1;

    send_empty_request(main->server_ring[index]);

    // bool gotta_update_the_queue = false;
    // for (unsigned int i = 0; i < main->server_ring[index]->data_base->max_size; ++i) {
    //     ll_node_t *curr = main->server_ring[index]->data_base->buckets[i]->head;
    //     while (curr) {
    //         doc_data_t *doc_data = curr->data;
    //         if (has_key(main->server_ring[successor]->data_base, doc_data->doc_name))
    //             send_empty_request(main->server_ring[successor]);
    //         curr = curr->next;
    //     }
    // }

    copy_entries(main->server_ring[index]->data_base, main->server_ring[successor]->data_base);

    free_server(&main->server_ring[index]);

    for (int i = index; i < MAX_SERVERS - 1; ++i)
        main->server_ring[i] = main->server_ring[i + 1];

    main->server_ring[MAX_SERVERS - 1] = NULL;

    main->number_of_servers--;
}

response *loader_forward_request(load_balancer* main, request *req) {
    unsigned int doc_hash = hash_string(req->doc_name);

    for (int i = 0; i < main->number_of_servers; ++i)
        if (doc_hash < hash_uint(&main->server_ring[i]->id)) {
            return server_handle_request(main->server_ring[i], req);
        }

    return server_handle_request(main->server_ring[0], req);
}

void free_load_balancer(load_balancer** main) {
    load_balancer *m = *main;
    for (int i = 0; i < MAX_SERVERS; ++i) {
        // if (m->server_ring[i]) {
        //     printf("[%.3d] %d\n", i, m->server_ring[i]->id);
        //     if (has_key(m->server_ring[i]->data_base, "financial_rich.txt"))
        //         printf("\tfinancial_rich.txt\n");
        // }
        free_server(&m->server_ring[i]);
    }

    free(m->server_ring);

    free(m);

    *main = NULL;
}

