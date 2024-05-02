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

/******************************************************************************/
/***************************** PRIVATE FUNCTIONS ******************************/
/******************************************************************************/

static
response *alloc_response(unsigned int resp_lenght, unsigned int log_lenght) {
    response *ptr = malloc(sizeof(*ptr));
    DIE(!ptr, "Malloc failed");

    ptr->server_response = calloc(1, resp_lenght + 1);
    DIE(!ptr->server_response, "Calloc failed");

    ptr->server_log = calloc(1, log_lenght + 1);
    DIE(!ptr->server_log, "Calloc failed");

    ptr->server_id = 0;

    return ptr;
}

static response
*server_edit_document(server *s, char *doc_name, char *doc_content) {
    response *exit_code = alloc_response(MAX_RESPONSE_LENGTH, MAX_LOG_LENGTH);
    exit_code->server_id = s->id;

    if (has_key(s->cache->data, doc_name)) {
        sprintf(exit_code->server_log, LOG_HIT, doc_name);
        sprintf(exit_code->server_response, MSG_B, doc_name);

        /* modify entry in cache and in db */
        char *cached_doc_content =
            (char *)get_value(s->cache->data, doc_name);
        memcpy(cached_doc_content, doc_content, DOC_CONTENT_LENGTH + 1);

        char *peristent_doc_content =
            (char *)get_value(s->documents, doc_name);
        memcpy(peristent_doc_content, doc_content, DOC_CONTENT_LENGTH + 1);

        printf(GENERIC_MSG, exit_code->server_id,
               exit_code->server_response, exit_code->server_id,
               exit_code->server_log);
    } else {
        if (has_key(s->documents, doc_name)) {
            sprintf(exit_code->server_response, MSG_B, doc_name);

            /* put doc in cache */
            add_entry(s->cache->data,
                      doc_name, DOC_NAME_LENGTH + 1,
                      doc_content, DOC_CONTENT_LENGTH + 1);

            /* modify entry */
            char *db_doc_content =
                (char *)get_value(s->documents, doc_name);
            memcpy(db_doc_content, doc_content, DOC_CONTENT_LENGTH + 1);
            printf(GENERIC_MSG, exit_code->server_id,
                   exit_code->server_response, exit_code->server_id,
                   exit_code->server_log);
        } else {
            sprintf(exit_code->server_response, MSG_C, doc_name);
            sprintf(exit_code->server_log, LOG_MISS, doc_name);
            
            add_entry(s->documents,
                      doc_name, DOC_NAME_LENGTH + 1,
                      doc_content, DOC_CONTENT_LENGTH + 1);
        }

        if (lru_cache_is_full(s->cache)) {
            add_entry(s->cache->data,
                      doc_name, DOC_NAME_LENGTH + 1,
                      doc_content, DOC_CONTENT_LENGTH + 1);
            char *oldest_doc_name = "hahalol";
            // get_oldest_doc();
            sprintf(exit_code->server_log,
                    LOG_EVICT, doc_name, oldest_doc_name);
            printf(GENERIC_MSG, exit_code->server_id,
                   exit_code->server_response, exit_code->server_id,
                   exit_code->server_log);
            // remove_entry(s->cache->data, oldest_doc_name);
            // boom oldest cached doc
        } else {
            add_entry(s->cache->data,
                      doc_name, DOC_NAME_LENGTH + 1,
                      doc_content, DOC_CONTENT_LENGTH + 1);
            sprintf(exit_code->server_log, LOG_MISS, doc_name);
            printf(GENERIC_MSG, exit_code->server_id,
                   exit_code->server_response, exit_code->server_id,
                   exit_code->server_log);
        }
    }

    return exit_code;
}

