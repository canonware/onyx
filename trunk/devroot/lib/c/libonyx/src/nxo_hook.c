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
#include "../include/libonyx/nxa_l.h"
#include "../include/libonyx/nxo_l.h"
#include "../include/libonyx/nxo_hook_l.h"
#include "../include/libonyx/nxo_name_l.h"

void
nxo_hook_new(cw_nxo_t *a_nxo, cw_nx_t *a_nx, void *a_data, cw_nxo_hook_eval_t
    *a_eval_f, cw_nxo_hook_ref_iter_t *a_ref_iter_f, cw_nxo_hook_delete_t
    *a_delete_f)
{
	cw_nxoe_hook_t	*hook;

	hook = (cw_nxoe_hook_t *)_cw_malloc(sizeof(cw_nxoe_hook_t));

	nxoe_l_new(&hook->nxoe, NXOT_HOOK, FALSE);
	nxo_null_new(&hook->tag);
	hook->data = a_data;
	hook->eval_f = a_eval_f;
	hook->ref_iter_f = a_ref_iter_f;
	hook->delete_f = a_delete_f;

	nxo_no_new(a_nxo);
	a_nxo->o.nxoe = (cw_nxoe_t *)hook;
	nxo_p_type_set(a_nxo, NXOT_HOOK);

	nxa_l_gc_register(nx_nxa_get(a_nx), (cw_nxoe_t *)hook);
}

void
nxoe_l_hook_delete(cw_nxoe_t *a_nxoe, cw_nx_t *a_nx)
{
	cw_nxoe_hook_t	*hook;

	hook = (cw_nxoe_hook_t *)a_nxoe;

	_cw_check_ptr(hook);
	_cw_dassert(hook->nxoe.magic == _CW_NXOE_MAGIC);
	_cw_assert(hook->nxoe.type == NXOT_HOOK);

	if (hook->delete_f != NULL)
		hook->delete_f(hook->data, a_nx);

	_CW_NXOE_FREE(hook);
}

cw_nxoe_t *
nxoe_l_hook_ref_iter(cw_nxoe_t *a_nxoe, cw_bool_t a_reset)
{
	cw_nxoe_t	*retval;
	cw_nxoe_hook_t	*hook;

	hook = (cw_nxoe_hook_t *)a_nxoe;

	if (a_reset)
		hook->ref_iter = 0;

	switch (hook->ref_iter) {
	case 0:
		hook->ref_iter++;
		retval = nxo_nxoe_get(&hook->tag);
		if (retval != NULL)
			break;
	case 1:
		hook->ref_iter++;
		if (hook->ref_iter_f != NULL)
			retval = hook->ref_iter_f(hook->data, TRUE);
		else
			retval = NULL;
		break;
	case 2:
		retval = hook->ref_iter_f(hook->data, FALSE);
		break;
	default:
		_cw_not_reached();
	}

	return retval;
}

void
nxo_l_hook_print(cw_nxo_t *a_thread)
{
	cw_nxo_t		*ostack, *depth, *nxo, *hnxo, *stdout_nxo;
	cw_nxoe_hook_t		*hook;
	cw_nxo_threade_t	error;

	ostack = nxo_thread_ostack_get(a_thread);
	NXO_STACK_GET(depth, ostack, a_thread);
	NXO_STACK_DOWN_GET(hnxo, ostack, a_thread, depth);
	if (nxo_type_get(depth) != NXOT_INTEGER || nxo_type_get(hnxo) !=
	    NXOT_HOOK) {
		nxo_thread_error(a_thread, NXO_THREADE_TYPECHECK);
		return;
	}
	stdout_nxo = nxo_thread_stdout_get(a_thread);

	hook = (cw_nxoe_hook_t *)hnxo->o.nxoe;

	_cw_check_ptr(hook);
	_cw_dassert(hook->nxoe.magic == _CW_NXOE_MAGIC);
	_cw_assert(hook->nxoe.type == NXOT_HOOK);


	if (nxo_type_get(&hook->tag) != NXOT_NULL) {
		error = nxo_file_output(stdout_nxo, "=");
		if (error) {
			nxo_thread_error(a_thread, error);
			return;
		}

		nxo = nxo_stack_push(ostack);
		nxo_dup(nxo, &hook->tag);
		nxo = nxo_stack_push(ostack);
		nxo_integer_new(nxo, nxo_integer_get(depth) - 1);
		_cw_onyx_code(a_thread,
		    "1 index type sprintdict exch get eval");

		error = nxo_file_output(stdout_nxo, "=");
		if (error) {
			nxo_thread_error(a_thread, error);
			return;
		}
	} else {
		error = nxo_file_output(stdout_nxo, "-hook-");
		if (error) {
			nxo_thread_error(a_thread, error);
			return;
		}
	}

	if (error) {
		nxo_thread_error(a_thread, error);
		return;
	}

	nxo_stack_npop(ostack, 2);
}

cw_nxo_t *
nxo_hook_tag_get(cw_nxo_t *a_nxo)
{
	cw_nxo_t	*retval;
	cw_nxoe_hook_t	*hook;

	_cw_check_ptr(a_nxo);
	_cw_dassert(a_nxo->magic == _CW_NXO_MAGIC);
	_cw_assert(nxo_type_get(a_nxo) == NXOT_HOOK);

	hook = (cw_nxoe_hook_t *)a_nxo->o.nxoe;

	_cw_check_ptr(hook);
	_cw_dassert(hook->nxoe.magic == _CW_NXOE_MAGIC);
	_cw_assert(hook->nxoe.type == NXOT_HOOK);

	retval = &hook->tag;

	return retval;
}

void *
nxo_hook_data_get(cw_nxo_t *a_nxo)
{
	void		*retval;
	cw_nxoe_hook_t	*hook;

	_cw_check_ptr(a_nxo);
	_cw_dassert(a_nxo->magic == _CW_NXO_MAGIC);
	_cw_assert(nxo_type_get(a_nxo) == NXOT_HOOK);

	hook = (cw_nxoe_hook_t *)a_nxo->o.nxoe;

	_cw_check_ptr(hook);
	_cw_dassert(hook->nxoe.magic == _CW_NXOE_MAGIC);
	_cw_assert(hook->nxoe.type == NXOT_HOOK);

	retval = hook->data;

	return retval;
}

void
nxo_hook_eval(cw_nxo_t *a_nxo, cw_nxo_t *a_thread)
{
	cw_nxoe_hook_t		*hook;

	_cw_check_ptr(a_nxo);
	_cw_dassert(a_nxo->magic == _CW_NXO_MAGIC);
	_cw_assert(nxo_type_get(a_nxo) == NXOT_HOOK);

	hook = (cw_nxoe_hook_t *)a_nxo->o.nxoe;

	_cw_check_ptr(hook);
	_cw_dassert(hook->nxoe.magic == _CW_NXOE_MAGIC);
	_cw_assert(hook->nxoe.type == NXOT_HOOK);

	if (hook->eval_f != NULL)
		hook->eval_f(hook->data, a_thread);
	else {
		cw_nxo_t	*ostack, *nxo;

		/*
		 * This hook can't be executed, so push it onto ostack just like
		 * a normal literal object would be.
		 */
		ostack = nxo_thread_ostack_get(a_thread);
		nxo = nxo_stack_push(ostack);
		nxo_dup(nxo, a_nxo);
	}
}
