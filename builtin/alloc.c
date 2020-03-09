#include "alloc.h"
#include <stdlib.h>
#include "err.h"
#include <memory.h>

void *
xmalloc (size_t size)
{
    void *ret = malloc (size);
    if (ret == NULL)
        err (2, "xmalloc can not allocate %lu bytes", (u_long) size);
    return ret;
}

void *
xcalloc (size_t nmem, size_t size)
{
    void *ret = calloc (nmem, size);
    if (ret == NULL)
        err (2, "xcalloc can not allocate %lu bytes", (u_long) (size * nmem));
    return ret;
}