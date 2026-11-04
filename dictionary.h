#ifndef DICTIONARY_H
#define DICTIONARY_H

#include <stdlib.h>
#include <string.h>
#include <immintrin.h>
#include <stdint.h>
#include <stdio.h>

struct dictionary {
    struct dictionary_entry *entries;
    size_t size;
    size_t capacity;
} dictionary;

struct dictionary_entry {
    char *key;
    void *value;
    uint32_t hash;
} dictionary_entry;

struct dictionary_iterator {
    const char *key;
    void *value;

    struct dictionary *dict;
    size_t index;
} dictionary_iterator;

#define DICTIONARY_INITIAL_CAPACITY 16

struct dictionary *new_dictionary();
void free_dictionary(struct dictionary *dict);

void dictionary_set(struct dictionary *dict, const char *key, void *value);
void *dictionary_get(struct dictionary *dict, const char *key);
void dictionary_remove(struct dictionary *dict, const char *key);

#endif // DICTIONARY_H
