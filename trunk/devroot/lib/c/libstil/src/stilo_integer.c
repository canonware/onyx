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

cw_stilo_threade_t
stilo_l_integer_print(cw_stilo_t *a_stilo, cw_stilo_t *a_file, cw_uint32_t
    a_depth)
{
	cw_stilo_threade_t	retval;

#if (_CW_STILOI_SIZEOF == 8)
	retval = stilo_file_output(a_file, "[q|s:s]", a_stilo->o.integer.i);
#else
	retval = stilo_file_output(a_file, "[i|s:s]", a_stilo->o.integer.i);
#endif

	return retval;
}
