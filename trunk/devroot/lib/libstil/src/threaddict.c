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
threaddict_l_populate(cw_stilo_t *a_dict, cw_stilo_t *a_thread)
{
	cw_stilo_t	*tstack;
	cw_stilo_t	*name, *value;

#define NENTRIES	4	/* Number of entries in threaddict. */
	stilo_dict_new(a_dict, stilo_thread_stil_get(a_thread), FALSE,
	    NENTRIES);

	tstack = stilo_thread_tstack_get(a_thread);
	name = stilo_stack_push(tstack);
	value = stilo_stack_push(tstack);

	/*
	 * Initialize entries that are not operators.
	 */

	/* threaddict. */
	stilo_name_new(name, stilo_thread_stil_get(a_thread),
	    stiln_str(STILN_threaddict), stiln_len(STILN_threaddict), TRUE);
	stilo_dup(value, a_dict);
	stilo_dict_def(a_dict, stilo_thread_stil_get(a_thread), name, value);

	/* errordict. */
	stilo_name_new(name, stilo_thread_stil_get(a_thread),
	    stiln_str(STILN_errordict), stiln_len(STILN_errordict), TRUE);
	stilo_dup(value, stilo_thread_errordict_get(a_thread));
	stilo_dict_def(a_dict, stilo_thread_stil_get(a_thread), name, value);

	/* currenterror. */
	stilo_name_new(name, stilo_thread_stil_get(a_thread),
	    stiln_str(STILN_currenterror),
	    stiln_len(STILN_currenterror), TRUE);
	stilo_dup(value, stilo_thread_currenterror_get(a_thread));
	stilo_dict_def(a_dict, stilo_thread_stil_get(a_thread), name, value);

	/* userdict. */
	stilo_name_new(name, stilo_thread_stil_get(a_thread),
	    stiln_str(STILN_userdict), stiln_len(STILN_userdict), TRUE);
	stilo_dup(value, stilo_thread_userdict_get(a_thread));
	stilo_dict_def(a_dict, stilo_thread_stil_get(a_thread), name, value);

	stilo_stack_npop(tstack, 2);

#ifdef _LIBSTIL_DBG
	if (stilo_dict_count(a_dict) != NENTRIES) {
		_cw_out_put_e("stilo_dict_count(a_dict) != NENTRIES "
		    "([i] != [i])\n", stilo_dict_count(a_dict), NENTRIES);
		_cw_error("Adjust NENTRIES");
	}
#endif
#undef NENTRIES
}
