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
cw_dbg_t	*cw_g_dbg = NULL;
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
		cw_g_dbg = dbg_new(cw_g_mem);
		try_stage = 1;
#ifdef _LIBSTASH_DBG
		dbg_register(cw_g_dbg, "mem_error");
		dbg_register(cw_g_dbg, "pezz_error");
		dbg_register(cw_g_dbg, "pool_error");
#endif
#ifdef _LIBSTASH_MEM_DBG
		cw_g_mem_mem = mem_new(NULL, NULL);
		try_stage = 2;

		cw_g_mem = mem_new(NULL, cw_g_mem_mem);
		try_stage = 3;
#else
		cw_g_mem = mem_new(NULL, NULL);
		try_stage = 3;
#endif
		cw_g_out = out_new(NULL, cw_g_mem);
		out_default_fd_set(cw_g_out, 1);
		try_stage = 4;
	}
	xep_catch(_CW_XEPV_OOM) {
		switch (try_stage) {
		case 3:
			mem_delete(cw_g_mem);
			cw_g_mem = NULL;
		case 2:
#ifdef _LIBSTASH_MEM_DBG
			mem_delete(cw_g_mem_mem);
			cw_g_mem_mem = NULL;
#endif
		case 1:
			dbg_delete(cw_g_dbg);
			cw_g_dbg = NULL;
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

	dbg_delete(cw_g_dbg);
	cw_g_dbg = NULL;

	xep_l_shutdown();
	thd_l_shutdown();
}
