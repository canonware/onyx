/******************************************************************************
 *
 * <Copyright = jasone>
 * <License>
 *
 ******************************************************************************
 *
 * Version: <Version>
 *
 * Initialization and shutdown functions for the whole library.
 *
 ******************************************************************************/

#include "../include/libstash/libstash.h"

/*  #define _LIBSTASH_MEM_DBG */

void	thd_l_init(void);
void	thd_l_shutdown(void);
void	xep_l_init(void);
void	xep_l_shutdown(void);

/* Globals. */
cw_mem_t	*cw_g_mem = NULL;
cw_out_t	*cw_g_out = NULL;
#ifdef _LIBSTASH_MEM_DBG
static cw_mem_t	*cw_g_mem_mem = NULL;
#endif

void
libstash_init(void)
{
	volatile cw_uint32_t	try_stage = 0;

	/* Start up global modules. */
	thd_l_init();
	xep_l_init();

	xep_begin();
	xep_try {
#ifdef _LIBSTASH_MEM_DBG
		cw_g_mem_mem = mem_new(NULL, NULL);
		try_stage = 1;

		cw_g_mem = mem_new(NULL, cw_g_mem_mem);
		try_stage = 2;
#else
		cw_g_mem = mem_new(NULL, NULL);
		try_stage = 2;
#endif
		cw_g_out = out_new(NULL, cw_g_mem);
		out_default_fd_set(cw_g_out, 1);
		try_stage = 3;
	}
	xep_catch(_CW_STASHX_OOM) {
		switch (try_stage) {
		case 2:
			mem_delete(cw_g_mem);
			cw_g_mem = NULL;
		case 1:
#ifdef _LIBSTASH_MEM_DBG
			mem_delete(cw_g_mem_mem);
			cw_g_mem_mem = NULL;
#endif
		case 0:
			break;
		default:
			_cw_not_reached();
		}
	}
	xep_end();
}

void
libstash_shutdown(void)
{
	/* Shut down global modules in reverse order. */
	out_delete(cw_g_out);
	cw_g_out = NULL;

	mem_delete(cw_g_mem);
	cw_g_mem = NULL;

#ifdef _LIBSTASH_MEM_DBG
	mem_delete(cw_g_mem_mem);
	cw_g_mem_mem = NULL;
#endif

	xep_l_shutdown();
	thd_l_shutdown();
}
