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

/* Default setting for whether to record stacks on error. */
#define	CURRENTERROR_RECORDSTACKS	TRUE

void
currenterror_l_populate(cw_nxo_t *a_dict, cw_nxo_t *a_thread)
{
	cw_nxo_t	*tstack;
	cw_nxo_t	*name, *val;

#define NENTRIES	11	/* Number of entries in currenterror. */
	nxo_dict_new(a_dict, nxo_thread_nx_get(a_thread),
	    nxo_thread_currentlocking(a_thread), NENTRIES);

	tstack = nxo_thread_tstack_get(a_thread);
	name = nxo_stack_push(tstack);
	val = nxo_stack_push(tstack);

	/*
	 * Initialize operators.
	 */
	nxo_name_new(name, nxo_thread_nx_get(a_thread),
	    nxn_str(NXN_stop), nxn_len(NXN_stop), TRUE);
	nxo_operator_new(val, systemdict_stop, NXN_stop);
	nxo_attr_set(val, NXOA_EXECUTABLE);
	nxo_dict_def(a_dict, nxo_thread_nx_get(a_thread), name, val);

	/*
	 * Initialize entries that are not operators.
	 */
	nxo_name_new(name, nxo_thread_nx_get(a_thread),
	    nxn_str(NXN_newerror), nxn_len(NXN_newerror), TRUE);
	nxo_boolean_new(val, FALSE);
	nxo_dict_def(a_dict, nxo_thread_nx_get(a_thread), name, val);

	nxo_name_new(name, nxo_thread_nx_get(a_thread),
	    nxn_str(NXN_errorname), nxn_len(NXN_errorname), TRUE);
	nxo_string_new(val, nxo_thread_nx_get(a_thread),
	    nxo_thread_currentlocking(a_thread), 0);
	nxo_dict_def(a_dict, nxo_thread_nx_get(a_thread), name, val);

	nxo_name_new(name, nxo_thread_nx_get(a_thread),
	    nxn_str(NXN_command), nxn_len(NXN_command), TRUE);
	nxo_null_new(val);
	nxo_dict_def(a_dict, nxo_thread_nx_get(a_thread), name, val);

	nxo_name_new(name, nxo_thread_nx_get(a_thread),
	    nxn_str(NXN_estack), nxn_len(NXN_estack), TRUE);
	nxo_stack_new(val, nxo_thread_nx_get(a_thread), FALSE);
	nxo_dict_def(a_dict, nxo_thread_nx_get(a_thread), name, val);

	nxo_name_new(name, nxo_thread_nx_get(a_thread),
	    nxn_str(NXN_istack), nxn_len(NXN_istack), TRUE);
	nxo_stack_new(val, nxo_thread_nx_get(a_thread), FALSE);
	nxo_dict_def(a_dict, nxo_thread_nx_get(a_thread), name, val);

	nxo_name_new(name, nxo_thread_nx_get(a_thread),
	    nxn_str(NXN_ostack), nxn_len(NXN_ostack), TRUE);
	nxo_stack_new(val, nxo_thread_nx_get(a_thread), FALSE);
	nxo_dict_def(a_dict, nxo_thread_nx_get(a_thread), name, val);

	nxo_name_new(name, nxo_thread_nx_get(a_thread),
	    nxn_str(NXN_dstack), nxn_len(NXN_dstack), TRUE);
	nxo_stack_new(val, nxo_thread_nx_get(a_thread), FALSE);
	nxo_dict_def(a_dict, nxo_thread_nx_get(a_thread), name, val);

	nxo_name_new(name, nxo_thread_nx_get(a_thread),
	    nxn_str(NXN_recordstacks), nxn_len(NXN_recordstacks), TRUE);
	nxo_boolean_new(val, CURRENTERROR_RECORDSTACKS);
	nxo_dict_def(a_dict, nxo_thread_nx_get(a_thread), name, val);

	nxo_name_new(name, nxo_thread_nx_get(a_thread),
	    nxn_str(NXN_line), nxn_len(NXN_line), TRUE);
	nxo_integer_new(val, 1);
	nxo_dict_def(a_dict, nxo_thread_nx_get(a_thread), name, val);
	
	nxo_name_new(name, nxo_thread_nx_get(a_thread),
	    nxn_str(NXN_column), nxn_len(NXN_column), TRUE);
	nxo_integer_new(val, 1);
	nxo_dict_def(a_dict, nxo_thread_nx_get(a_thread), name, val);
	
	nxo_stack_npop(tstack, 2);

#ifdef _CW_DBG
	if (nxo_dict_count(a_dict) != NENTRIES) {
		_cw_out_put_e("nxo_dict_count(a_dict) != NENTRIES "
		    "([i] != [i])\n", nxo_dict_count(a_dict), NENTRIES);
		_cw_error("Adjust NENTRIES");
	}
#endif
#undef NENTRIES
}
