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
 * thd test.
 *
 ******************************************************************************/

#include "../include/libonyx/libonyx.h"

cw_mtx_t mtx;
cw_thd_t *thread_a, *thread_b;

void *
thread_entry_func(void *a_arg)
{
    char *arg_str = (char *) a_arg;

    fprintf(stderr, "%s(): Argument string: \"%s\"\n", __func__, arg_str);

    mtx_lock(&mtx);
    fprintf(stderr, "%s(): thd_self() returns %s\n", __func__,
	    (thd_self() == thread_a) ? "thread_a" : (thd_self() == thread_b)
	    ? "thread_b" : "<error>");
    mtx_unlock(&mtx);

    return NULL;
}

int
main()
{
    libonyx_init();
    fprintf(stderr, "Test begin\n");
    mtx_new(&mtx);

    thread_b = NULL;
    mtx_lock(&mtx);
    thread_a = thd_new(thread_entry_func, (void *)"Thread A argument", TRUE);
    mtx_unlock(&mtx);
    cw_assert(thd_join(thread_a) == NULL);
    thread_a = NULL;

    mtx_lock(&mtx);
    thread_b = thd_new(thread_entry_func, (void *)"Thread B argument", TRUE);
    mtx_unlock(&mtx);
    cw_assert(thd_join(thread_b) == NULL);

    mtx_delete(&mtx);
    fprintf(stderr, "Test end\n");
    libonyx_shutdown();

    return 0;
}
