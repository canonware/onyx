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

/* Default setting for whether to record stacks on error. */
#define	DERROR_RECORDSTACKS	TRUE

void
derror_populate(cw_stilo_t *a_dict, cw_stilt_t *a_stilt)
{
	cw_stilo_t	name, val;	/* XXX GC-unsafe. */

#define NENTRIES	7	/* Number of entries in derror. */
	stilo_dict_new(a_dict, a_stilt, NENTRIES);
#undef NENTRIES

	/*
	 * Initialize entries that are not operators.
	 */
	{
		static const cw_uint8_t	str[] = "newerror";

		stilo_name_new(&name, a_stilt, str, sizeof(str) - 1, TRUE);
		stilo_boolean_new(&val, FALSE);

		stilo_dict_def(a_dict, a_stilt, &name, &val);
	}
	{
		static const cw_uint8_t	str[] = "errorname";

		stilo_name_new(&name, a_stilt, str, sizeof(str) - 1, TRUE);
		stilo_string_new(&val, a_stilt, 0);

		stilo_dict_def(a_dict, a_stilt, &name, &val);
	}
	{
		static const cw_uint8_t	str[] = "command";

		stilo_name_new(&name, a_stilt, str, sizeof(str) - 1, TRUE);
		stilo_null_new(&val);

		stilo_dict_def(a_dict, a_stilt, &name, &val);
	}
	{
		static const cw_uint8_t	str[] = "ostack";

		stilo_name_new(&name, a_stilt, str, sizeof(str) - 1, TRUE);
		stilo_array_new(&val, a_stilt, 0);

		stilo_dict_def(a_dict, a_stilt, &name, &val);
	}
	{
		static const cw_uint8_t	str[] = "estack";

		stilo_name_new(&name, a_stilt, str, sizeof(str) - 1, TRUE);
		stilo_array_new(&val, a_stilt, 0);

		stilo_dict_def(a_dict, a_stilt, &name, &val);
	}
	{
		static const cw_uint8_t	str[] = "dstack";

		stilo_name_new(&name, a_stilt, str, sizeof(str) - 1, TRUE);
		stilo_array_new(&val, a_stilt, 0);

		stilo_dict_def(a_dict, a_stilt, &name, &val);
	}
	{
		static const cw_uint8_t	str[] = "recordstacks";

		stilo_name_new(&name, a_stilt, str, sizeof(str) - 1, TRUE);
		stilo_boolean_new(&val, DERROR_RECORDSTACKS);

		stilo_dict_def(a_dict, a_stilt, &name, &val);
	}
}
