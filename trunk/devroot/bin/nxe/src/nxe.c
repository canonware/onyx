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
	cw_char_t	data_a[] = "AB\nD";
	cw_char_t	data_b[] = "\nabc";
	cw_char_t	data_c[] = "012\n";
	cw_bufc_t	bufc;

	buf = buf_new(NULL, (cw_opaque_alloc_t *)mem_malloc_e,
	    (cw_opaque_realloc_t *)mem_realloc_e, (cw_opaque_dealloc_t
	    *)mem_free_e, cw_g_mem, NULL);

	bufm = bufm_new(NULL, buf, NULL);

	fprintf(stderr, "length %llu, position %llu, line %llu\n", buf_len(buf),
	    bufm_pos(bufm), bufm_line(bufm));
	bufm_after_insert(bufm, data_a, strlen(data_a));

	fprintf(stderr, "length %llu, position %llu, line %llu\n", buf_len(buf),
	    bufm_pos(bufm), bufm_line(bufm));
	bufm_seek(bufm, 1, BUFW_BEG);

	fprintf(stderr, "length %llu, position %llu, line %llu\n", buf_len(buf),
	    bufm_pos(bufm), bufm_line(bufm));
	bufm_after_insert(bufm, data_b, strlen(data_b));

	fprintf(stderr, "length %llu, position %llu, line %llu\n", buf_len(buf),
	    bufm_pos(bufm), bufm_line(bufm));
	bufm_seek(bufm, 2, BUFW_BEG);
	
	fprintf(stderr, "length %llu, position %llu, line %llu\n", buf_len(buf),
	    bufm_pos(bufm), bufm_line(bufm));
	bufm_before_insert(bufm, data_c, strlen(data_c));

	fprintf(stderr, "length %llu, position %llu, line %llu\n", buf_len(buf),
	    bufm_pos(bufm), bufm_line(bufm));

	for (bufm_seek(bufm, 0, BUFW_BEG); bufm_pos(bufm) <= buf_len(buf);
	    bufm_seek(bufm, 1, BUFW_REL)) {
		_cw_assert(bufm_after_get(bufm, &bufc) == FALSE);
		fprintf(stderr, "position %llu, line %llu, char :%c:\n",
		    bufm_pos(bufm), bufm_line(bufm), bufc_char_get(bufc));
	}

	fprintf(stderr, "length %llu, position %llu, line %llu\n", buf_len(buf),
	    bufm_pos(bufm), bufm_line(bufm));
	bufm_seek(bufm, 5, BUFW_BEG);

	bufm_before_set(bufm, 'X');
	bufm_after_set(bufm, 'Y');

	fprintf(stderr, "bufm_after_get():\n");
	for (bufm_seek(bufm, 0, BUFW_BEG); bufm_pos(bufm) <= buf_len(buf);
	    bufm_seek(bufm, 1, BUFW_REL)) {
		_cw_assert(bufm_after_get(bufm, &bufc) == FALSE);
		fprintf(stderr, "position %llu, line %llu, char :%c:\n",
		    bufm_pos(bufm), bufm_line(bufm), bufc_char_get(bufc));
	}

	fprintf(stderr, "bufm_before_get():\n");
	for (bufm_seek(bufm, 1, BUFW_BEG); bufm_pos(bufm) <= buf_len(buf);
	    bufm_seek(bufm, 1, BUFW_REL)) {
		_cw_assert(bufm_before_get(bufm, &bufc) == FALSE);
		fprintf(stderr, "position %llu, line %llu, char :%c:\n",
		    bufm_pos(bufm), bufm_line(bufm), bufc_char_get(bufc));
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
