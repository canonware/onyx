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
#include "../include/libstil/stila_l.h"
#include "../include/libstil/stilo_l.h"
#include "../include/libstil/stilo_hook_l.h"
#include "../include/libstil/stilo_name_l.h"

void
stilo_hook_new(cw_stilo_t *a_stilo, cw_stil_t *a_stil, void *a_data,
    cw_stilo_hook_eval_t *a_eval_f, cw_stilo_hook_ref_iter_t
    *a_ref_iter_f, cw_stilo_hook_delete_t *a_delete_f)
{
	cw_stiloe_hook_t	*hook;

	hook = (cw_stiloe_hook_t *)_cw_malloc(sizeof(cw_stiloe_hook_t));

	stiloe_l_new(&hook->stiloe, STILOT_HOOK, FALSE);
	stilo_null_new(&hook->tag);
	hook->data = a_data;
	hook->eval_f = a_eval_f;
	hook->ref_iter_f = a_ref_iter_f;
	hook->delete_f = a_delete_f;

	memset(a_stilo, 0, sizeof(cw_stilo_t));
	a_stilo->o.stiloe = (cw_stiloe_t *)hook;
#ifdef _LIBSTIL_DBG
	a_stilo->magic = _CW_STILO_MAGIC;
#endif
	a_stilo->type = STILOT_HOOK;

	stila_l_gc_register(stil_stila_get(a_stil), (cw_stiloe_t *)hook);
}

void
stiloe_l_hook_delete(cw_stiloe_t *a_stiloe, cw_stil_t *a_stil)
{
	cw_stiloe_hook_t	*hook;

	hook = (cw_stiloe_hook_t *)a_stiloe;

	_cw_check_ptr(hook);
	_cw_assert(hook->stiloe.magic == _CW_STILOE_MAGIC);
	_cw_assert(hook->stiloe.type == STILOT_HOOK);

	if (hook->delete_f != NULL)
		hook->delete_f(hook->data, a_stil);

	_CW_STILOE_FREE(hook);
}

cw_stiloe_t *
stiloe_l_hook_ref_iter(cw_stiloe_t *a_stiloe, cw_bool_t a_reset)
{
	cw_stiloe_t		*retval;
	cw_stiloe_hook_t	*hook;

	hook = (cw_stiloe_hook_t *)a_stiloe;

	if (a_reset)
		hook->ref_iter = 0;

	switch (hook->ref_iter) {
	case 0:
		hook->ref_iter++;
		retval = stilo_stiloe_get(&hook->tag);
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
stilo_l_hook_print(cw_stilo_t *a_thread)
{
	cw_stilo_t		*ostack, *depth, *stilo, *hstilo, *stdout_stilo;
	cw_stiloe_hook_t	*hook;
	cw_stilo_threade_t	error;

	ostack = stilo_thread_ostack_get(a_thread);
	STILO_STACK_GET(depth, ostack, a_thread);
	STILO_STACK_DOWN_GET(hstilo, ostack, a_thread, depth);
	if (stilo_type_get(depth) != STILOT_INTEGER || stilo_type_get(hstilo)
	    != STILOT_HOOK) {
		stilo_thread_error(a_thread, STILO_THREADE_TYPECHECK);
		return;
	}
	stdout_stilo = stil_stdout_get(stilo_thread_stil_get(a_thread));

	hook = (cw_stiloe_hook_t *)hstilo->o.stiloe;

	_cw_check_ptr(hook);
	_cw_assert(hook->stiloe.magic == _CW_STILOE_MAGIC);
	_cw_assert(hook->stiloe.type == STILOT_HOOK);


	if (stilo_type_get(&hook->tag) != STILOT_NULL) {
		error = stilo_file_output(stdout_stilo, "=");
		if (error) {
			stilo_thread_error(a_thread, error);
			return;
		}

		stilo = stilo_stack_push(ostack);
		stilo_dup(stilo, &hook->tag);
		stilo = stilo_stack_push(ostack);
		stilo_integer_new(stilo, stilo_integer_get(depth) - 1);
		_cw_stil_code(a_thread,
		    "1 index type sprintdict exch get eval");

		error = stilo_file_output(stdout_stilo, "=");
		if (error) {
			stilo_thread_error(a_thread, error);
			return;
		}
	} else {
		error = stilo_file_output(stdout_stilo, "-hook-");
		if (error) {
			stilo_thread_error(a_thread, error);
			return;
		}
	}

	if (error) {
		stilo_thread_error(a_thread, error);
		return;
	}

	stilo_stack_npop(ostack, 2);
}

cw_stilo_t *
stilo_hook_tag_get(cw_stilo_t *a_stilo)
{
	cw_stilo_t		*retval;
	cw_stiloe_hook_t	*hook;

	_cw_check_ptr(a_stilo);
	_cw_assert(a_stilo->magic == _CW_STILO_MAGIC);
	_cw_assert(a_stilo->type == STILOT_HOOK);

	hook = (cw_stiloe_hook_t *)a_stilo->o.stiloe;

	_cw_check_ptr(hook);
	_cw_assert(hook->stiloe.magic == _CW_STILOE_MAGIC);
	_cw_assert(hook->stiloe.type == STILOT_HOOK);

	retval = &hook->tag;

	return retval;
}

void *
stilo_hook_data_get(cw_stilo_t *a_stilo)
{
	void			*retval;
	cw_stiloe_hook_t	*hook;

	_cw_check_ptr(a_stilo);
	_cw_assert(a_stilo->magic == _CW_STILO_MAGIC);
	_cw_assert(a_stilo->type == STILOT_HOOK);

	hook = (cw_stiloe_hook_t *)a_stilo->o.stiloe;

	_cw_check_ptr(hook);
	_cw_assert(hook->stiloe.magic == _CW_STILOE_MAGIC);
	_cw_assert(hook->stiloe.type == STILOT_HOOK);

	retval = hook->data;

	return retval;
}

cw_stilo_threade_t
stilo_hook_eval(cw_stilo_t *a_stilo, cw_stilo_t *a_thread)
{
	cw_stilo_threade_t	retval;
	cw_stiloe_hook_t	*hook;

	_cw_check_ptr(a_stilo);
	_cw_assert(a_stilo->magic == _CW_STILO_MAGIC);
	_cw_assert(a_stilo->type == STILOT_HOOK);

	hook = (cw_stiloe_hook_t *)a_stilo->o.stiloe;

	_cw_check_ptr(hook);
	_cw_assert(hook->stiloe.magic == _CW_STILOE_MAGIC);
	_cw_assert(hook->stiloe.type == STILOT_HOOK);

	if (hook->eval_f == NULL) {
		retval = STILO_THREADE_INVALIDACCESS;
		goto RETURN;
	}

	retval = hook->eval_f(hook->data, a_thread);
	RETURN:
	return retval;
}
