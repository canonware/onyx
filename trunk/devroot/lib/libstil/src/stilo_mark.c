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
#include "../include/libstil/stilo_mark_l.h"

cw_stilte_t
stilo_l_mark_print(cw_stilo_t *a_stilo, cw_stilo_t *a_file, cw_uint32_t
    a_depth)
{
	cw_stilte_t	retval;
	
	retval = stilo_file_output(a_file, "-mark-");

	return retval;
}
