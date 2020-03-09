#ifndef ALLOC_H
#define ALLOC_H
#include <inttypes.h>
#include <string.h>

#ifndef CHAR_BIT
# define CHAR_BIT __CHAR_BIT__
#endif

void * xcalloc (size_t nmem, size_t size);
void * xmalloc (size_t size);

#define bitsizeof(x)  (CHAR_BIT * sizeof(x))

#define maximum_signed_value_of_type(a) \
    (INTMAX_MAX >> (bitsizeof(intmax_t) - bitsizeof(a)))

#define maximum_unsigned_value_of_type(a) \
    (UINTMAX_MAX >> (bitsizeof(uintmax_t) - bitsizeof(a)))

/*
 * Signed integer overflow is undefined in C, so here's a helper macro
 * to detect if the sum of two integers will overflow.
 *
 * Requires: a >= 0, typeof(a) equals typeof(b)
 */
#define signed_add_overflows(a, b) \
    ((b) > maximum_signed_value_of_type(a) - (a))

#define unsigned_add_overflows(a, b) \
    ((b) > maximum_unsigned_value_of_type(a) - (a))

#define NORETURN __attribute__((__noreturn__))

extern NORETURN void die(const char *err, ...) __attribute__((format (printf, 1, 2)));

static inline size_t st_add(size_t a, size_t b)
{
    if (unsigned_add_overflows(a, b))
        die("size_t overflow: %"PRIuMAX" + %"PRIuMAX,
            (uintmax_t)a, (uintmax_t)b);
    return a + b;
}
#define st_add3(a,b,c)   st_add(st_add((a),(b)),(c))

typedef unsigned long int u_long;

#define FLEX_ALLOC_MEM(x, flexname, buf, len) do { \
	size_t flex_array_len_ = (len); \
	(x) = xcalloc(1, st_add3(sizeof(*(x)), flex_array_len_, 1)); \
	memcpy((void *)(x)->flexname, (buf), flex_array_len_); \
} while (0)
#define FLEXPTR_ALLOC_MEM(x, ptrname, buf, len) do { \
	size_t flex_array_len_ = (len); \
	(x) = xcalloc(1, st_add3(sizeof(*(x)), flex_array_len_, 1)); \
	memcpy((x) + 1, (buf), flex_array_len_); \
	(x)->ptrname = (void *)((x)+1); \
} while(0)

#endif