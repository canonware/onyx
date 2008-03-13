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

void
do_mem(void)
{
    void *p;

    p = cw_malloc(1024);
    cw_check_ptr(p);

    p = cw_realloc(p, 2048);
    cw_check_ptr(p);

    cw_free(p);

    p = cw_calloc(8, 128);
    cw_check_ptr(p);

    cw_free(p);
}

int
main()
{
    fprintf(stderr, "Test begin\n");
    do_mem();

    fprintf(stderr, "libonyx_init()\n");
    libonyx_init();
    do_mem();

    fprintf(stderr, "libonyx_shutdown()\n");
    libonyx_shutdown();
    do_mem();

    fprintf(stderr, "Test end\n");
    return 0;
}
