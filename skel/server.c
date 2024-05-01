/**
 * @file server.c
 * @author GOIDESCU Rares-Stefan (known.as.rares@gmail.com)
 * @date 2024-05-01
 * 
 * @copyright Copyright (c) 2024
 * 
 */

#include <stdio.h>
#include <string.h>
#include "server.h"
#include "constants.h"
#include "hash_map.h"
#include "lru_cache.h"
#include "utils.h"

static response
*server_edit_document(server *s, char *doc_name, char *doc_content) {
    response *exit_code = malloc(sizeof(*exit_code));
    DIE(!exit_code, "Malloc failed");
    
    // As zice sa fac cache-ul un hashmap in care tin pointeri catre documente
    // care se afla in momentul actual in cache
    // (key, val) = (doc_pointer, age_in_cache)

    if (has_key(s->cache->data, doc_name)) {
        strcpy(exit_code->server_log, MSG_B);
        strcpy(exit_code->server_response, LOG_HIT);

        // modify entry in cache and in db
    } else {
        if (has_key(s->documents, doc_name)) {
            strcpy(exit_code->server_response, MSG_B);
            // ? strcpy(exit_code->server_log, LOG_MISS);

            // put doc in cache
            // modify entry
        } else {
            strcpy(exit_code->server_response, MSG_C);
            // add entry in cache and db
        }

        if (s->cache->data->size == s->cache->data->max_size) {
            strcpy(exit_code->server_log, LOG_EVICT);
            char *oldest_doc_name;
            // get_oldest_doc();
            remove_entry(s->cache->data, oldest_doc_name);
            // boom oldest cached doc
        } else {
            strcpy(exit_code->server_log, LOG_MISS);
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
    /* TODO */
    return NULL;
}

response *server_handle_request(server *s, request *req) {
    /* TODO */
    return NULL;
}

void free_server(server **s) {
    /* TODO */
}
