/****************************************************************************
 *
 * <Copyright = "jasone">
 * <License>
 *
 ****************************************************************************
 *
 * Version: <Version>
 *
 ****************************************************************************/

#include "../include/libstil/libstil.h"

void
stil_op_print(cw_stilt_t *a_stilt)
{

}

void
stil_op_pstack(cw_stilt_t *a_stilt)
{
	cw_stils_t	*stils;
	cw_stilo_t	*stilo;
	cw_sint32_t	fd;

	stils = stilt_data_stack_get(a_stilt);
	fd = stilt_stdout_get(a_stilt);

	for (stilo = stils_get(stils, 0); stilo != NULL; stilo =
		 stils_get_down(stils, stilo))
		stilo_print(stilo, fd, TRUE, TRUE);
}

void
stil_op_stack(cw_stilt_t *a_stilt)
{
	cw_stils_t	*stils;
	cw_stilo_t	*stilo;
	cw_sint32_t	fd;

	stils = stilt_data_stack_get(a_stilt);
	fd = stilt_stdout_get(a_stilt);

	for (stilo = stils_get(stils, 0); stilo != NULL; stilo =
		 stils_get_down(stils, stilo))
		stilo_print(stilo, fd, FALSE, TRUE);
}
