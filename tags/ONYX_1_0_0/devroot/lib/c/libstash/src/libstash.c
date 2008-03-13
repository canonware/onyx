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

/*  #define _CW_MEM_DBG */

void	thd_l_init(void);
void	thd_l_shutdown(void);
void	xep_l_init(void);
void	xep_l_shutdown(void);

/* Globals. */
cw_mem_t	*cw_g_mem = NULL;
cw_out_t	*out_std = NULL;
cw_out_t	*out_err = NULL;
#ifdef _CW_MEM_DBG
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
#ifdef _CW_MEM_DBG
		cw_g_mem_mem = mem_new(NULL, NULL);
		try_stage = 1;

		cw_g_mem = mem_new(NULL, cw_g_mem_mem);
		try_stage = 2;
#else
		cw_g_mem = mem_new(NULL, NULL);
		try_stage = 2;
#endif

		out_std = out_new(NULL, cw_g_mem);
		out_default_fd_set(out_std, 1);
		try_stage = 3;

		out_err = out_new(NULL, cw_g_mem);
		out_default_fd_set(out_err, 2);
		try_stage = 4;
	}
	xep_catch(_CW_STASHX_OOM) {
		switch (try_stage) {
		case 3:
			out_delete(out_std);
		case 2:
			mem_delete(cw_g_mem);
			cw_g_mem = NULL;
		case 1:
#ifdef _CW_MEM_DBG
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
	out_delete(out_err);
	out_err = NULL;

	out_delete(out_std);
	out_std = NULL;

	mem_delete(cw_g_mem);
	cw_g_mem = NULL;

#ifdef _CW_MEM_DBG
	mem_delete(cw_g_mem_mem);
	cw_g_mem_mem = NULL;
#endif

	xep_l_shutdown();
	thd_l_shutdown();
}
