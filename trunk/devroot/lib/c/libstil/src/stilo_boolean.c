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

void
stilo_boolean_new(cw_stilo_t *a_stilo, cw_bool_t a_val)
{
	stilo_l_new(a_stilo, STILOT_BOOLEAN);
	a_stilo->o.boolean.val = a_val;
}


cw_stilte_t
stilo_l_boolean_print(cw_stilo_t *a_stilo, cw_stilo_t *a_file, cw_uint32_t
    a_depth)
{
	cw_stilte_t	retval;

	if (a_stilo->o.boolean.val)
		retval = stilo_file_output(a_file, "true");
	else
		retval = stilo_file_output(a_file, "false");

	return retval;
}

cw_bool_t
stilo_boolean_get(cw_stilo_t *a_stilo)
{
	_cw_check_ptr(a_stilo);
	_cw_assert(a_stilo->magic == _CW_STILO_MAGIC);
	_cw_assert(a_stilo->type == STILOT_BOOLEAN);

	return a_stilo->o.boolean.val;
}

void
stilo_boolean_set(cw_stilo_t *a_stilo, cw_bool_t a_val)
{
	_cw_check_ptr(a_stilo);
	_cw_assert(a_stilo->magic == _CW_STILO_MAGIC);
	_cw_assert(a_stilo->type == STILOT_BOOLEAN);

	a_stilo->o.boolean.val = a_val;
}
