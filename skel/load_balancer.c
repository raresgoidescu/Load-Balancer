/**
 * @file load_balancer.c
 * @author GOIDESCU Rares-Stefan (known.as.rares@gmail.com)
 * @date 2024-05-06
 * 
 * @copyright Copyright (c) 2024
 * 
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


/******************************************************************************/
/***************************** PRIVATE FUNCTIONS ******************************/
/******************************************************************************/

/**
 * @brief Functie care adauga un server pe ring,
 *  mentinandu-le sortate.
 * @param arr - server ring
 * @param size - numarul curent de servere in ring
 * @param to_add - serverul de inserat
 */
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

/**
 * @brief Functie care trimite o cerere goala catre un server
 *  cu scopul de a da trigger la coada de cereri
 */
static
void send_empty_request(server *s) {
    request *dummy_get = malloc(sizeof(*dummy_get));
    DIE(!dummy_get, "Malloc failed");

    dummy_get->type = GET_DOCUMENT;
    dummy_get->doc_name = NULL; /* Black magic pentru a da trigger unui goto */
    dummy_get->doc_content = NULL;

    /* Daca nu are cereri in coada, nu are sens sa trimitem */
    if (!q_is_empty(s->requests)) {
        response *dummy = server_handle_request(s, dummy_get);
        free(dummy->server_response);
        free(dummy->server_log);
        free(dummy);
    }

    free(dummy_get);
}

/**
 * @brief Copiaza documentele de pe un hash map pe altul
 */
static
void copy_entries(hash_map_t *src, hash_map_t *dst) {
	if (!src || !dst)
		return;

	for (unsigned int i = 0; i < src->max_size; ++i) {
		ll_node_t *curr = src->buckets[i]->head;
		while (curr) {
			entry_t *curr_entry = curr->data;
			ll_node_t *spare_pointer = curr->next;
			unsigned long int key_len = strlen(curr_entry->key) + 1;
			unsigned long int val_len = strlen(curr_entry->val) + 1;
			add_entry(dst, curr_entry->key, key_len, curr_entry->val, val_len);
			remove_entry(src, curr_entry->key);
			curr = spare_pointer;
		}
	}
}


/******************************************************************************/
/***************************** PUBLIC FUNCTIONS *******************************/
/******************************************************************************/

load_balancer *init_load_balancer(bool enable_vnodes) {
    /* So no bonus? No bonus :( */
    (void)enable_vnodes; /* Evade compiler warnings */

    load_balancer *load_balancer = malloc(sizeof(*load_balancer));
    DIE(!load_balancer, "Malloc failed");

    load_balancer->server_ring = calloc(MAX_SERVERS, sizeof(server *));
    DIE(!load_balancer->server_ring, "Calloc failed");

    for (int i = 0; i < MAX_SERVERS; ++i)
        load_balancer->server_ring[i] = NULL; /* Better safe than sorry */

    load_balancer->number_of_servers = 0;

    load_balancer->hash_function_servers = *hash_uint;
    load_balancer->hash_function_docs = *hash_string;

    return load_balancer;
}

