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

    /*
        is_doc_in_cache() {
            MSG_B,
            LOG_HIT,
            modify_entry_in_cache_and_in_memory
        } else {
            is_doc_in_db() {
                MSG_B,
                put_doc_in_cache(),
                modify_entry_in_cache_and_in_memory
            } else {
                MSG_C,
                add_new_entry_in_cache_and_in_memory
            }

            is_cache_full() {
                LOG_EVICT,
                boom_one_entry_from_cache_and_move_it_to_memory
            } else {
                LOG_MISS,
            }
        }
    */

    // As zice sa fac cache-ul un hashmap in care tin pointeri catre documente
    // care se afla in momentul actual in cache
    // (key, val) = (doc_pointer, age_in_cache)

    if (has_key(s->cache->data, doc_name)) {
        /// Sigur se vrea altceva de la mine la faza asta
        exit_code->server_id = s->documents->hash(doc_name) % s->documents->max_size;

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
