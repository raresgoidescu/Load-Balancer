/**
 * @file server.c
 * @author GOIDESCU Rares-Stefan (known.as.rares@gmail.com)
 * @date 2024-05-06
 * 
 * @copyright Copyright (c) 2024
 * 
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "server.h"
#include "constants.h"
#include "doubly_linked_list.h"
#include "hash_map.h"
#include "lru_cache.h"
#include "queue.h"
#include "simply_linked_list.h"
#include "utils.h"

/******************************************************************************/
/***************************** PRIVATE FUNCTIONS ******************************/
/******************************************************************************/

/**
 * @brief Functie care aloca memorie pentru o structura de tipul
 *  response
 */
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

/**
 * NOTE: Aceasta urmeaza flow-ul descris in imaginea din enuntul teme.
 * @brief Functie care proceseaza o cerere de editare a unui document.
 * @return response* - care contine informatii despre procesarea cererii
 */
static response
*server_edit_document(server *s, char *doc_name, char *doc_content) {
    response *exit_code = alloc_response(MAX_RESPONSE_LENGTH, MAX_LOG_LENGTH);
    exit_code->server_id = s->id;

    long unsigned int name_len = strlen(doc_name) + 1;
    long unsigned int content_len = strlen(doc_content) + 1;

    if (has_key(s->cache->map, doc_name) && has_key(s->data_base, doc_name)) {
        sprintf(exit_code->server_response, MSG_B, doc_name);
        sprintf(exit_code->server_log, LOG_HIT, doc_name);

        remove_entry(s->data_base, doc_name);
        add_entry(s->data_base, doc_name, name_len, doc_content, content_len);

        dll_node_t *dummy = lru_cache_get(s->cache, doc_name);
        doc_data_t *dummy_data = dummy->data;
        free(dummy_data->doc_content);
        dummy_data->doc_content = strdup(doc_content);

        printf(GENERIC_MSG, exit_code->server_id,
               exit_code->server_response, exit_code->server_id,
               exit_code->server_log);
    } else {
        if (has_key(s->data_base, doc_name)) {
            sprintf(exit_code->server_response, MSG_B, doc_name);

            remove_entry(s->data_base, doc_name);
            add_entry(s->data_base,
                      doc_name, name_len,
                      doc_content, content_len);
        } else {
            sprintf(exit_code->server_response, MSG_C, doc_name);
            sprintf(exit_code->server_log, LOG_MISS, doc_name);

            add_entry(s->data_base,
                      doc_name, name_len,
                      doc_content, content_len);
        }

        if (lru_cache_is_full(s->cache)) {
            void *oldest_doc_name;
            lru_cache_put(s->cache, doc_name, doc_content, &oldest_doc_name);
            sprintf(exit_code->server_log,
                    LOG_EVICT, doc_name, (char *)oldest_doc_name);
            printf(GENERIC_MSG, exit_code->server_id,
                   exit_code->server_response, exit_code->server_id,
                   exit_code->server_log);
            free(oldest_doc_name);
        } else {
            void *dummy;
            lru_cache_put(s->cache, doc_name, doc_content, &dummy);
            sprintf(exit_code->server_log, LOG_MISS, doc_name);
            printf(GENERIC_MSG, exit_code->server_id,
                   exit_code->server_response, exit_code->server_id,
                   exit_code->server_log);
        }
    }

    return exit_code;
}

/**
 * NOTE: Aceasta urmeaza flow-ul descris in imaginea din enuntul teme.
 * @brief Functie care proceseaza o cerere de obtinere a unui document.
 * @return response* - care contine informatii despre procesarea cererii
 */
