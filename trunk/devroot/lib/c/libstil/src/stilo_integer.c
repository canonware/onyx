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
#include "../include/libstil/stilo_integer_l.h"

void
stilo_l_integer_print(cw_stilo_t *a_thread)
{
	cw_stilo_t		*ostack, *depth, *integer, *stdout_stilo;
	cw_stilo_threade_t	error;

	ostack = stilo_thread_ostack_get(a_thread);
	STILO_STACK_GET(depth, ostack, a_thread);
	STILO_STACK_DOWN_GET(integer, ostack, a_thread, depth);
	if (stilo_type_get(depth) != STILOT_INTEGER || stilo_type_get(integer)
	    != STILOT_INTEGER) {
		stilo_thread_error(a_thread, STILO_THREADE_TYPECHECK);
		return;
	}
	stdout_stilo = stil_stdout_get(stilo_thread_stil_get(a_thread));

#if (_CW_STILOI_SIZEOF == 8)
	error = stilo_file_output(stdout_stilo, "[q|s:s]",
	    integer->o.integer.i);
#else
	error = stilo_file_output(stdout_stilo, "[i|s:s]",
	    integer->o.integer.i);
#endif

	if (error) {
		stilo_thread_error(a_thread, error);
		return;
	}

	stilo_stack_npop(ostack, 2);
}
