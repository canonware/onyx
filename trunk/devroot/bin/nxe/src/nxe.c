/******************************************************************************
 *
 * <Copyright = jasone>
 * <License>
 *
 ******************************************************************************
 *
 * Version: <Version>
 *
 ******************************************************************************/

#include "nxe.h"

void
foo(cw_nx_t *a_nx, cw_nxo_t *a_thread)
{
	cw_buf_t	*buf;

	buf = buf_new(NULL, (cw_opaque_alloc_t *)mem_malloc_e,
	    (cw_opaque_realloc_t *)mem_realloc_e, (cw_opaque_dealloc_t
	    *)mem_free_e, cw_g_mem);

	buf_delete(buf);
}

int
main(int argc, char **argv, char **envp)
{
	cw_nx_t		nx;
	cw_nxo_t	thread;

	libonyx_init();
	nx_new(&nx, NULL, argc, argv, envp);
	nxo_thread_new(&thread, &nx);

	_cw_onyx_code(&thread, "`Hello world!\\n' print flush");

	foo(&nx, &thread);	/* XXX */

	nx_delete(&nx);
	libonyx_shutdown();
	return 0;
}
