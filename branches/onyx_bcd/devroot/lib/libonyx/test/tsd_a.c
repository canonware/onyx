/* -*- mode: c ; c-file-style: "canonware-c-style" -*-
 ******************************************************************************
 *
 * <Copyright = jasone>
 * <License>
 *
 ******************************************************************************
 *
 * Version: <Version>
 *
 * tsd test.
 *
 ******************************************************************************/

#include "../include/libonyx/libonyx.h"

#define NTHREADS 20
#define NITERATIONS 1000

cw_tsd_t tsd;
cw_uint8_t arr[NITERATIONS];

void *
thread_entry_func(void *a_arg)
{
    cw_uint32_t i;

    for (i = 0; i < NITERATIONS; i++)
    {
	tsd_set(&tsd, (void *)&arr[i]);
	thd_yield();
	cw_assert((cw_uint8_t *)tsd_get(&tsd) == &arr[i]);
	thd_yield();
    }

    return NULL;
}

int
main()
{
    cw_thd_t *thds[NTHREADS];
    cw_uint32_t i;

    libonyx_init();
    fprintf(stderr, "Test begin\n");

    tsd_new(&tsd, NULL);

    for (i = 0; i < NTHREADS; i++)
    {
	thds[i] = thd_new(thread_entry_func, NULL, TRUE);
    }
	
    for (i = 0; i < NTHREADS; i++)
    {
	thd_join(thds[i]);
    }

    tsd_delete(&tsd);

    fprintf(stderr, "Test end\n");
    libonyx_shutdown();
    return 0;
}
