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

cw_stilo_threade_t
stilo_l_operator_print(cw_stilo_t *a_stilo, cw_stilo_t *a_file, cw_uint32_t
    a_depth)
{
	cw_stilo_threade_t	retval;
	
	if (a_stilo->op_code != STILN_ZERO) {
		_cw_assert(a_stilo->op_code <= STILN_LAST);
		if (a_stilo->fast_op) {
			retval = stilo_file_output(a_file,
			    "---[s]---", stiln_str(a_stilo->op_code));
		} else {
			retval = stilo_file_output(a_file, "--[s]--",
			    stiln_str(a_stilo->op_code));
		}
	} else
		retval = stilo_file_output(a_file, "-operator-");

	return retval;
}
