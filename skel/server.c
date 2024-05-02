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
#include "simply_linked_list.h"
#include "utils.h"

static response
*server_edit_document(server *s, char *doc_name, char *doc_content) {
    response *exit_code = malloc(sizeof(*exit_code));
    DIE(!exit_code, "Malloc failed");

    exit_code->server_response = calloc(1, DOC_CONTENT_LENGTH + 1);
    DIE(!exit_code->server_response, "Calloc failed");

    exit_code->server_log = calloc(1, strlen(LOG_EVICT) + 2 * DOC_NAME_LENGTH);
    DIE(!exit_code->server_log, "Calloc failed");

    if (has_key(s->cache->data, doc_name)) {
        sprintf(exit_code->server_log, LOG_HIT, doc_name);
        sprintf(exit_code->server_response, MSG_B, doc_name);

        // modify entry in cache and in db
    } else {
        if (has_key(s->documents, doc_name)) {
            sprintf(exit_code->server_response, MSG_B, doc_name);

            // put doc in cache
            // modify entry
        } else {
            sprintf(exit_code->server_response, MSG_C, doc_name);
            // add entry in cache and db
        }

        if (s->cache->data->size == s->cache->data->max_size) {
            char *oldest_doc_name = "hahalol";
            // get_oldest_doc();
            sprintf(exit_code->server_log, LOG_EVICT, doc_name, oldest_doc_name);
            // remove_entry(s->cache->data, oldest_doc_name);
            // boom oldest cached doc
        } else {
            sprintf(exit_code->server_log, LOG_MISS, doc_name);
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
    sv->documents = create_hash_map(10, hash_string, compare_strings, free_entry);
    sv->task_queue = q_create(REQUEST_LENGTH, TASK_QUEUE_SIZE);

    return sv;
}

response *server_handle_request(server *s, request *req) {
    response *exit_code = malloc(sizeof(*exit_code));
    DIE(!exit_code, "Malloc failed\n");

    exit_code->server_response = calloc(1, DOC_CONTENT_LENGTH + 1);
    DIE(!exit_code->server_response, "Calloc failed");

    exit_code->server_log = calloc(1, strlen(LOG_EVICT) + 2 * DOC_NAME_LENGTH);
    DIE(!exit_code->server_log, "Calloc failed");

    exit_code->server_id = s->id;

    if (req->type == EDIT_DOCUMENT) {
        q_enqueue(s->task_queue, req);
        sprintf(exit_code->server_response, MSG_A, "EDIT", req->doc_name);
        sprintf(exit_code->server_log, LOG_LAZY_EXEC, s->task_queue->size);
    } else {
        request *curr_req = NULL;

        while (!q_is_empty(s->task_queue)) {
            ll_node_t *front = q_front(s->task_queue);
            curr_req = (request *)front->data;

            char *req_type = get_request_type_str(curr_req->type);

            printf("Current req type:\t%s\ndoc_name:\t%s\ndoc_content:\t%s\n", req_type, curr_req->doc_name, curr_req->doc_content);
            
            q_dequeue(s->task_queue);
        }
        if (curr_req) {
            sprintf(exit_code->server_response, "%s", curr_req->doc_content);
            sprintf(exit_code->server_log, LOG_HIT, req->doc_name);
        } else {
            sprintf(exit_code->server_response, "(null)");
            sprintf(exit_code->server_log, LOG_FAULT, req->doc_name);
        }
    }

    return exit_code;
}

void free_server(server **s) {
    free_map(&(*s)->documents);
    free_lru_cache(&(*s)->cache);
    q_free(&(*s)->task_queue);
}
