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
	cw_bufm_t	*bufm;
	cw_bufc_t	bufc, data[4];
	cw_uint32_t	i;

	buf = buf_new(NULL, (cw_opaque_alloc_t *)mem_malloc_e,
	    (cw_opaque_realloc_t *)mem_realloc_e, (cw_opaque_dealloc_t
	    *)mem_free_e, cw_g_mem, NULL);

	bufm = bufm_new(NULL, buf, NULL);


	data[0].c = 'A';
	data[1].c = 'B';
	data[2].c = '\n';
	data[3].c = 'C';
	bufm_bufc_insert(bufm, data, 4);

/*  	fprintf(stderr, "line at offset %llu: %llu\n", bufm_pos(bufm), */
/*  	    bufm_line(bufm)); */

	bufm_abs_seek(bufm, 1);

	for (i = 0; i < 5; i++) {
		fprintf(stderr, "line at offset %llu: %llu\n", bufm_pos(bufm),
		    bufm_line(bufm));
		bufc = bufm_bufc_get(bufm);
		fprintf(stderr, "line at offset %llu: :%c:\n", bufm_pos(bufm),
		    bufc.c);

		bufm_rel_seek(bufm, 1);
	}
		
	bufm_delete(bufm);
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
