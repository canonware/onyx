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

/* Prototypes for library-private functions that are only used in this file. */
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
void
mem_l_init(void);
void
mem_l_shutdown(void);
void
nxa_l_init(void);
void
nxa_l_shutdown(void);
void
systemdict_l_init(void);
void
systemdict_l_shutdown(void);

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
	mem_l_init();
	try_stage = 1;

	nxa_l_init();
    }
    xep_catch(CW_ONYXX_OOM)
    {
	switch (try_stage)
	{
	    case 1:
	    {
		mem_l_shutdown();
	    }
	    case 0:
	    {
		xep_l_shutdown();
#ifdef CW_THREADS
		thd_l_shutdown();
#endif
		break;
	    }
	    default:
	    {
		cw_not_reached();
	    }
	}
    }
    xep_end();

    systemdict_l_init();
}

void
libonyx_shutdown(void)
{
    /* Shut down global modules in reverse order. */
    systemdict_l_shutdown();
    nxa_l_shutdown();
    mem_l_shutdown();
    xep_l_shutdown();
#ifdef CW_THREADS
    thd_l_shutdown();
#endif
}
