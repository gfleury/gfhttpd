#include <stdio.h>  /* printf */
#include <stdlib.h> /* malloc */
#include <string.h> /* strcpy */

#include "http_stream/headers.h"
#include "uthash.h"

int main(int argc, char *argv[])
{
    int ret = 0;
    char *names[] = {"joe", "bob", "betty", NULL};
    char *values[] = {"bill", "marley", "booty", NULL};
    headers *s, *pheaders = NULL;

    mem_pool mp = mp_new(16 * 1024);

    for (int i = 0; names[i]; ++i)
    {
        // s = (headers *)malloc(sizeof *s);
        // s->name = names[i];
        // s->n_name = strlen(names[i]);
        // s->value = values[i];
        // s->n_value = strlen(values[i]);
        // HASH_ADD_KEYPTR(hh, pheaders, s->name, s->n_name, s);
        insert_header(&pheaders, mp, names[i], strlen(names[i]), values[i], strlen(values[i]));
    }

    for (int i = 0; names[i]; ++i)
    {
        s = get_header(&pheaders, names[i]);
        if (s)
        {
            // printf("%s id is %s\n", names[i], s->value);
            ret = strncmp(s->value, values[i], s->n_value);
            if (ret != 0)
                goto end;
        }
    }

    char *new_names[] = {"Server", ":status", "content-lenght", NULL};
    char *new_values[] = {"quiche", "200", "5", NULL};

    for (int i = 0; new_names[i]; ++i)
    {
        insert_header(&pheaders, mp, new_names[i], strlen(new_names[i]), new_values[i], strlen(new_values[i]));
    }

    for (int i = 0; new_names[i]; ++i)
    {
        headers *s = get_header(&pheaders, new_names[i]);
        if (s)
        {
            // printf("%s id is %s\n", new_names[i], s->value);
            ret = strncmp(s->value, new_values[i], s->n_value);
            if (ret != 0)
                goto end;
        }
    }

end:
    /* free the hash table contents */
    // HASH_ITER(hh, pheaders, s, tmp)
    // {
    //     HASH_DEL(pheaders, s);
    //     // free(s);
    // }
    delete_header_all(&pheaders);
    mp_delete(&mp, true);

    return ret;
}