void loader_add_server(load_balancer* main, int server_id, int cache_size) {
    server *new_server = init_server(cache_size);
    new_server->id = server_id;

    insert_keep_sorted(main->server_ring, &main->number_of_servers, new_server);

    /* Gasesc unde a fost inserat noul server pe ring */
    int dst_idx = -1;
    for (int i = 0; i < main->number_of_servers; ++i)
        if (main->server_ring[i]->id == server_id) {
            dst_idx = i;
            break;
        }

    /* Daca nu a fost adaugat sau e un singur server in ring, ne oprim */
    if (dst_idx == -1 || main->number_of_servers == 1)
        return;

    /* Determinam care ar fi serverul care poate "dona" fisiere */
    int src_idx = (dst_idx == main->number_of_servers - 1) ? 0 : dst_idx + 1;

    /* Daca, cumva, este unul si acelasi server, ne oprim */
    if (dst_idx == src_idx)
        return;

    server *dst_s = main->server_ring[dst_idx];
    server *src_s = main->server_ring[src_idx];

    unsigned int dst_hash = hash_uint(&server_id);
    unsigned int src_hash = hash_uint(&src_s->id);
    unsigned int last_server_hash =
        hash_uint(&main->server_ring[main->number_of_servers - 1]->id);

    hash_map_t *src_db = src_s->data_base;

    send_empty_request(src_s);

    /* Verificam toate documentele din "posibilul donator" */
    for (unsigned int i = 0; i < src_db->max_size; ++i) {
        ll_node_t *curr = src_db->buckets[i]->head;
        while (curr) {
            entry_t *curr_entry = curr->data;
            ll_node_t *spare_pointer = curr->next;

            long unsigned int key_length = strlen(curr_entry->key) + 1;
            long unsigned int val_length = strlen(curr_entry->val) + 1;

            unsigned int key_hash = hash_string(curr_entry->key);

            /* "Only God knows why this works" si imi e frica sa schimb ceva */
            /* In principiu, sunt cazuri limita si base-ul */
            /* Ar fi putut fi scris mai frumos, dar cum am zis, I'm scared */
            if (!src_idx && key_hash > src_hash && key_hash <= dst_hash) {
                /*
                 * Daca serverul sursa e la inceput, verificam daca exista
                 * vreun fisier care are hash-ul mai mare decat hash-ul
                 * serverului care il detine si mai mic decat hash-ul
                 * serverului care l-ar putea prelua
                 */
                add_entry(dst_s->data_base,
                          curr_entry->key, key_length,
                          curr_entry->val, val_length);

                lru_cache_remove(src_s->cache, curr_entry->key);

                remove_entry(src_db, curr_entry->key);
            } else if (key_hash > last_server_hash && !dst_idx) {
                /*
                 * Daca serverul adaugat este la inceput, ne uitam la
                 * potentialul donator si verificam daca tine fisiere cu
                 * hash mai mare decat hash-ul ultimului server din ring, caz
                 * in care trebuie mutate pe primul server din ring
                 */
                add_entry(dst_s->data_base,
                          curr_entry->key, key_length,
                          curr_entry->val, val_length);

                lru_cache_remove(src_s->cache, curr_entry->key);

                remove_entry(src_db, curr_entry->key);
            } else if (src_idx && key_hash <= dst_hash) {
                /*
                 * Cazul de baza, cand serverul adaugat nu este la extremitati
                 */
                add_entry(dst_s->data_base,
                          curr_entry->key, key_length,
                          curr_entry->val, val_length);

                lru_cache_remove(src_s->cache, curr_entry->key);

                remove_entry(src_db, curr_entry->key);
            }
            curr = spare_pointer;
        }
    }
}

void loader_remove_server(load_balancer* main, int server_id) {
    if (main->number_of_servers == 0)
        return;

    /* Cautam serverul pe care vrem sa il eliminam din ring */
    int src_idx = -1;
    for (int i = 0; i < main->number_of_servers; ++i)
        if (main->server_ring[i]->id == server_id) {
            src_idx = i;
            break;
        }

    /* Daca nu a fost gasit, nu facem nimic */
    if (src_idx == -1)
        return;

    /* Determinam serverul care ar putea prelua documentele */
    int dst_idx = (src_idx == main->number_of_servers - 1) ? 0 : src_idx + 1;

    server *dst_s = main->server_ring[dst_idx];
    server *src_s = main->server_ring[src_idx];

    /*
     * Trimitem un request gol catre serverul pe care il stergem 
     * pentru a goli coada de cereri
     */
    send_empty_request(src_s);

    /* Mutam documentele */
    copy_entries(src_s->data_base, dst_s->data_base);

    free_server(&src_s);

    /* Actualizam ring-ul de servere */
    for (int i = src_idx; i < MAX_SERVERS - 1; ++i)
        main->server_ring[i] = main->server_ring[i + 1];

    main->server_ring[MAX_SERVERS - 1] = NULL;

    main->number_of_servers--;
}

response *loader_forward_request(load_balancer* main, request *req) {
    unsigned int doc_hash = hash_string(req->doc_name);

    for (int i = 0; i < main->number_of_servers; ++i)
        if (doc_hash < hash_uint(&main->server_ring[i]->id))
            return server_handle_request(main->server_ring[i], req);

    return server_handle_request(main->server_ring[0], req);
}

void free_load_balancer(load_balancer** main) {
    load_balancer *m = *main;
    for (int i = 0; i < MAX_SERVERS; ++i) {
    #ifdef DEBUG
        if (m->server_ring[i]) {
            printf("[%.3d] %d\n", i, m->server_ring[i]->id);
            if (has_key(m->server_ring[i]->data_base, "financial_rich.txt"))
                printf("\tfinancial_rich.txt\n");
        }
    #endif
        free_server(&m->server_ring[i]);
    }

    free(m->server_ring);

    free(m);

    *main = NULL;
}
