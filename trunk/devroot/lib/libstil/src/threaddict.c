/******************************************************************************
 *
 * <Copyright = "jasone">
 * <License>
 *
 ******************************************************************************
 *
 * Version: <Version>
 *
 ******************************************************************************/

#include "../include/libstil/libstil.h"

void
threaddict_populate(cw_stilo_t *a_dict, cw_stilt_t *a_stilt)
{
	cw_stilo_t	name, val;	/* XXX GC-unsafe. */

#define NENTRIES	3	/* Number of entries in threaddict. */
	stilo_dict_new(a_dict, a_stilt, NENTRIES);
#undef NENTRIES

	/*
	 * Initialize entries that are not operators.
	 */
	{
		const cw_uint8_t	str[] = "errordict";

		stilo_name_new(&name, a_stilt, str, sizeof(str) - 1, TRUE);
		stilo_dup(&val, stilt_errordict_get(a_stilt));

		stilo_dict_def(a_dict, a_stilt, &name, &val);
	}
	{
		const cw_uint8_t	str[] = "$error";

		stilo_name_new(&name, a_stilt, str, sizeof(str) - 1, TRUE);
		stilo_dup(&val, stilt_derror_get(a_stilt));

		stilo_dict_def(a_dict, a_stilt, &name, &val);
	}
	{
		const cw_uint8_t	str[] = "userdict";

		stilo_name_new(&name, a_stilt, str, sizeof(str) - 1, TRUE);
		stilo_dup(&val, stilt_userdict_get(a_stilt));

		stilo_dict_def(a_dict, a_stilt, &name, &val);
	}
}
