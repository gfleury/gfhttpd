#include <assert.h>
#include <errno.h>
#include <stdlib.h>
#include <string.h>

#include <stdio.h>

#include "mem/mem.h"

typedef struct test
{
    int f;
    char c;
    char d[24];
    long l;
} test;

#define NE 10
#define MSIZE 1024

#define BIGCHUNK (MSIZE * 16)

int simple_allocations()
{
    mem_pool mp = mp_new(MSIZE);
    assert(mp);

    assert(mp_has_left(mp) == MSIZE);

    test *t = NULL;

    for (int c = 0; c < NE; c++)
    {
        t = mp_alloc(mp, sizeof(test));
        assert(t);
    }

    mp_print_info(mp);
    assert(mp_has_left(mp) == align_size(MSIZE) - (NE * sizeof(test)));

    // Next
    void *p = mp_alloc(mp, BIGCHUNK);
    assert(p);

    mp_print_info(mp);

    assert(mp_has_left(mp) == 0);

    void *x = mp_alloc(mp, BIGCHUNK);
    assert(x);

    assert(mp_has_left(mp) == 0);

    mp_print_info(mp);

    mp_delete(&mp, true);
    assert(mp == NULL);

    return (EXIT_SUCCESS);
}

int loop_allocation(int LOOP)
{
    mem_pool mp = mp_new(MSIZE * 256);
    assert(mp);

    assert(mp_has_left(mp) == MSIZE * 256);

    test *t = NULL;

    for (int c = 0; c < LOOP; c++)
    {
        t = mp_alloc(mp, sizeof(test));
        assert(t);
        memset(t, c, sizeof(test));
        assert(t->c == (char)c);
    }

    assert(mp_has_left(mp) == align_size(MSIZE * 256) - (LOOP * sizeof(test)));

    // mp_free(mp);

    for (int c = 0; c < LOOP; c++)
    {
        t = mp_alloc(mp, sizeof(test));
        assert(t);
        memset(t, 0, sizeof(test));
        assert(t->c == 0);
    }

    mp_print_info(mp);

    mp_delete(&mp, true);
    assert(mp == NULL);

    return (EXIT_SUCCESS);
}

int under_allocate(int LOOP)
{
    mem_pool mp = mp_new(17);
    assert(mp);

    assert(mp_has_left(mp) == 17);

    test *t = NULL;

    for (int c = 0; c < LOOP; c++)
    {
        t = mp_alloc(mp, sizeof(test) + c);
        assert(t);
        memset(t, c, sizeof(test));
        assert(t->c == (char)c);
    }

    mp_free(mp);

    for (int c = 0; c < LOOP; c++)
    {
        t = mp_alloc(mp, sizeof(test));
        assert(t);
        memset(t, 0, sizeof(test));
        assert(t->c == 0);
    }

    mp_delete(&mp, true);
    assert(mp == NULL);

    return (EXIT_SUCCESS);
}

int main()
{
    return simple_allocations() +
           loop_allocation(1) +
           loop_allocation(128) +
           loop_allocation(256) +
           loop_allocation(512) +
           loop_allocation(64) +
           loop_allocation(32) +
           loop_allocation(16) +
           loop_allocation(8) +
           loop_allocation(1024) +
           loop_allocation(2048) +
           under_allocate(256);
}
