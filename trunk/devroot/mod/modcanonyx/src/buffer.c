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

#include "modcanonyx.h"

static void
buffer_p_eval(void *a_data, cw_nxo_t *a_thread)
{
}

static cw_nxoe_t *
buffer_p_ref_iter(void *a_data, cw_bool_t a_reset)
{
	return NULL;
}

static void
buffer_p_delete(void *a_data, cw_nx_t *a_nx)
{
	_cw_free(a_data);
}

void
canonyx_buffer(cw_nxo_t *a_thread)
{
	cw_nxo_t		*ostack, /*  *tstack,  */*nxo, *tag;
	cw_nx_t			*nx;
	cw_buf_t		*buffer;
	static const cw_uint8_t	tagname[] = "buffer";

	ostack = nxo_thread_ostack_get(a_thread);
/*  	tstack = nxo_thread_tstack_get(a_thread); */
	nx = nxo_thread_nx_get(a_thread);

	/* Allocate and initialize the buffer structure. */
	buffer = (cw_buf_t *)_cw_malloc(sizeof(cw_buf_t));
	memset(buffer, 0, sizeof(cw_buf_t));

	/* Create a reference to the buffer. */
	nxo = nxo_stack_push(ostack);
	nxo_hook_new(nxo, nx, buffer, buffer_p_eval, buffer_p_ref_iter,
	    buffer_p_delete);

	/* Set the hook tag. */
	tag = nxo_hook_tag_get(nxo);
	nxo_name_new(tag, nx, tagname, sizeof(tagname) - 1, TRUE);
	nxo_attr_set(tag, NXOA_EXECUTABLE);
}

#if (0)
void
canonyx_buffer_file_open(cw_nxo_t *a_thread)
{
	cw_nxo_t	*ostack, *tstack, *file, *tfile;
	cw_nx_t		*nx;
	cw_uint32_t	len;

	ostack = nxo_thread_ostack_get(a_thread);
	tstack = nxo_thread_tstack_get(a_thread);
	nx = nxo_thread_nx_get(a_thread);
	NXO_STACK_GET(filename, ostack, a_thread);
	if (nxo_type_get(filename) != NXOT_STRING) {
		nxo_thread_error(a_thread, NXO_THREADE_TYPECHECK);
		return;
	}

	/*
	 * Create a copy of filename with an extra byte to store a `\0'
	 * terminator.
	 */
	tfile = nxo_stack_push(tstack);
	len = nxo_string_len_get(file);
	nxo_string_new(tfile, nx, FALSE, len + 1);
	nxo_string_lock(file);
	nxo_string_set(tfile, 0, nxo_string_get(file), len);
	nxo_string_unlock(file);
	nxo_string_el_set(tfile, '\0' len);

	/* Open file. */

}
#endif
