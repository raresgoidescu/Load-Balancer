/*
 * Copyright (c) 2024, GOIDESCU Rares-Stefan 312CA <known.as.rares@gmail.com>
 */

#include <stdbool.h>

#include "simply_linked_list.h"

typedef struct hash_map_t {
    /* Array de liste */
    linked_list_t **buckets;
    /* Numarul de entry-uri */
    unsigned int size;
    /* Numarul maxim de bucket-uri */
    unsigned int map_max_size;
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