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

cw_stilte_t
stilo_l_condition_print(cw_stilo_t *a_stilo, cw_stilo_t *a_file, cw_uint32_t
    a_depth)
{
	cw_stilte_t	retval;

	retval = stilo_file_output(a_file, "-condition-");

	return retval;
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
