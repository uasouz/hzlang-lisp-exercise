//
// Created by hadara on 4/17/19.
//

#include <stdlib.h>
#include "alloc.h"

#ifndef EXERCISES_HASHMAP_H
#define EXERCISES_HASHMAP_H

#define HASHMAP_INITIAL_SIZE 64
/* grow / shrink by 2^2 */
#define HASHMAP_RESIZE_BITS 2
/* load factor in percent */
#define HASHMAP_LOAD_FACTOR 80

/* Structs */
struct hashmap_entry {
    struct hashmap_entry *next;
    unsigned int hash;
};

typedef int (*hashmap_cmp_fn)(const void *hashmap_cmp_fn_data,
                              const void *entry, const void *entry_or_key,
                              const void *keydata);

struct hashmap {

    struct hashmap_entry **table;

    /* User-supplied builtin to test two git-hashmap entries for equality. Shall
     * return 0 if the entries are equal. */
    hashmap_cmp_fn cmpfn;
    const void *cmpfn_data;

    /* total number of entries (0 means the hashmap is empty) */
    unsigned int size;

    unsigned int tablesize;
    unsigned int grow_at;
    unsigned int shrink_at;
};

extern void hashmap_entry_init(void *entry, unsigned int hash);

extern void hashmap_init(struct hashmap *map,
                         hashmap_cmp_fn equals_function,
                         const void *equals_function_data,
                         size_t initial_size);

extern void hashmap_free(struct hashmap *map, int free_entries);

extern void *hashmap_get(const struct hashmap *map, const void *key,
                         const void *keydata);

extern void hashmap_add(struct hashmap *map, void *entry);

extern void *hashmap_put(struct hashmap *map, void *entry);

extern void *hashmap_remove(struct hashmap *map, const void *key,
                            const void *keydata);

/*
 * Used to iterate over all entries of a hashmap. Note that it is
 * not safe to add or remove entries to the hashmap while
 * iterating.
 * NOT THREAD SAFE!
 */
struct hashmap_iter {
    struct hashmap *map;
    struct hashmap_entry *next;
    unsigned int tablepos;
};

/* Initializes a `hashmap_iter` structure. */
extern void hashmap_iter_init(struct hashmap *map, struct hashmap_iter *iter);

/* Returns the next hashmap_entry, or NULL if there are no more entries. */
extern void *hashmap_iter_next(struct hashmap_iter *iter);

/* Initializes the iterator and returns the first entry, if any. */
static inline void *hashmap_iter_first(struct hashmap *map,
                                       struct hashmap_iter *iter)
{
    hashmap_iter_init(map, iter);
    return hashmap_iter_next(iter);
}

//MEMHASH FROM GIT
extern unsigned int hash(const void *buf, size_t len);

extern unsigned int hash1a(const void *buf, size_t len);

extern uint64_t hash64(const void *buf, size_t len);

extern uint64_t hash641a(const void *buf, size_t len);

#endif //EXERCISES_HASHMAP_H
