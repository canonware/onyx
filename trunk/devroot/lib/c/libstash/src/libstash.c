/****************************************************************************
 *
 * <Copyright = jasone>
 * <License>
 *
 ****************************************************************************
 *
 * Version: <Version>
 *
 * <<< Description >>>
 *
 * Initialization and shutdown functions for the whole library.
 *
 ****************************************************************************/

#include "../include/libstash/libstash.h"

/* Globals. */
cw_dbg_t	*cw_g_dbg = NULL;
cw_mem_t	*cw_g_mem = NULL;
cw_out_t	*cw_g_out = NULL;
#ifdef _LIBSTASH_DBG
cw_mem_t	*cw_g_mem_mem = NULL;
#endif

cw_bool_t
libstash_init(void)
{
	/* Start up global modules. */
	cw_g_dbg = dbg_new(cw_g_mem);
	if (cw_g_dbg == NULL)
		goto OOM_1;
#ifdef _LIBSTASH_DBG
	dbg_register(cw_g_dbg, "mem_error");
	dbg_register(cw_g_dbg, "pezz_error");

	cw_g_mem_mem = mem_new(NULL, NULL);
	if (cw_g_mem_mem == NULL)
		goto OOM_2;

	cw_g_mem = mem_new(NULL, cw_g_mem_mem);
	if (cw_g_mem == NULL)
		goto OOM_3;
#else
	cw_g_mem = mem_new(NULL, NULL);
	if (cw_g_mem == NULL)
		goto OOM_3;
#endif

	cw_g_out = out_new(NULL, cw_g_mem);
	if (cw_g_out == NULL)
		goto OOM_4;

	return FALSE;
	OOM_4:
	mem_delete(cw_g_mem);
	cw_g_mem = NULL;
	OOM_3:
#ifdef _LIBSTASH_DBG
	mem_delete(cw_g_mem_mem);
	cw_g_mem_mem = NULL;
	OOM_2:
#endif
	dbg_delete(cw_g_dbg);
	cw_g_dbg = NULL;
	OOM_1:
	return TRUE;
}

void
libstash_shutdown(void)
{
	/* Shut down global modules in reverse order. */
	out_delete(cw_g_out);
	cw_g_out = NULL;

	mem_delete(cw_g_mem);
	cw_g_mem = NULL;

#ifdef _LIBSTASH_DBG
	mem_delete(cw_g_mem_mem);
	cw_g_mem_mem = NULL;
#endif

	dbg_delete(cw_g_dbg);
	cw_g_dbg = NULL;
}
