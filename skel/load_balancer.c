/*
 * Copyright (c) 2024, <>
 */

#include "load_balancer.h"
#include "constants.h"
#include "queue.h"
#include "server.h"
#include "utils.h"
#include <stdio.h>
#include <stdlib.h>

static
void insert_keep_sorted(server **arr, unsigned int *size,
                        server *to_add) {
    unsigned int i = *size;
    while (i > 0 && hash_uint(&arr[i - 1]->id) > hash_uint(&to_add->id)) {
        arr[i] = arr[i - 1];
        --i;
    }
    arr[i] = to_add;
    ++(*size);
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
    // printf("server pointer : %p\n", new_server);
    new_server->id = server_id;

    insert_keep_sorted(main->server_ring, &main->number_of_servers, new_server);
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

    request *dummy_get = malloc(sizeof(*dummy_get));
    dummy_get->type = GET_DOCUMENT;
    dummy_get->doc_name = NULL;
    dummy_get->doc_content = NULL;

    if (!q_is_empty(main->server_ring[index]->requests)) {
        response *dummy = server_handle_request(main->server_ring[index], dummy_get);
        free(dummy->server_response);
        free(dummy->server_log);
        free(dummy);
    }

    free(dummy_get);

    unsigned int successor;
    if (index == main->number_of_servers - 1)
        successor = 0;
    else
        successor = index + 1;

    
}

response *loader_forward_request(load_balancer* main, request *req) {
    unsigned int doc_hash = hash_string(req->doc_name);

    // printf("doc hash : %u\n", doc_hash);
    for (unsigned int i = 0; i < main->number_of_servers; ++i) {
        // printf("server[%d] hash : %u\n", i, hash_uint(&main->server_ring[i]->id));
        if (doc_hash < hash_uint(&main->server_ring[i]->id))
            return server_handle_request(main->server_ring[i], req);
    }

    return server_handle_request(main->server_ring[0], req);
}

void free_load_balancer(load_balancer** main) {
    load_balancer *m = *main;
    for (int i = 0; i < MAX_SERVERS; ++i) {
        // printf("pointer[%d]: %p\n", i, m->server_ring[i]);
        free_server(&m->server_ring[i]);
    }

    free(m->server_ring);

    free(m);

    *main = NULL;
}

