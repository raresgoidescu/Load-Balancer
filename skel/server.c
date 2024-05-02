/**
 * @file server.c
 * @author GOIDESCU Rares-Stefan (known.as.rares@gmail.com)
 * @date 2024-05-01
 * 
 * @copyright Copyright (c) 2024
 * 
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "server.h"
#include "constants.h"
#include "hash_map.h"
#include "lru_cache.h"
#include "queue.h"
#include "utils.h"

static response
*server_edit_document(server *s, char *doc_name, char *doc_content) {
    response *exit_code = malloc(sizeof(*exit_code));
    
    if (has_key(s->cache->data, doc_name)) {
        exit_code->server_log = MSG_B;
        exit_code->server_response = LOG_HIT;

        // modify entry in cache and in db
    } else {
        if (has_key(s->documents, doc_name)) {
            exit_code->server_response = MSG_B;
            // ? strcpy(exit_code->server_log, LOG_MISS);

            // put doc in cache
            // modify entry
        } else {
            exit_code->server_response = MSG_C;
            // add entry in cache and db
        }

        if (s->cache->data->size == s->cache->data->max_size) {
            exit_code->server_log = LOG_EVICT;
            char *oldest_doc_name;
            // get_oldest_doc();
            remove_entry(s->cache->data, oldest_doc_name);
            // boom oldest cached doc
        } else {
            exit_code->server_log = LOG_MISS;
        }
    }

    return exit_code;
}

static response
*server_get_document(server *s, char *doc_name) {
    /* TODO */
    return NULL;
}

server *init_server(unsigned int cache_size) {
    server *sv = malloc(sizeof(*sv));
    DIE(!sv, "Malloc failed");

    sv->cache = init_lru_cache(cache_size);
    sv->documents = create_hash_map(100, hash_string, compare_strings, free_entry);
    sv->id = 0;
    sv->task_queue = q_create(REQUEST_LENGTH);

    return sv;
}

response *server_handle_request(server *s, request *req) {
    /* TODO */
    return NULL;
}

void free_server(server **s) {
    /* TODO */
}
