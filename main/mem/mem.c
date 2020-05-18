/*
 * Highly inspired, if not copied, from CII Book from David R. Hanson
*/
#include <assert.h>
#include <string.h>

#include <stdio.h>

#include "mem.h"

#define THRESHOLD 10
struct mem_pool
{
    mem_pool prev;
    char *avail;
    char *limit;
};

static mem_pool freechunks;
static int nfree;

#define ALIGNMENT 16

mem_pool mp_new(size_t nbytes)
{
    mem_pool mp;

    size_t size = sizeof(struct mem_pool) + nbytes;

    int ret = posix_memalign((void **)&mp, ALIGNMENT, size);
    assert(ret == 0);
    assert(mp != NULL);

    mp->avail = (char *)((mem_pool)mp + 1);
    mp->limit = (char *)mp + size;
    mp->prev = NULL;

    return mp;
}

void mp_delete(mem_pool *ap, bool clean)
{
    if (clean)
    {
        nfree = THRESHOLD;
    }
    assert(ap && *ap);
    mp_free(*ap);
    free(*ap);
    *ap = NULL;
}

void *mp_alloc(mem_pool mp, size_t nbytes)
{
    assert(mp);
    assert(nbytes > 0);

    while (nbytes > mp->limit - mp->avail)
    {
        mem_pool ptr;
        char *limit;
        if ((ptr = freechunks) != NULL)
        {
            freechunks = freechunks->prev;
            nfree--;
            limit = ptr->limit;
        }
        else
        {
            size_t size = sizeof(struct mem_pool) + nbytes;
            int ret = posix_memalign((void **)&ptr, ALIGNMENT, size);
            assert(ret == 0);
            assert(ptr != NULL);

            limit = (char *)ptr + size;
        }
        *ptr = *mp;
        mp->avail = (char *)((mem_pool)ptr + 1);
        mp->limit = limit;
        mp->prev = ptr;
    }
    mp->avail += nbytes;
    return mp->avail - nbytes;
}

void *mp_calloc(mem_pool mp, size_t count, size_t nbytes)
{
    void *ptr;
    assert(count > 0);
    ptr = mp_alloc(mp, count * nbytes);
    memset(ptr, '\0', count * nbytes);
    return ptr;
}

void mp_free(mem_pool mp)
{
    assert(mp);
    while (mp->prev)
    {
        struct mem_pool tmp = *mp->prev;
        if (nfree < THRESHOLD)
        {
            mp->prev->prev = freechunks;
            freechunks = mp->prev;
            nfree++;
            freechunks->limit = mp->limit;
        }
        else
            free(mp->prev);
        *mp = tmp;
    }
    // Check If the base from mp_new is still here.
    assert(mp->limit != NULL);
    assert(mp->avail != NULL);
}

void mp_print_info(mem_pool mp)
{
    int c = 0;
    while (mp)
    {
        printf("mem_pool: %d\n", c++);
        printf("\tfree avail: %lu", mp_has_left(mp));

        printf("\n");
        mp = mp->prev;
    }
}

size_t mp_has_left(mem_pool mp)
{
    return mp->limit - mp->avail;
}

size_t align_size(size_t s)
{
    return (((s + ALIGNMENT - 1) / ALIGNMENT) * ALIGNMENT);
}

/*
 * Mock a fake free so uthash can think it freed his structures
*/
void mp_fake_free(void *ptr)
{
    return;
}