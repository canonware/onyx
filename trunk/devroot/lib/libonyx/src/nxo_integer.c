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

#include "../include/libonyx/libonyx.h"
#include "../include/libonyx/nxo_integer_l.h"

void
nxo_l_integer_print(cw_nxo_t *a_thread)
{
	cw_nxo_t		*ostack, *depth, *integer, *stdout_nxo;
	cw_nxo_threade_t	error;

	ostack = nxo_thread_ostack_get(a_thread);
	NXO_STACK_GET(depth, ostack, a_thread);
	NXO_STACK_DOWN_GET(integer, ostack, a_thread, depth);
	if (nxo_type_get(depth) != NXOT_INTEGER || nxo_type_get(integer) !=
	    NXOT_INTEGER) {
		nxo_thread_error(a_thread, NXO_THREADE_TYPECHECK);
		return;
	}
	stdout_nxo = nx_stdout_get(nxo_thread_nx_get(a_thread));

#if (_CW_NXOI_SIZEOF == 8)
	error = nxo_file_output(stdout_nxo, "[q|s:s]", integer->o.integer.i);
#else
	error = nxo_file_output(stdout_nxo, "[i|s:s]", integer->o.integer.i);
#endif

	if (error) {
		nxo_thread_error(a_thread, error);
		return;
	}

	nxo_stack_npop(ostack, 2);
}
