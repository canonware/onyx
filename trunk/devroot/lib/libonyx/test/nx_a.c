/* -*- mode: c ; c-file-style: "canonware-c-style" -*-
 ******************************************************************************
 *
 * <Copyright = jasone>
 * <License>
 *
 ******************************************************************************
 *
 * Version: Onyx <Version = onyx>
 *
 ******************************************************************************/

#include "../include/libonyx/libonyx.h"

#define NITERATIONS 5
#define NINTERPS 3

void *
thread_entry_func(void *a_arg)
{
    cw_nx_t nx;
    cw_nxo_t thread;
    cw_uint32_t i;

    cw_assert(nx_new(&nx, NULL, NULL) == &nx);
    nxo_thread_new(&thread, &nx);

    for (i = 0; i < NITERATIONS; i++)
    {
	cw_onyx_code(&thread, "10000 {[`a' `b' `c']} repeat clear");
	fprintf(stderr, ".");
    }
/*     cw_onyx_code(&thread, "gcdict $stats get eval 2 sprint"); */

    nx_delete(&nx);
    return NULL;
}

int
main(int argc, char **argv, char **envp)
{
    cw_thd_t *thds[NINTERPS];
    cw_uint32_t i;

    fprintf(stderr, "Test begin\n");
    libonyx_init(argc, argv, envp);

    for (i = 0; i < NINTERPS; i++)
    {
	thds[i] = thd_new(thread_entry_func, NULL, TRUE);
    }

    for (i = 0; i < NINTERPS; i++)
    {
	thd_join(thds[i]);
    }

#if (0)
    {
	cw_nx_t nx;
	cw_nxo_t thread;

	cw_assert(nx_new(&nx, NULL, NULL) == &nx);
	nxo_thread_new(&thread, &nx);

	fprintf(stderr, "\n");
	cw_onyx_code(&thread, "gcdict $stats get eval 2 sprint");

	nx_delete(&nx);
    }
#endif

    libonyx_shutdown();
    fprintf(stderr, "\nTest end\n");
    return 0;
}
