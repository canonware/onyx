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

void
stilo_hook_new(cw_stilo_t *a_stilo, cw_stil_t *a_stil, void *a_data,
    cw_stilo_hook_exec_t *a_exec_f, cw_stilo_hook_ref_iter_t
    *a_ref_iter_f, cw_stilo_hook_delete_t *a_delete_f)
{
	cw_stiloe_hook_t	*hook;

	hook = (cw_stiloe_hook_t *)_cw_malloc(sizeof(cw_stiloe_hook_t));

	stiloe_l_new(&hook->stiloe, STILOT_HOOK, FALSE);
	hook->data = a_data;
	hook->exec_f = a_exec_f;
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
	_cw_assert(hook->stiloe.type == STILOT_FILE);

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

	if (hook->ref_iter_f != NULL)
		retval = hook->ref_iter_f(hook->data, a_reset);
	else
		retval = NULL;

	return retval;
}

cw_stilo_threade_t
stilo_l_hook_print(cw_stilo_t *a_stilo, cw_stilo_t *a_file, cw_uint32_t
    a_depth)
{
	cw_stilo_threade_t	retval;

	retval = stilo_file_output(a_file, "-hook-");

	return retval;
}

void *
stilo_hook_get(cw_stilo_t *a_stilo)
{
	void			*retval;
	cw_stiloe_hook_t	*hook;

	_cw_check_ptr(a_stilo);
	_cw_assert(a_stilo->magic == _CW_STILO_MAGIC);
	_cw_assert(a_stilo->type == STILOT_HOOK);

	hook = (cw_stiloe_hook_t *)a_stilo->o.stiloe;

	_cw_check_ptr(hook);
	_cw_assert(hook->stiloe.magic == _CW_STILOE_MAGIC);
	_cw_assert(hook->stiloe.type == STILOT_FILE);

	retval = hook->data;

	return retval;
}

cw_stilo_threade_t
stilo_hook_exec(cw_stilo_t *a_stilo, cw_stilo_t *a_thread)
{
	cw_stilo_threade_t	retval;
	cw_stiloe_hook_t	*hook;

	_cw_check_ptr(a_stilo);
	_cw_assert(a_stilo->magic == _CW_STILO_MAGIC);
	_cw_assert(a_stilo->type == STILOT_HOOK);

	hook = (cw_stiloe_hook_t *)a_stilo->o.stiloe;

	_cw_check_ptr(hook);
	_cw_assert(hook->stiloe.magic == _CW_STILOE_MAGIC);
	_cw_assert(hook->stiloe.type == STILOT_FILE);

	if (hook->exec_f == NULL) {
		retval = STILO_THREADE_INVALIDACCESS;
		goto RETURN;
	}

	retval = hook->exec_f(hook->data, a_thread);
	RETURN:
	return retval;
}
