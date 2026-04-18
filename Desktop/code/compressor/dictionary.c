#include "dictionary.h"

struct dictionary *new_dictionary()
{
    struct dictionary *dict = malloc(sizeof(struct dictionary));
    if (!dict) {
        perror("malloc");
        exit(1);
    }
    dict->size = 0;
    dict->capacity = DICTIONARY_INITIAL_CAPACITY;
    dict->entries = calloc(dict->capacity, sizeof(struct dictionary_entry));
    if (!dict->entries) {
        perror("calloc");
        exit(1);
    }
    return dict;
}

void free_dictionary(struct dictionary *dict)
{
    for (size_t i = 0; i < dict->capacity; i++) {
        free(dict->entries[i].key);
    }
    free(dict->entries);
    free(dict);
}

static inline __m256i rotl32(__m256i x, int r) {
    return _mm256_or_si256(_mm256_slli_epi32(x, r), _mm256_srli_epi32(x, 32-r));
}

static inline uint32_t rotl32_scalar(uint32_t x, int r) {
    return (x << r) | (x >> (32 - r));
}

static inline uint32_t read32(const void *ptr) {
    uint32_t v;
    memcpy(&v, ptr, sizeof(v));
    return v;
}

static inline size_t strlen_avx2(const char* str) {
    const char* p = str;

    while ((uintptr_t)p % 32 != 0) {
        if (*p == '\0') return p - str;
        p++;
    }

    __m256i zeros = _mm256_setzero_si256();
    while (1) {
        uintptr_t page_offset = (uintptr_t)p % 4096;
        if (page_offset <= 4096 - 32) {
            __m256i chunk = _mm256_loadu_si256((const __m256i*)p);
            __m256i cmp = _mm256_cmpeq_epi8(chunk, zeros);
            uint32_t mask = _mm256_movemask_epi8(cmp);

            if (mask != 0) {
                return (p - str) + __builtin_ctz(mask);
            }

            p += 32;
        } else {
            for (int i = 0; i < 32; i++) {
                if (p[i] == '\0') return (p - str) + i;
            }
            p += 32;
        }
    }
}

// Implements XXHash32 algorithm for hashing keys
uint32_t hash_key(const char *key)
{
    const uint8_t *data = (const uint8_t *)key;
    size_t len = strlen_avx2(key);

    const uint32_t PRIME1 = 0x9E3779B1;
    const uint32_t PRIME2 = 0x85EBCA77;
    const uint32_t PRIME3 = 0xC2B2AE3D;
    const uint32_t PRIME4 = 0x27D4EB2F;
    const uint32_t PRIME5 = 0x165667B1;
    const uint32_t seed = 0;

    const uint8_t *ptr = data;
    const uint8_t *end = data + len;
    uint32_t hash;

    if (len >= 16) {
        uint32_t v1 = seed + PRIME1 + PRIME2;
        uint32_t v2 = seed + PRIME2;
        uint32_t v3 = seed;
        uint32_t v4 = seed - PRIME1;

        const uint8_t *limit = end - 16;

        if (len >= 32) {
            __m256i acc = _mm256_set_epi32(
                0, 0, 0, 0,
                (int32_t)v4, (int32_t)v3, (int32_t)v2, (int32_t)v1
            );
            const __m256i PRIME1v = _mm256_set1_epi32((int32_t)PRIME1);
            const __m256i PRIME2v = _mm256_set1_epi32((int32_t)PRIME2);

            const uint8_t *avx_limit = end - 32;
            while (ptr <= avx_limit) {
                __m256i data_vec = _mm256_loadu_si256((const __m256i*)ptr);
                acc = _mm256_add_epi32(acc, _mm256_mullo_epi32(data_vec, PRIME2v));
                acc = rotl32(acc, 13);
                acc = _mm256_mullo_epi32(acc, PRIME1v);
                ptr += 32;
            }
            __m128i low = _mm256_castsi256_si128(acc);
            uint32_t vs[4];
            _mm_storeu_si128((__m128i*)vs, low);
            v1 = vs[0]; v2 = vs[1]; v3 = vs[2]; v4 = vs[3];
        }
        
        while (ptr <= limit) {
            v1 = rotl32_scalar(v1 + (*(uint32_t*)(ptr + 0)) * PRIME2, 13) * PRIME1;
            v2 = rotl32_scalar(v2 + (*(uint32_t*)(ptr + 4)) * PRIME2, 13) * PRIME1;
            v3 = rotl32_scalar(v3 + (*(uint32_t*)(ptr + 8)) * PRIME2, 13) * PRIME1;
            v4 = rotl32_scalar(v4 + (*(uint32_t*)(ptr + 12)) * PRIME2, 13) * PRIME1;
            ptr += 16;
        }

        hash = rotl32_scalar(v1, 1) + rotl32_scalar(v2, 7)
                + rotl32_scalar(v3, 12) + rotl32_scalar(v4, 18);

    } else {
        hash = seed + PRIME5;
    }

    hash += (uint32_t)len;

    while (ptr + 4 <= end) {
        uint32_t k;
        memcpy(&k, ptr, sizeof(k));

        hash += k * PRIME3;
        hash = rotl32_scalar(hash, 17) * PRIME4;

        ptr += 4;
    }

    while (ptr < end) {
        hash += (*ptr) * PRIME5;
        hash = rotl32_scalar(hash, 11) * PRIME1;
        ptr++;
    }

    hash ^= hash >> 15;
    hash *= PRIME2;
    hash ^= hash >> 13;
    hash *= PRIME3;
    hash ^= hash >> 16;
    return hash;
}

