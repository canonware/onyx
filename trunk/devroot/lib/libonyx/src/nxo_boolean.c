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
#include "../include/libonyx/nxo_l.h"

void
nxo_boolean_new(cw_nxo_t *a_nxo, cw_bool_t a_val)
{
	nxo_l_new(a_nxo, NXOT_BOOLEAN);
	a_nxo->o.boolean.val = a_val;
}


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
	stdout_nxo = nx_stdout_get(nxo_thread_nx_get(a_thread));

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

cw_bool_t
nxo_boolean_get(cw_nxo_t *a_nxo)
{
	_cw_check_ptr(a_nxo);
	_cw_assert(a_nxo->magic == _CW_NXO_MAGIC);
	_cw_assert(a_nxo->type == NXOT_BOOLEAN);

	return a_nxo->o.boolean.val;
}

void
nxo_boolean_set(cw_nxo_t *a_nxo, cw_bool_t a_val)
{
	_cw_check_ptr(a_nxo);
	_cw_assert(a_nxo->magic == _CW_NXO_MAGIC);
	_cw_assert(a_nxo->type == NXOT_BOOLEAN);

	a_nxo->o.boolean.val = a_val;
}
