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

#include "../include/modslate.h"

static const struct cw_slate_entry slate_ops[] = {
	ENTRY(buffer),
	ENTRY(buffer_length),
	ENTRY(buffer_lines),
	ENTRY(buffer_undo),
	ENTRY(buffer_redo),
	ENTRY(buffer_history_active),
	ENTRY(buffer_history_setactive),
	ENTRY(buffer_history_startgroup),
	ENTRY(buffer_history_endgroup),
	ENTRY(buffer_history_flush),
	ENTRY(marker),
	ENTRY(marker_copy),
	ENTRY(marker_buffer),
	ENTRY(marker_line),
	ENTRY(marker_seekline),
	ENTRY(marker_position),
	ENTRY(marker_seek),
	ENTRY(marker_before_get),
	ENTRY(marker_after_get),
	ENTRY(marker_before_insert),
	ENTRY(marker_after_insert),
	ENTRY(marker_range_get),
	ENTRY(marker_range_cut)
};

void
slate_ops_init(cw_nxo_t *a_thread, const struct cw_slate_entry *a_entries,
    cw_uint32_t a_nentries)
{
	cw_nxo_t	*tstack;
	cw_nxo_t	*globaldict, *name, *value;
	cw_nx_t		*nx;
	cw_uint32_t	i;

	tstack = nxo_thread_tstack_get(a_thread);
	nx = nxo_thread_nx_get(a_thread);
	globaldict = nx_globaldict_get(nx);

	name = nxo_stack_push(tstack);
	value = nxo_stack_push(tstack);

	for (i = 0; i < a_nentries; i++) {
		nxo_name_new(name, nx, a_entries[i].op_n,
		    strlen(a_entries[i].op_n), FALSE);
		nxo_operator_new(value, a_entries[i].op_f, NXN_ZERO);
		nxo_attr_set(value, NXOA_EXECUTABLE);

		nxo_dict_def(globaldict, nx, name, value);
	}

	nxo_stack_npop(tstack, 2);
}

#if (0)
void
foo(cw_nx_t *a_nx, cw_nxo_t *a_thread)
{
	cw_buf_t	*buf;
	cw_bufm_t	*bufm, *bufm_b, *bufm_c;
	cw_uint8_t	*data;
	cw_uint8_t	data_a[] = "AB\nD";
	cw_uint8_t	data_b[] = "\nabc";
	cw_uint8_t	data_c[] = "012\n";

	buf = buf_new(NULL, (cw_opaque_alloc_t *)mem_malloc_e,
	    (cw_opaque_realloc_t *)mem_realloc_e, (cw_opaque_dealloc_t
	    *)mem_free_e, cw_g_mem);

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
		_cw_assert((data = bufm_after_get(bufm)) != NULL);
		fprintf(stderr, "position %llu, line %llu, char :%c:\n",
		    bufm_pos(bufm), bufm_line(bufm), *data);
	}

	fprintf(stderr, "length %llu, position %llu, line %llu\n", buf_len(buf),
	    bufm_pos(bufm), bufm_line(bufm));
	bufm_seek(bufm, 5, BUFW_BEG);

	data = bufm_before_get(bufm);
	*data = 'X';
	data = bufm_after_get(bufm);
	*data = 'Y';

	fprintf(stderr, "bufm_after_get():\n");
	for (bufm_seek(bufm, 0, BUFW_BEG); bufm_pos(bufm) <= buf_len(buf);
	    bufm_seek(bufm, 1, BUFW_REL)) {
		_cw_assert((data = bufm_after_get(bufm)) != NULL);
		fprintf(stderr, "position %llu, line %llu, char :%c:\n",
		    bufm_pos(bufm), bufm_line(bufm), *data);
	}

	fprintf(stderr, "bufm_before_get():\n");
	for (bufm_seek(bufm, 1, BUFW_BEG); bufm_pos(bufm) <= buf_len(buf);
	    bufm_seek(bufm, 1, BUFW_REL)) {
		_cw_assert((data = bufm_before_get(bufm)) != NULL);
		fprintf(stderr, "position %llu, line %llu, char :%c:\n",
		    bufm_pos(bufm), bufm_line(bufm), *data);
	}
	
	bufm_b = bufm_new(NULL, buf, NULL);
	bufm_c = bufm_new(NULL, buf, NULL);

	fprintf(stderr, "bufm_remove():\n");
	bufm_seek(bufm_b, 1, BUFW_BEG);
	bufm_seek(bufm_c, 3, BUFW_BEG);
	bufm_remove(bufm_b, bufm_c);

	fprintf(stderr, "bufm_after_get():\n");
	for (bufm_seek(bufm, 0, BUFW_BEG); bufm_pos(bufm) <= buf_len(buf);
	    bufm_seek(bufm, 1, BUFW_REL)) {
		_cw_assert((data = bufm_after_get(bufm)) != NULL);
		fprintf(stderr, "position %llu, line %llu, char :%c:\n",
		    bufm_pos(bufm), bufm_line(bufm), *data);
	}

	buf_elmsize_set(buf, 7);

	fprintf(stderr, "bufm_after_get(): after buf_elmsize_set()\n");
	for (bufm_seek(bufm, 0, BUFW_BEG); bufm_pos(bufm) <= buf_len(buf);
	    bufm_seek(bufm, 1, BUFW_REL)) {
		_cw_assert((data = bufm_after_get(bufm)) != NULL);
		fprintf(stderr, "position %llu, line %llu, char :%c:\n",
		    bufm_pos(bufm), bufm_line(bufm), *data);
	}

	bufm_delete(bufm_c);
	bufm_delete(bufm_b);
	bufm_delete(bufm);
	buf_delete(buf);
}
#endif

void
slate_init (void *a_arg, cw_nxo_t *a_thread)
{
/*  	fprintf(stderr, "%s:%u:%s(): Got here\n", __FILE__, __LINE__, */
/*  	    __FUNCTION__); */
	slate_buffer_init(a_thread);

#if (0)
	foo(nxo_thread_nx_get(a_thread), a_thread);	/* XXX */
#endif
}
