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
 * Initialization and shutdown functions for the whole library.
 *
 ******************************************************************************/

#include "../include/libonyx/libonyx.h"

/* #define CW_MEM_DBG */

#ifdef CW_THREADS
void
thd_l_init(void);
void
thd_l_shutdown(void);
#endif
void
xep_l_init(void);
void
xep_l_shutdown(void);

/* Globals. */
cw_mem_t *cw_g_mem = NULL;
#ifdef CW_MEM_DBG
static cw_mem_t *cw_g_mem_mem = NULL;
#endif

#if (defined(CW_SOCKET) && defined(CW_THREADS))
cw_mtx_t cw_g_gethostbyname_mtx;
cw_mtx_t cw_g_getprotobyname_mtx;
cw_mtx_t cw_g_getservbyname_mtx;
#endif

void
libonyx_init(void)
{
    volatile cw_uint32_t try_stage = 0;

    /* Start up global modules. */
#ifdef CW_THREADS
    thd_l_init();
#endif
    xep_l_init();

    xep_begin();
    xep_try
    {
#ifdef CW_MEM_DBG
	cw_g_mem_mem = mem_new(NULL, NULL);
	try_stage = 1;

	cw_g_mem = mem_new(NULL, cw_g_mem_mem);
	try_stage = 2;
#else
	cw_g_mem = mem_new(NULL, NULL);
	try_stage = 2;
#endif
    }
    xep_catch(CW_ONYXX_OOM)
    {
	switch (try_stage)
	{
	    case 2:
	    case 1:
	    {
#ifdef CW_MEM_DBG
		mem_delete(cw_g_mem_mem);
		cw_g_mem_mem = NULL;
#endif
	    }
	    case 0:
	    {
		break;
	    }
	    default:
	    {
		cw_not_reached();
	    }
	}
    }
    xep_end();

#ifdef CW_POSIX
    /* Ignore SIGPIPE, so that writing to a closed socket won't crash the
     * program. */
    signal(SIGPIPE, SIG_IGN);
#endif

#if (defined(CW_SOCKET) && defined(CW_THREADS))
    /* Initialize mutexes that protect non-reentrant functions. */
    mtx_new(&cw_g_gethostbyname_mtx);
    mtx_new(&cw_g_getprotobyname_mtx);
    mtx_new(&cw_g_getservbyname_mtx);
#endif
}

void
libonyx_shutdown(void)
{
    /* Shut down global modules in reverse order. */
#if (defined(CW_SOCKET) && defined(CW_THREADS))
    mtx_delete(&cw_g_getservbyname_mtx);
    mtx_delete(&cw_g_getprotobyname_mtx);
    mtx_delete(&cw_g_gethostbyname_mtx);
#endif

    mem_delete(cw_g_mem);
    cw_g_mem = NULL;

#ifdef CW_MEM_DBG
    mem_delete(cw_g_mem_mem);
    cw_g_mem_mem = NULL;
#endif

    xep_l_shutdown();
#ifdef CW_THREADS
    thd_l_shutdown();
#endif
}
