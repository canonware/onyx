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

#include "../include/libstil/libstil.h"
#include "../include/libstil/stilo_l.h"
#include "../include/libstil/stilo_operator_l.h"

void
stilo_operator_new(cw_stilo_t *a_stilo, cw_op_t *a_op, cw_stiln_t a_stiln)
{
	stilo_l_new(a_stilo, STILOT_OPERATOR);
	a_stilo->o.operator.f = a_op;
	a_stilo->op_code = a_stiln;
}

void
stilo_l_operator_print(cw_stilo_t *a_thread)
{
	cw_stilo_t		*ostack, *depth, *operator, *stdout_stilo;
	cw_stilo_threade_t	error;

	ostack = stilo_thread_ostack_get(a_thread);
	STILO_STACK_GET(depth, ostack, a_thread);
	STILO_STACK_DOWN_GET(operator, ostack, a_thread, depth);
	if (stilo_type_get(depth) != STILOT_INTEGER || stilo_type_get(operator)
	    != STILOT_OPERATOR) {
		stilo_thread_error(a_thread, STILO_THREADE_TYPECHECK);
		return;
	}
	stdout_stilo = stil_stdout_get(stilo_thread_stil_get(a_thread));

	if (operator->op_code != STILN_ZERO) {
		_cw_assert(operator->op_code <= STILN_LAST);
		if (operator->fast_op) {
			error = stilo_file_output(stdout_stilo,
			    "---[s]---", stiln_str(operator->op_code));
		} else {
			error = stilo_file_output(stdout_stilo, "--[s]--",
			    stiln_str(operator->op_code));
		}
	} else
		error = stilo_file_output(stdout_stilo, "-operator-");

	if (error) {
		stilo_thread_error(a_thread, error);
		return;
	}

	stilo_stack_npop(ostack, 2);
}
