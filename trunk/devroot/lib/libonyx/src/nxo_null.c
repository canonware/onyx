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
#include "../include/libonyx/nxo_null_l.h"

void
nxo_l_null_print(cw_nxo_t *a_thread)
{
	cw_nxo_t		*ostack, *depth, *null, *stdout_nxo;
	cw_nxo_threade_t	error;

	ostack = nxo_thread_ostack_get(a_thread);
	NXO_STACK_GET(depth, ostack, a_thread);
	NXO_STACK_DOWN_GET(null, ostack, a_thread, depth);
	if (nxo_type_get(depth) != NXOT_INTEGER || nxo_type_get(null)
	    != NXOT_NULL) {
		nxo_thread_error(a_thread, NXO_THREADE_TYPECHECK);
		return;
	}
	stdout_nxo = nxo_thread_stdout_get(a_thread);

	error = nxo_file_output(stdout_nxo, "null");

	if (error) {
		nxo_thread_error(a_thread, error);
		return;
	}

	nxo_stack_npop(ostack, 2);
}
