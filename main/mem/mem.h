#ifndef _MEM_H
#define _MEM_H
#include <stdbool.h>
#include <stdlib.h>

typedef struct mem_pool *mem_pool;
extern mem_pool mp_new(size_t size);
extern void mp_delete(mem_pool *ap, bool clean);
extern void *mp_alloc(mem_pool mp, size_t nbytes);
extern void *mp_calloc(mem_pool mp, size_t count, size_t nbytes);
extern void mp_free(mem_pool mp);
extern void mp_print_info(mem_pool mp);
extern size_t mp_has_left(mem_pool mp);
extern size_t align_size(size_t s);
extern void mp_fake_free(void *ptr);
extern char *mp_strndup(mem_pool mp, const char *s1, size_t n);
#undef mem_pool
#endif