static void dictionary_insert_raw(struct dictionary *dict,
                                  char *key,
                                  void *value,
                                  uint32_t hash)
{
    size_t index = hash & (dict->capacity - 1);

    while (dict->entries[index].key != NULL) {
        index = (index + 1) & (dict->capacity - 1);
    }

    dict->entries[index].key = key;
    dict->entries[index].value = value;
    dict->entries[index].hash = hash;
    dict->size++;
}

static void dictionary_resize(struct dictionary *dict) {
    size_t old_capacity = dict->capacity;
    struct dictionary_entry *old_entries = dict->entries;

    dict->capacity *= 2;
    dict->entries = calloc(dict->capacity, sizeof(struct dictionary_entry));
    dict->size = 0;

    for (size_t i = 0; i < old_capacity; i++) {
        if (old_entries[i].key != NULL) {
            dictionary_insert_raw(dict, old_entries[i].key, old_entries[i].value, old_entries[i].hash);
        }
    }

    free(old_entries);
}

// Will overwrite existing value if key already exists
void dictionary_set(struct dictionary *dict, const char *key, void *value)
{
    if (value == NULL) {
        fprintf(stderr, "dictionary_set: NULL value is not allowed\n");
        return;
    }

    if (dict->size * 2 >= dict->capacity) {
        dictionary_resize(dict);
    }

    uint32_t hash = hash_key(key);
    size_t index = (size_t)(hash & (dict->capacity - 1));

    while (dict->entries[index].key != NULL) {
        if (dict->entries[index].hash == hash && strcmp(dict->entries[index].key, key) == 0) {
            dict->entries[index].value = value;
            return;
        }
        index = (index + 1) & (dict->capacity - 1);
    }
    dict->entries[index].key = strdup(key);
    dict->entries[index].value = value;
    dict->entries[index].hash = hash;

    dict->size++;
}

void *dictionary_get(struct dictionary *dict, const char *key)
{
    uint32_t hash = hash_key(key);
    size_t index = (size_t)(hash & (dict->capacity - 1));
    size_t i = index;
    for (;;) {
        if (dict->entries[i].key == NULL) return NULL;

        if (dict->entries[i].hash == hash &&
            strcmp(dict->entries[i].key, key) == 0)
            return dict->entries[i].value;

        i = (i + 1) & (dict->capacity - 1);
        if (i == index) return NULL;
    }
    return NULL;
}

void dictionary_remove(struct dictionary *dict, const char *key)
{
    if (dict->size == 0) {
        fprintf(stderr, "Dictionary is empty\n");
        return;
    }

    uint32_t hash = hash_key(key);
    size_t index = (size_t)(hash & (dict->capacity - 1));

    size_t i = index;
    for (;;) {
        if (dict->entries[i].key == NULL) return; // not found

        if (dict->entries[i].hash == hash &&
            strcmp(dict->entries[i].key, key) == 0)
            break;

        i = (i + 1) & (dict->capacity - 1);
        if (i == index) return; // full loop
    }
    index = i;

    free(dict->entries[index].key);

    dict->entries[index].key = NULL;
    dict->entries[index].value = NULL;
    dict->size--;

    size_t hole = index;
    size_t next = (hole + 1) & (dict->capacity - 1);

    while (dict->entries[next].key != NULL) {
        size_t natural = dict->entries[next].hash & (dict->capacity - 1);
        int should_move;
        if (hole < next) {
            should_move = (natural <= hole) || (natural > next);
        } else {
            should_move = (natural <= hole) && (natural > next);
        }

        if (should_move) {
            dict->entries[hole] = dict->entries[next];
            dict->entries[next].key = NULL;
            dict->entries[next].value = NULL;
            hole = next;
        }

        next = (next + 1) & (dict->capacity - 1);
        
        if (next == hole) break;
    }
}
