/**
 * @file hash_map.h
 * @author GOIDESCU Rares-Stefan (known.as.rares@gmail.com)
 * @date 2024-05-01
 * 
 * @copyright Copyright (c) 2024
 * 
 */

#ifndef __HASH_MAP__
#define __HASH_MAP__

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include "utils.h"
#include "simply_linked_list.h"

typedef struct hash_map_t {
    /* Array de liste */
    linked_list_t **buckets;
    /* Numarul de entry-uri */
    unsigned int size;
    /* Numarul maxim de bucket-uri */
    unsigned int max_size;
    /* Functie care aplica un algoritm de hashing pe o cheie */
    unsigned int (*hash)(void *);
    /* Functie care compara doua chei */
    bool (*compare)(void *, void *);
    /* Functie care elibereaza memoria ocupata de un entry in dictionar */
    void (*free_entry)(void *);
} hash_map_t;

typedef struct entry_t {
    void *key;
    void *val;
} entry_t;

/**
 * @brief Functie care elibereaza memoria unui entry (key, val)
 */
void free_entry(void *entry_ptr);

/**
 * @brief Functie care aloca memoria necesara unui hash map
 *  si initializeaza campurile acestuia
 * @return (hash_map_t *) - pointer catre noul dictionar
 */
hash_map_t *create_hash_map(unsigned int n_buckets,
                            unsigned int (*hash_func)(void *),
                            bool (*compare_func)(void *, void *),
                            void (*free_key_val)(void *));

/**
 * @brief Functie care determina daca un dictionar contine o cheie
 * @return - true if has key, false otherwise
 */
bool has_key(hash_map_t *map, void *key);

/**
 * @brief Functie care intoarce valoarea corespunzatoare cheii
 */
void *get_value(hash_map_t *map, void *key);

/**
 * @brief Functie care adauga un entry in dictionar
 */
void add_entry(hash_map_t *map,
               void *key, unsigned int key_size,
               void *val, unsigned int val_size);

/**
 * @brief Functie care elimina un entry din dictionar
 */
void remove_entry(hash_map_t *map, void *key);

/**
 * @brief Functie care elibereaza memoria folosita de un dictionar
 */
void free_map(hash_map_t **map_ref);

void copy_entries(hash_map_t *src, hash_map_t *dst);

/**
 * @brief Functie care afiseaza continutul unui dictionar cu (str, str)
 */
void print_map(hash_map_t *map, FILE *stream);

#endif /* __HASH_MAP__ */
