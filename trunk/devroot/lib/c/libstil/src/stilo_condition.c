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
#include "../include/libstil/stilo_condition_l.h"
#include "../include/libstil/stilo_mutex_l.h"

void
stilo_condition_new(cw_stilo_t *a_stilo, cw_stil_t *a_stil)
{
	cw_stiloe_condition_t	*condition;

	condition = (cw_stiloe_condition_t
	    *)_cw_malloc(sizeof(cw_stiloe_condition_t));

	stiloe_l_new(&condition->stiloe, STILOT_CONDITION, FALSE);
	cnd_new(&condition->condition);

	memset(a_stilo, 0, sizeof(cw_stilo_t));
	a_stilo->o.stiloe = (cw_stiloe_t *)condition;
#ifdef _LIBSTIL_DBG
	a_stilo->magic = _CW_STILO_MAGIC;
#endif
	a_stilo->type = STILOT_CONDITION;

	stila_l_gc_register(stil_stila_get(a_stil), (cw_stiloe_t *)condition);
}

void
stiloe_l_condition_delete(cw_stiloe_t *a_stiloe, cw_stil_t *a_stil)
{
	cw_stiloe_condition_t	*condition;

	condition = (cw_stiloe_condition_t *)a_stiloe;

	_cw_check_ptr(condition);
	_cw_assert(condition->stiloe.magic == _CW_STILOE_MAGIC);
	_cw_assert(condition->stiloe.type == STILOT_CONDITION);

	cnd_delete(&condition->condition);

	_CW_STILOE_FREE(condition);
}

cw_stiloe_t *
stiloe_l_condition_ref_iter(cw_stiloe_t *a_stiloe, cw_bool_t a_reset)
{
	return NULL;
}

void
stilo_l_condition_print(cw_stilo_t *a_thread)
{
	cw_stilo_t		*ostack, *depth, *condition, *stdout_stilo;
	cw_stilo_threade_t	error;

	ostack = stilo_thread_ostack_get(a_thread);
	STILO_STACK_GET(depth, ostack, a_thread);
	STILO_STACK_DOWN_GET(condition, ostack, a_thread, depth);
	if (stilo_type_get(depth) != STILOT_INTEGER || stilo_type_get(condition)
	    != STILOT_CONDITION) {
		stilo_thread_error(a_thread, STILO_THREADE_TYPECHECK);
		return;
	}
	stdout_stilo = stil_stdout_get(stilo_thread_stil_get(a_thread));

	error = stilo_file_output(stdout_stilo, "-condition-");

	if (error) {
		stilo_thread_error(a_thread, error);
		return;
	}

	stilo_stack_npop(ostack, 2);
}

void
stilo_condition_signal(cw_stilo_t *a_stilo)
{
	cw_stiloe_condition_t	*condition;

	_cw_check_ptr(a_stilo);
	_cw_assert(a_stilo->magic == _CW_STILO_MAGIC);
	_cw_assert(a_stilo->type == STILOT_CONDITION);

	condition = (cw_stiloe_condition_t *)a_stilo->o.stiloe;

	_cw_check_ptr(condition);
	_cw_assert(condition->stiloe.magic == _CW_STILOE_MAGIC);
	_cw_assert(condition->stiloe.type == STILOT_CONDITION);

	cnd_signal(&condition->condition);
}

void
stilo_condition_broadcast(cw_stilo_t *a_stilo)
{
	cw_stiloe_condition_t	*condition;

	_cw_check_ptr(a_stilo);
	_cw_assert(a_stilo->magic == _CW_STILO_MAGIC);
	_cw_assert(a_stilo->type == STILOT_CONDITION);

	condition = (cw_stiloe_condition_t *)a_stilo->o.stiloe;

	_cw_check_ptr(condition);
	_cw_assert(condition->stiloe.magic == _CW_STILOE_MAGIC);
	_cw_assert(condition->stiloe.type == STILOT_CONDITION);

	cnd_broadcast(&condition->condition);
}

void
stilo_condition_wait(cw_stilo_t *a_stilo, cw_stilo_t *a_mutex)
{
	cw_stiloe_condition_t	*condition;
	cw_stiloe_mutex_t	*mutex;

	_cw_check_ptr(a_stilo);
	_cw_assert(a_stilo->magic == _CW_STILO_MAGIC);
	_cw_assert(a_stilo->type == STILOT_CONDITION);

	condition = (cw_stiloe_condition_t *)a_stilo->o.stiloe;

	_cw_check_ptr(condition);
	_cw_assert(condition->stiloe.magic == _CW_STILOE_MAGIC);
	_cw_assert(condition->stiloe.type == STILOT_CONDITION);

	_cw_check_ptr(a_mutex);
	_cw_assert(a_mutex->magic == _CW_STILO_MAGIC);
	_cw_assert(a_mutex->type == STILOT_MUTEX);

	mutex = (cw_stiloe_mutex_t *)a_mutex->o.stiloe;

	_cw_check_ptr(mutex);
	_cw_assert(mutex->stiloe.magic == _CW_STILOE_MAGIC);
	_cw_assert(mutex->stiloe.type == STILOT_MUTEX);

	cnd_wait(&condition->condition, &mutex->lock);
}

cw_bool_t
stilo_condition_timedwait(cw_stilo_t *a_stilo, cw_stilo_t *a_mutex, const struct
    timespec *a_timeout)
{
	cw_bool_t		retval;
	cw_stiloe_condition_t	*condition;
	cw_stiloe_mutex_t	*mutex;

	_cw_check_ptr(a_stilo);
	_cw_assert(a_stilo->magic == _CW_STILO_MAGIC);
	_cw_assert(a_stilo->type == STILOT_CONDITION);

	condition = (cw_stiloe_condition_t *)a_stilo->o.stiloe;

	_cw_check_ptr(condition);
	_cw_assert(condition->stiloe.magic == _CW_STILOE_MAGIC);
	_cw_assert(condition->stiloe.type == STILOT_CONDITION);

	_cw_check_ptr(a_mutex);
	_cw_assert(a_mutex->magic == _CW_STILO_MAGIC);
	_cw_assert(a_mutex->type == STILOT_MUTEX);

	mutex = (cw_stiloe_mutex_t *)a_mutex->o.stiloe;

	_cw_check_ptr(mutex);
	_cw_assert(mutex->stiloe.magic == _CW_STILOE_MAGIC);
	_cw_assert(mutex->stiloe.type == STILOT_MUTEX);

	retval = cnd_timedwait(&condition->condition, &mutex->lock, a_timeout);
	
	return retval;
}
