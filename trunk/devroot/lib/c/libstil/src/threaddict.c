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

void
threaddict_populate(cw_stilo_t *a_dict, cw_stilt_t *a_stilt)
{
	cw_stilo_t	name, val;	/* XXX GC-unsafe. */

#define NENTRIES	3	/* Number of entries in threaddict. */
	stilo_dict_new(a_dict, stilt_stil_get(a_stilt), NENTRIES);

	/*
	 * Initialize entries that are not operators.
	 */
	stilo_name_new(&name, stilt_stil_get(a_stilt),
	    stiln_str(STILN_errordict), stiln_len(STILN_errordict), TRUE);
	stilo_dup(&val, stilt_errordict_get(a_stilt));
	stilo_dict_def(a_dict, a_stilt, &name, &val);

	stilo_name_new(&name, stilt_stil_get(a_stilt),
	    stiln_str(STILN_sym_derror), stiln_len(STILN_sym_derror), TRUE);
	stilo_dup(&val, stilt_derror_get(a_stilt));
	stilo_dict_def(a_dict, a_stilt, &name, &val);

	stilo_name_new(&name, stilt_stil_get(a_stilt),
	    stiln_str(STILN_userdict), stiln_len(STILN_userdict), TRUE);
	stilo_dup(&val, stilt_userdict_get(a_stilt));
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
