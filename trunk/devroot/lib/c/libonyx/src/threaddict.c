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

#include "../include/libonyx/libonyx.h"

void
threaddict_l_populate(cw_nxo_t *a_dict, cw_nxo_t *a_thread)
{
	cw_nxo_t	*tstack;
	cw_nxo_t	*name, *value;

#define NENTRIES	4	/* Number of entries in threaddict. */
	nxo_dict_new(a_dict, nxo_thread_nx_get(a_thread), FALSE, NENTRIES);

	tstack = nxo_thread_tstack_get(a_thread);
	name = nxo_stack_push(tstack);
	value = nxo_stack_push(tstack);

	/*
	 * Initialize entries that are not operators.
	 */

	/* threaddict. */
	nxo_name_new(name, nxo_thread_nx_get(a_thread), nxn_str(NXN_threaddict),
	    nxn_len(NXN_threaddict), TRUE);
	nxo_dup(value, a_dict);
	nxo_dict_def(a_dict, nxo_thread_nx_get(a_thread), name, value);

	/* errordict. */
	nxo_name_new(name, nxo_thread_nx_get(a_thread), nxn_str(NXN_errordict),
	    nxn_len(NXN_errordict), TRUE);
	nxo_dup(value, nxo_thread_errordict_get(a_thread));
	nxo_dict_def(a_dict, nxo_thread_nx_get(a_thread), name, value);

	/* currenterror. */
	nxo_name_new(name, nxo_thread_nx_get(a_thread),
	    nxn_str(NXN_currenterror), nxn_len(NXN_currenterror), TRUE);
	nxo_dup(value, nxo_thread_currenterror_get(a_thread));
	nxo_dict_def(a_dict, nxo_thread_nx_get(a_thread), name, value);

	/* userdict. */
	nxo_name_new(name, nxo_thread_nx_get(a_thread), nxn_str(NXN_userdict),
	    nxn_len(NXN_userdict), TRUE);
	nxo_dup(value, nxo_thread_userdict_get(a_thread));
	nxo_dict_def(a_dict, nxo_thread_nx_get(a_thread), name, value);

	nxo_stack_npop(tstack, 2);

#ifdef _LIBONYX_DBG
	if (nxo_dict_count(a_dict) != NENTRIES) {
		_cw_out_put_e("nxo_dict_count(a_dict) != NENTRIES "
		    "([i] != [i])\n", nxo_dict_count(a_dict), NENTRIES);
		_cw_error("Adjust NENTRIES");
	}
#endif
#undef NENTRIES
}
