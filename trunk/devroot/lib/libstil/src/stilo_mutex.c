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
#include "../include/libstil/stil_l.h"
#include "../include/libstil/stila_l.h"
#include "../include/libstil/stilo_l.h"
#include "../include/libstil/stilo_mutex_l.h"

void
stilo_mutex_new(cw_stilo_t *a_stilo, cw_stil_t *a_stil)
{
	cw_stiloe_mutex_t	*mutex;

	mutex = (cw_stiloe_mutex_t *)_cw_malloc(sizeof(cw_stiloe_mutex_t));

	stiloe_l_new(&mutex->stiloe, STILOT_MUTEX, FALSE);
	mtx_new(&mutex->lock);

	memset(a_stilo, 0, sizeof(cw_stilo_t));
	a_stilo->o.stiloe = (cw_stiloe_t *)mutex;
#ifdef _LIBSTIL_DBG
	a_stilo->magic = _CW_STILO_MAGIC;
#endif
	a_stilo->type = STILOT_MUTEX;

	stila_l_gc_register(stil_stila_get(a_stil), (cw_stiloe_t *)mutex);
}

void
stiloe_l_mutex_delete(cw_stiloe_t *a_stiloe, cw_stil_t *a_stil)
{
	cw_stiloe_mutex_t	*mutex;

	mutex = (cw_stiloe_mutex_t *)a_stiloe;

	_cw_check_ptr(mutex);
	_cw_assert(mutex->stiloe.magic == _CW_STILOE_MAGIC);
	_cw_assert(mutex->stiloe.type == STILOT_MUTEX);

	mtx_delete(&mutex->lock);

	_CW_STILOE_FREE(mutex);
}

cw_stiloe_t *
stiloe_l_mutex_ref_iter(cw_stiloe_t *a_stilo, cw_bool_t a_reset)
{
	return NULL;
}

cw_stilte_t
stilo_l_mutex_print(cw_stilo_t *a_stilo, cw_stilo_t *a_file, cw_uint32_t
    a_depth)
{
	cw_stilte_t	retval;

	retval = stilo_file_output(a_file, "-mutex-");

	return retval;
}

void
stilo_mutex_lock(cw_stilo_t *a_stilo)
{
	cw_stiloe_mutex_t	*mutex;

	_cw_check_ptr(a_stilo);
	_cw_assert(a_stilo->magic == _CW_STILO_MAGIC);
	_cw_assert(a_stilo->type == STILOT_MUTEX);

	mutex = (cw_stiloe_mutex_t *)a_stilo->o.stiloe;

	_cw_check_ptr(mutex);
	_cw_assert(mutex->stiloe.magic == _CW_STILOE_MAGIC);
	_cw_assert(mutex->stiloe.type == STILOT_MUTEX);

	mtx_lock(&mutex->lock);
}

cw_bool_t
stilo_mutex_trylock(cw_stilo_t *a_stilo)
{
	cw_bool_t		retval;
	cw_stiloe_mutex_t	*mutex;

	_cw_check_ptr(a_stilo);
	_cw_assert(a_stilo->magic == _CW_STILO_MAGIC);
	_cw_assert(a_stilo->type == STILOT_MUTEX);

	mutex = (cw_stiloe_mutex_t *)a_stilo->o.stiloe;

	_cw_check_ptr(mutex);
	_cw_assert(mutex->stiloe.magic == _CW_STILOE_MAGIC);
	_cw_assert(mutex->stiloe.type == STILOT_MUTEX);

	retval = mtx_trylock(&mutex->lock);

	return retval;
}

void
stilo_mutex_unlock(cw_stilo_t *a_stilo)
{
	cw_stiloe_mutex_t	*mutex;

	_cw_check_ptr(a_stilo);
	_cw_assert(a_stilo->magic == _CW_STILO_MAGIC);
	_cw_assert(a_stilo->type == STILOT_MUTEX);

	mutex = (cw_stiloe_mutex_t *)a_stilo->o.stiloe;

	_cw_check_ptr(mutex);
	_cw_assert(mutex->stiloe.magic == _CW_STILOE_MAGIC);
	_cw_assert(mutex->stiloe.type == STILOT_MUTEX);

	mtx_unlock(&mutex->lock);
}
