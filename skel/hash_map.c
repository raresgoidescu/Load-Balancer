/**
 * @file hash_map.c
 * @author GOIDESCU Rares-Stefan (known.as.rares@gmail.com)
 * @date 2024-05-01
 * 
 * @copyright Copyright (c) 2024
 * 
 */

#include "hash_map.h"

void free_entry(void *entry_ptr) {
  entry_t *entry = entry_ptr;
  free(entry->key);
  free(entry->val);
  free(entry);
}

hash_map_t *create_hash_map(unsigned int n_buckets,
                            unsigned int (*hash_func)(void *),
                            bool (*compare_func)(void *, void *),
                            void (*free_key_val)(void *)) {
  hash_map_t *map = malloc(sizeof(*map));
  DIE(!map, "Malloc failed");

  map->buckets = calloc(n_buckets, sizeof(linked_list_t *));
  DIE(!map->buckets, "Calloc failed");

  for (unsigned int i = 0; i < n_buckets; ++i)
    map->buckets[i] = create_ll(sizeof(entry_t));

  map->size = 0;
  map->max_size = n_buckets;

  map->hash = *hash_func;
  map->compare = *compare_func;
  map->free_entry = *free_key_val;

  return map;
}

bool has_key(hash_map_t *map, void *key) {
  unsigned int index = map->hash(key) % map->max_size;

  ll_node_t *curr = map->buckets[index]->head;
  while (curr) {
    if (!map->compare(key, ((entry_t *)curr->data)->key))
      return true;
    curr = curr->next;
  }

  return false;
}

void *get_value(hash_map_t *map, void *key) {
  unsigned int index = map->hash(key) % map->max_size;
  ll_node_t *curr = map->buckets[index]->head;

  while (curr) {
    if (!map->compare(key, ((entry_t *)curr->data)->key))
      return ((entry_t *)curr->data)->val;
    curr = curr->next;
  }

  return NULL;
}

void add_entry(hash_map_t *map, void *key, unsigned int key_size, void *val,
               unsigned int val_size) {
  unsigned int index = map->hash(key) % map->max_size;
  linked_list_t *curr_bucket = map->buckets[index];

  entry_t new_entry; // malloc(sizeof(entry_t));
  // DIE(!new_entry, "Malloc failed");

  new_entry.key = calloc(1, key_size);
  DIE(!new_entry.key, "Calloc failed");

  new_entry.val = calloc(1, val_size);
  DIE(!new_entry.val, "Calloc failed");

  memcpy(new_entry.key, key, key_size);
  memcpy(new_entry.val, val, val_size);

  add_ll_nth_node(curr_bucket, curr_bucket->size, &new_entry);

  map->size++;
}

void remove_entry(hash_map_t *map, void *key) {
  unsigned int index = map->hash(key) % map->max_size;

  ll_node_t *curr = map->buckets[index]->head;
  ll_node_t *prev = NULL;

  while (curr) {
    entry_t *curr_entry = curr->data;
    if (!map->compare(key, curr_entry->key)) {
      break;
    }
    prev = curr;
    curr = curr->next;
  }

  if (!curr) {
    return;
  } else {
    map->free_entry(curr->data);

    if (!prev) {
      map->buckets[index]->head = curr->next;
    } else {
      prev->next = curr->next;
    }

    map->buckets[index]->size--;

    free(curr);
    curr = NULL;
  }
}

void free_map(hash_map_t **map_ref) {
  hash_map_t *map = *map_ref;
  for (unsigned int i = 0; i < map->max_size; ++i) {
    while (map->buckets[i]->size) {
      map->free_entry(map->buckets[i]->head->data);
      ll_node_t *_to_free = remove_ll_nth_node(map->buckets[i], 0);
      free(_to_free);
    }
    free_ll(&map->buckets[i]);
  }
  free(map->buckets);
  free(map);
  *map_ref = NULL;
}

void print_map(hash_map_t *map, FILE *stream) {
  for (unsigned int i = 0; i < map->max_size; ++i) {
    fprintf(stream, "[Bucket %u]\n", i);
    ll_node_t *curr = map->buckets[i]->head;
    while (curr) {
      fprintf(stream, "\tKey: %s\n\tVal: %s\n",
              (char *)((entry_t *)curr->data)->key,
              (char *)((entry_t *)curr->data)->val);
      curr = curr->next;
    }
    fprintf(stream, "-----\n");
  }
}
