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

#define	_NXO_OPERATOR_C_

#include "../include/libonyx/libonyx.h"
#include "../include/libonyx/nxo_l.h"
#include "../include/libonyx/nxo_operator_l.h"

void
nxo_operator_new(cw_nxo_t *a_nxo, cw_op_t *a_op, cw_nxn_t a_nxn)
{
	nxo_p_new(a_nxo, NXOT_OPERATOR);
	a_nxo->o.operator.f = a_op;
	nxo_p_opcode_set(a_nxo, a_nxn);
}

void
nxo_l_operator_print(cw_nxo_t *a_thread)
{
	cw_nxo_t		*ostack, *depth, *operator, *stdout_nxo;
	cw_nxo_threade_t	error;

	ostack = nxo_thread_ostack_get(a_thread);
	NXO_STACK_GET(depth, ostack, a_thread);
	NXO_STACK_DOWN_GET(operator, ostack, a_thread, depth);
	if (nxo_type_get(depth) != NXOT_INTEGER || nxo_type_get(operator)
	    != NXOT_OPERATOR) {
		nxo_thread_error(a_thread, NXO_THREADE_TYPECHECK);
		return;
	}
	stdout_nxo = nxo_thread_stdout_get(a_thread);

	if (nxo_p_opcode_get(operator) != NXN_ZERO) {
		_cw_assert(nxo_p_opcode_get(operator) <= NXN_LAST);
		error = nxo_file_output(stdout_nxo, "--[s]--",
		    nxn_str(nxo_p_opcode_get(operator)));
	} else
		error = nxo_file_output(stdout_nxo, "-operator-");

	if (error) {
		nxo_thread_error(a_thread, error);
		return;
	}

	nxo_stack_npop(ostack, 2);
}