static response
*server_get_document(server *s, char *doc_name) {
    response *exit_code = alloc_response(MAX_RESPONSE_LENGTH, MAX_LOG_LENGTH);

    if (has_key(s->cache->map, doc_name) && has_key(s->data_base, doc_name)) {
        dll_node_t *doc_node = lru_cache_get(s->cache, doc_name);
        doc_data_t *doc_data = (doc_data_t *)doc_node->data;
        char *doc_content = doc_data->doc_content;
        sprintf(exit_code->server_response, "%s", doc_content);
        sprintf(exit_code->server_log, LOG_HIT, doc_name);
    } else {
        if (has_key(s->data_base, doc_name)) {
            char *doc_content = (char *)get_value(s->data_base, doc_name);
            sprintf(exit_code->server_response, "%s", doc_content);

            if (lru_cache_is_full(s->cache)) {
                void *oldest_doc_name;
                lru_cache_put(s->cache,
                              doc_name, doc_content,
                              &oldest_doc_name);
                sprintf(exit_code->server_log,
                        LOG_EVICT, doc_name, (char*)oldest_doc_name);
                free(oldest_doc_name);
            } else {
                void *dummy;
                lru_cache_put(s->cache, doc_name, doc_content, &dummy);
                sprintf(exit_code->server_log, LOG_MISS, doc_name);
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

void free_request_fields(void **r_ref) {
    request *r = *r_ref;
    free(r->doc_name);
    free(r->doc_content);
    free(r);
    *r_ref = NULL;
}

server *init_server(unsigned int cache_size) {
    server *s = malloc(sizeof(*s));
    DIE(!s, "Malloc failed");

    s->cache = init_lru_cache(cache_size);
    s->data_base = create_hash_map(229,
                                   hash_string,
                                   compare_strings,
                                   free_entry);
    s->requests = q_create(sizeof(request),
                           TASK_QUEUE_SIZE,
                           free_request_fields);

    return s;
}

#ifdef DEBUG

void print_cache_order(lru_cache *cache);

#endif

response *server_handle_request(server *s, request *req) {
    response *exit_code = alloc_response(MAX_RESPONSE_LENGTH, MAX_LOG_LENGTH);
    exit_code->server_id = s->id;

    /*
     * Facem o copie a request-ului pentru a nu folosi in continuare
     * zone de memorie care ar putea fi eliberate pe parcurs si o punem
     * in coada de cereri
     */
    request *req_copy = malloc(sizeof(*req_copy));
    DIE(!req_copy, "Malloc failed");
    req_copy->type = req->type;
    if (req->doc_content)
        req_copy->doc_content = strdup(req->doc_content);
    else
        req_copy->doc_content = NULL;
    if (req->doc_name)
        req_copy->doc_name = strdup(req->doc_name);
    else
        goto null_req_doc_name;
    /* 
     * In caz ca nu primim un nume de document,
     * nu punem request-ul in queue, si doar dam
     * pop cozii de cereri
     */
    q_enqueue(s->requests, req_copy);

    char *operation = (req_copy->type == EDIT_DOCUMENT) ? "EDIT" : "GET";
    sprintf(exit_code->server_response, MSG_A, operation, req_copy->doc_name);
    sprintf(exit_code->server_log, LOG_LAZY_EXEC, s->requests->size);

    if (req->type == GET_DOCUMENT) {
null_req_doc_name:
        /* Dam pop cererilor din coada */
        while (!q_is_empty(s->requests)) {
            ll_node_t *front = s->requests->front;
            request *see = front->data;

            if (see->type == GET_DOCUMENT) {
                response *get_exit_code = server_get_document(s, see->doc_name);
                sprintf(exit_code->server_response,
                        "%s", get_exit_code->server_response);
                sprintf(exit_code->server_log,
                        "%s", get_exit_code->server_log);
                free(get_exit_code->server_response);
                free(get_exit_code->server_log);
                free(get_exit_code);
            } else {
                response *edit_exit_code =
                    server_edit_document(s, see->doc_name, see->doc_content);
                sprintf(exit_code->server_response,
                        "%s", edit_exit_code->server_response);
                sprintf(exit_code->server_log,
                        "%s", edit_exit_code->server_log);
                free(edit_exit_code->server_response);
                free(edit_exit_code->server_log);
                free(edit_exit_code);
            }

        #ifdef DEBUG
            printf("cache_size: %d\n", s->cache->map->size);
            print_cache_order(s->cache);
        #endif
            q_dequeue(s->requests);
        }
    }

    free(req_copy);

    return exit_code;
}

#ifdef DEBUG

void print_cache_order(lru_cache *cache) {
    if (!cache->data)
        return;

    dll_node_t *curr = cache->data->head;

    printf("\n================ Newest ================\n");

    for (unsigned int i = 0; i < cache->data->size; i++) {
        doc_data_t *doc_data = curr->data;
        printf("| \t%-*s |\n", 30, doc_data->doc_name);
        curr = curr->next;
    }

    printf("================ Oldest ================\n\n");
}

#endif

void free_server(server **s) {
    if (!*s || !s)
        return;
    server *server = *s;
    free_map(&server->data_base);
    free_lru_cache(&server->cache);
    queue_t *q = server->requests;
    if (q) {
        while (!q_is_empty(q)) {
            q_dequeue(q);
        }
    }
    free(q);
    free(server);
    q = NULL;
    *s = NULL;
}
