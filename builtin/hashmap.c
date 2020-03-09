
#include "hashmap.h"

#define FNV32_BASE ((unsigned int) 0x811c9dc5)
#define FNV32_PRIME ((unsigned int) 0x01000193)

#define FNV64_BASE ((uint64_t) 0xcbf29ce484222325)
#define FNV64_PRIME ((uint64_t) 0x100000001b3)

//FNV-1 32 Bit
unsigned int hash(const void *buf, size_t len) {
    unsigned int hash = FNV32_BASE;
    unsigned char *ucbuf = (unsigned char *) buf;
    while (len--) {
        unsigned int c = *ucbuf++;
        hash = (hash * FNV32_PRIME) ^ c;
    }
    return hash;
}

//FNV-1a 32 Bit
//To test
unsigned int hash1a(const void *buf, size_t len) {
    unsigned int hash = FNV32_BASE;
    unsigned char *ucbuf = (unsigned char *) buf;
    while (len--) {
        unsigned int c = *ucbuf++;
        hash = hash ^ c;
        hash = (hash * FNV32_PRIME);
    }
    return hash;
}

//FNV-1 64 Bit
//TO Test
uint64_t hash64(const void *buf, size_t len) {
    uint64_t hash = FNV64_BASE;
    unsigned char *ucbuf = (unsigned char *) buf;
    while (len--) {
        uint64_t c = *ucbuf++;
        hash = (hash * FNV64_PRIME) ^ c;
    }
    return hash;
}

//FNV-1a 64 Bit
//TO Test
uint64_t hash641a(const void *buf, size_t len) {
    uint64_t hash = FNV64_BASE;
    unsigned char *ucbuf = (unsigned char *) buf;
    while (len--) {
        uint64_t c = *ucbuf++;
        hash = hash ^ c;
        hash = (hash * FNV64_PRIME);
    }
    return hash;
}

void hashmap_entry_init(void *entry, unsigned int hash) {
    struct hashmap_entry *e = entry;
    e->hash = hash;
    e->next = NULL;
}

static void alloc_table(struct hashmap *map, unsigned int size) {
    map->tablesize = size;
    map->table = xcalloc(size, sizeof(struct hashmap_entry *));

    /* calculate resize thresholds for new size */
    map->grow_at = (unsigned int) ((uint64_t) size * HASHMAP_LOAD_FACTOR / 100);
    if (size <= HASHMAP_INITIAL_SIZE)
        map->shrink_at = 0;
    else
        /*
         * The shrink-threshold must be slightly smaller than
         * (grow-threshold / resize-factor) to prevent erratic resizing,
         * thus we divide by (resize-factor + 1).
         */
        map->shrink_at = map->grow_at / ((1 << HASHMAP_RESIZE_BITS) + 1);
}

/* Stub awalys true function in case no compare function is provided by the user */
static int always_equal(const void *unused_cmp_data,
                        const void *unused1,
                        const void *unused2,
                        const void *unused_keydata) {
    return 0;
}

void hashmap_init(struct hashmap *map, hashmap_cmp_fn entry_cmp_function,
                  const void *cmp_function_data, size_t initial_size) {
    unsigned int size = HASHMAP_INITIAL_SIZE;

    /* Initialize struct with all bits set to 0 */
    memset(map, 0, sizeof(*map));

    map->cmpfn = entry_cmp_function ? entry_cmp_function : always_equal;
    map->cmpfn_data = cmp_function_data;

    /* calculate initial table size and allocate the table */
    initial_size = (unsigned int) ((uint64_t) initial_size * 100
                                   / HASHMAP_LOAD_FACTOR);
    while (initial_size > size)
        size <<= HASHMAP_RESIZE_BITS;
    alloc_table(map, size);

}

void hashmap_free(struct hashmap *map, int free_entries)
{
    if (!map || !map->table)
        return;
    if (free_entries) {
        struct hashmap_iter iter;
        struct hashmap_entry *e;
        hashmap_iter_init(map, &iter);
        while ((e = hashmap_iter_next(&iter)))
            free(e);
    }
    free(map->table);
    memset(map, 0, sizeof(*map));
}

static inline unsigned int bucket(const struct hashmap *map,
                                  const struct hashmap_entry *key) {
    return key->hash & (map->tablesize - 1);
}

static void rehash(struct hashmap *map, unsigned int newsize) {
    unsigned int i, oldsize = map->tablesize;
    struct hashmap_entry **oldtable = map->table;

    alloc_table(map, newsize);
    for (i = 0; i < oldsize; i++) {
        struct hashmap_entry *e = oldtable[i];
        while (e) {
            struct hashmap_entry *next = e->next;
            unsigned int b = bucket(map, e);
            e->next = map->table[b];
            map->table[b] = e;
            e = next;
        }
    }
    free(oldtable);
}

void hashmap_add(struct hashmap *map, void *entry) {
    unsigned int b = bucket(map, entry);

    /* add entry */
    ((struct hashmap_entry *) entry)->next = map->table[b];
    map->table[b] = entry;

    /* fix size and rehash if appropriate */
    map->size++;
    if (map->size > map->grow_at)
        rehash(map, map->tablesize << HASHMAP_RESIZE_BITS);
}

static inline int entry_equals(const struct hashmap *map,
                               const struct hashmap_entry *e1, const struct hashmap_entry *e2,
                               const void *keydata) {
    return (e1 == e2) ||
           (e1->hash == e2->hash &&
            !map->cmpfn(map->cmpfn_data, e1, e2, keydata));
}

static inline struct hashmap_entry **find_entry_ptr(const struct hashmap *map,
                                                    const struct hashmap_entry *key, const void *keydata) {
    struct hashmap_entry **e = &map->table[bucket(map, key)];
    while (*e && !entry_equals(map, *e, key, keydata))
        e = &(*e)->next;
    return e;
}

void *hashmap_get(const struct hashmap *map, const void *key, const void *keydata) {
    return *find_entry_ptr(map, key, keydata);
}

void *hashmap_remove(struct hashmap *map, const void *key, const void *keydata) {
    struct hashmap_entry *old;
    struct hashmap_entry **e = find_entry_ptr(map, key, keydata);
    if (!*e)
        return NULL;

    /* remove existing entry */
    old = *e;
    *e = old->next;
    old->next = NULL;

    /* fix size and rehash if appropriate */
    map->size--;
    if (map->size < map->shrink_at)
        rehash(map, map->tablesize >> HASHMAP_RESIZE_BITS);

    return old;
}

void *hashmap_put(struct hashmap *map, void *entry)
{
    struct hashmap_entry *old = hashmap_remove(map, entry, NULL);
    hashmap_add(map, entry);
    return old;
}

void hashmap_iter_init(struct hashmap *map, struct hashmap_iter *iter)
{
    iter->map = map;
    iter->tablepos = 0;
    iter->next = NULL;
}

void *hashmap_iter_next(struct hashmap_iter *iter)
{
    struct hashmap_entry *current = iter->next;
    for (;;) {
        if (current) {
            iter->next = current->next;
            return current;
        }

        if (iter->tablepos >= iter->map->tablesize)
            return NULL;

        current = iter->map->table[iter->tablepos++];
    }
}