static response
*server_get_document(server *s, char *doc_name) {
    response *exit_code = alloc_response(MAX_RESPONSE_LENGTH, MAX_LOG_LENGTH);

    if (has_key(s->cache->data, doc_name)) {
        sprintf(exit_code->server_response,
                "%s", (char *)get_value(s->cache->data, doc_name));
        sprintf(exit_code->server_log, LOG_HIT, doc_name);
    } else {
        if (has_key(s->documents, doc_name)) {
            char *doc_content = (char *)get_value(s->documents, doc_name);
            sprintf(exit_code->server_response, "%s", doc_content);
            /* put doc in cache */
            if (lru_cache_is_full(s->cache)) {
                /** TODO: **/
                char *oldest_doc_name = "za goochcoolen";
                // get oldest doc
                sprintf(exit_code->server_log,
                        LOG_EVICT, doc_name, oldest_doc_name);
                // remove entries
                add_entry(s->cache->data,
                          doc_name, DOC_NAME_LENGTH + 1,
                          doc_content, DOC_CONTENT_LENGTH + 1);
            } else {
                sprintf(exit_code->server_log, LOG_MISS, doc_name);
                add_entry(s->cache->data,
                          doc_name, DOC_NAME_LENGTH + 1,
                          doc_content, DOC_CONTENT_LENGTH + 1);
            }
        } else {
            sprintf(exit_code->server_response, "%s", "(null)");
            sprintf(exit_code->server_log, LOG_FAULT, doc_name);
        }
    }

    return exit_code;
}

/******************************************************************************/
/***************************** PUBLIC FUNCTIONS *******************************/
/******************************************************************************/

server *init_server(unsigned int cache_size) {
    server *sv = malloc(sizeof(*sv));
    DIE(!sv, "Malloc failed");

    sv->cache = init_lru_cache(cache_size);
    sv->documents =
        create_hash_map(100, hash_string, compare_strings, free_entry);
    sv->task_queue = q_create(sizeof(request), TASK_QUEUE_SIZE);

    return sv;
}

response *server_handle_request(server *s, request *req) {
    response *exit_code = alloc_response(MAX_RESPONSE_LENGTH, MAX_LOG_LENGTH);

    exit_code->server_id = s->id;

    char *req_doc_name = calloc(1, DOC_NAME_LENGTH + 1);
    DIE(!req_doc_name, "Calloc failed");
    memcpy(req_doc_name, req->doc_name, DOC_NAME_LENGTH + 1);

    char *req_doc_content = calloc(1, DOC_CONTENT_LENGTH + 1);
    DIE(!req_doc_content, "Calloc failed");

    if (req->doc_content)
        memcpy(req_doc_content, req->doc_content, DOC_CONTENT_LENGTH + 1);

    request *my_req = calloc(1, sizeof(*my_req));
    DIE(!my_req, "Malloc failed");

    my_req->type = req->type;
    my_req->doc_name = req_doc_name;
    my_req->doc_content = req_doc_content;

    q_enqueue(s->task_queue, my_req);

    free(my_req);

    char *operation = (req->type == EDIT_DOCUMENT) ? "EDIT" : "GET";
    sprintf(exit_code->server_response, MSG_A, operation, req->doc_name);
    sprintf(exit_code->server_log, LOG_LAZY_EXEC, s->task_queue->size);

    if (req->type == GET_DOCUMENT) {
        while (!q_is_empty(s->task_queue)) {
            ll_node_t *front = q_front(s->task_queue);
            request *curr_req = (request *)front->data;

            char *doc_name = curr_req->doc_name;
            char *doc_content = curr_req->doc_content;

            if (curr_req->type == GET_DOCUMENT) {
                response *get_exit_code =
                    server_get_document(s, doc_name);
                sprintf(exit_code->server_response,
                        "%s", get_exit_code->server_response);
                sprintf(exit_code->server_log,
                        "%s", get_exit_code->server_log);
                free(get_exit_code->server_response);
                free(get_exit_code->server_log);
                free(get_exit_code);
            } else {
                response *edit_exit_code =
                    server_edit_document(s, doc_name, doc_content);
                sprintf(exit_code->server_response,
                        "%s", edit_exit_code->server_response);
                sprintf(exit_code->server_log,
                        "%s", edit_exit_code->server_log);
                free(edit_exit_code->server_response);
                free(edit_exit_code->server_log);
                free(edit_exit_code);
            }

            free(doc_name);
            free(doc_content);
            
            q_dequeue(s->task_queue);
        }
    }

    return exit_code;
}

void free_server(server **s) {
    server *server = *s;

    free_map(&server->documents);

    free_lru_cache(&server->cache);

    queue_t *queue = server->task_queue;
    if (queue) {
        while (!q_is_empty(queue)) {
            q_dequeue(queue);
        }
    }

    free(queue);
    server->task_queue = NULL;

    free(server);
    *s = NULL;
}
