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

/* Default setting for whether to record stacks on error. */
#define	DERROR_RECORDSTACKS	TRUE

void
derror_populate(cw_stilo_t *a_dict, cw_stilt_t *a_stilt)
{
	cw_stilo_t	name, val;	/* XXX GC-unsafe. */

#define NENTRIES	7	/* Number of entries in derror. */
	stilo_dict_new(a_dict, stilt_stil_get(a_stilt), NENTRIES);

	/*
	 * Initialize entries that are not operators.
	 */
	stilo_name_new(&name, stilt_stil_get(a_stilt),
	    stiln_str(STILN_newerror), stiln_len(STILN_newerror), TRUE);
	stilo_boolean_new(&val, FALSE);
	stilo_dict_def(a_dict, a_stilt, &name, &val);

	stilo_name_new(&name, stilt_stil_get(a_stilt),
	    stiln_str(STILN_errorname), stiln_len(STILN_errorname), TRUE);
	stilo_string_new(&val, stilt_stil_get(a_stilt), 0);
	stilo_dict_def(a_dict, a_stilt, &name, &val);

	stilo_name_new(&name, stilt_stil_get(a_stilt), stiln_str(STILN_command),
	    stiln_len(STILN_command), TRUE);
	stilo_null_new(&val);
	stilo_dict_def(a_dict, a_stilt, &name, &val);

	stilo_name_new(&name, stilt_stil_get(a_stilt), stiln_str(STILN_ostack),
	    stiln_len(STILN_ostack), TRUE);
	stilo_array_new(&val, stilt_stil_get(a_stilt), 0);
	stilo_dict_def(a_dict, a_stilt, &name, &val);

	stilo_name_new(&name, stilt_stil_get(a_stilt), stiln_str(STILN_estack),
	    stiln_len(STILN_estack), TRUE);
	stilo_array_new(&val, stilt_stil_get(a_stilt), 0);
	stilo_dict_def(a_dict, a_stilt, &name, &val);

	stilo_name_new(&name, stilt_stil_get(a_stilt), stiln_str(STILN_dstack),
	    stiln_len(STILN_dstack), TRUE);
	stilo_array_new(&val, stilt_stil_get(a_stilt), 0);
	stilo_dict_def(a_dict, a_stilt, &name, &val);

	stilo_name_new(&name, stilt_stil_get(a_stilt),
	    stiln_str(STILN_recordstacks), stiln_len(STILN_recordstacks), TRUE);
	stilo_boolean_new(&val, DERROR_RECORDSTACKS);
	stilo_dict_def(a_dict, a_stilt, &name, &val);

#ifdef _LIBSTIL_DBG
	if (stilo_dict_count(a_dict) != NENTRIES) {
		_cw_out_put_e("stilo_dict_count(a_dict) != NENTRIES "
		    "([i] != [i])\n", stilo_dict_count(a_dict), NENTRIES);
		_cw_error("Adjust NENTRIES");
	}
#endif
#undef NENTRIES
}
