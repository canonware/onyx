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

#define	_NXO_BOOLEAN_C_

#include "../include/libonyx/libonyx.h"
#include "../include/libonyx/nxo_l.h"

void
nxo_l_boolean_print(cw_nxo_t *a_thread)
{
	cw_nxo_t		*ostack, *depth, *boolean, *stdout_nxo;
	cw_nxo_threade_t	error;

	ostack = nxo_thread_ostack_get(a_thread);
	NXO_STACK_GET(depth, ostack, a_thread);
	NXO_STACK_DOWN_GET(boolean, ostack, a_thread, depth);
	if (nxo_type_get(depth) != NXOT_INTEGER || nxo_type_get(boolean) !=
	    NXOT_BOOLEAN) {
		nxo_thread_error(a_thread, NXO_THREADE_TYPECHECK);
		return;
	}
	stdout_nxo = nxo_thread_stdout_get(a_thread);

	if (boolean->o.boolean.val)
		error = nxo_file_output(stdout_nxo, "true");
	else
		error = nxo_file_output(stdout_nxo, "false");

	if (error) {
		nxo_thread_error(a_thread, error);
		return;
	}

	nxo_stack_npop(ostack, 2);
}
