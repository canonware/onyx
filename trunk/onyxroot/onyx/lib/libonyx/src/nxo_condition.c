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
#include "../include/libonyx/nxo_condition_l.h"
#include "../include/libonyx/nxo_mutex_l.h"

void
nxo_condition_new(cw_nxo_t *a_nxo, cw_nx_t *a_nx)
{
	cw_nxoe_condition_t	*condition;

	condition = (cw_nxoe_condition_t
	    *)_cw_malloc(sizeof(cw_nxoe_condition_t));

	nxoe_l_new(&condition->nxoe, NXOT_CONDITION, FALSE);
	cnd_new(&condition->condition);

	memset(a_nxo, 0, sizeof(cw_nxo_t));
	a_nxo->o.nxoe = (cw_nxoe_t *)condition;
#ifdef _LIBONYX_DBG
	a_nxo->magic = _CW_NXO_MAGIC;
#endif
	a_nxo->type = NXOT_CONDITION;

	nxa_l_gc_register(nx_nxa_get(a_nx), (cw_nxoe_t *)condition);
}

void
nxoe_l_condition_delete(cw_nxoe_t *a_nxoe, cw_nx_t *a_nx)
{
	cw_nxoe_condition_t	*condition;

	condition = (cw_nxoe_condition_t *)a_nxoe;

	_cw_check_ptr(condition);
	_cw_assert(condition->nxoe.magic == _CW_NXOE_MAGIC);
	_cw_assert(condition->nxoe.type == NXOT_CONDITION);

	cnd_delete(&condition->condition);

	_CW_NXOE_FREE(condition);
}

cw_nxoe_t *
nxoe_l_condition_ref_iter(cw_nxoe_t *a_nxoe, cw_bool_t a_reset)
{
	return NULL;
}

void
nxo_l_condition_print(cw_nxo_t *a_thread)
{
	cw_nxo_t		*ostack, *depth, *condition, *stdout_nxo;
	cw_nxo_threade_t	error;

	ostack = nxo_thread_ostack_get(a_thread);
	NXO_STACK_GET(depth, ostack, a_thread);
	NXO_STACK_DOWN_GET(condition, ostack, a_thread, depth);
	if (nxo_type_get(depth) != NXOT_INTEGER || nxo_type_get(condition) !=
	    NXOT_CONDITION) {
		nxo_thread_error(a_thread, NXO_THREADE_TYPECHECK);
		return;
	}
	stdout_nxo = nx_stdout_get(nxo_thread_nx_get(a_thread));

	error = nxo_file_output(stdout_nxo, "-condition-");

	if (error) {
		nxo_thread_error(a_thread, error);
		return;
	}

	nxo_stack_npop(ostack, 2);
}

void
nxo_condition_signal(cw_nxo_t *a_nxo)
{
	cw_nxoe_condition_t	*condition;

	_cw_check_ptr(a_nxo);
	_cw_assert(a_nxo->magic == _CW_NXO_MAGIC);
	_cw_assert(a_nxo->type == NXOT_CONDITION);

	condition = (cw_nxoe_condition_t *)a_nxo->o.nxoe;

	_cw_check_ptr(condition);
	_cw_assert(condition->nxoe.magic == _CW_NXOE_MAGIC);
	_cw_assert(condition->nxoe.type == NXOT_CONDITION);

	cnd_signal(&condition->condition);
}

void
nxo_condition_broadcast(cw_nxo_t *a_nxo)
{
	cw_nxoe_condition_t	*condition;

	_cw_check_ptr(a_nxo);
	_cw_assert(a_nxo->magic == _CW_NXO_MAGIC);
	_cw_assert(a_nxo->type == NXOT_CONDITION);

	condition = (cw_nxoe_condition_t *)a_nxo->o.nxoe;

	_cw_check_ptr(condition);
	_cw_assert(condition->nxoe.magic == _CW_NXOE_MAGIC);
	_cw_assert(condition->nxoe.type == NXOT_CONDITION);

	cnd_broadcast(&condition->condition);
}

void
nxo_condition_wait(cw_nxo_t *a_nxo, cw_nxo_t *a_mutex)
{
	cw_nxoe_condition_t	*condition;
	cw_nxoe_mutex_t		*mutex;

	_cw_check_ptr(a_nxo);
	_cw_assert(a_nxo->magic == _CW_NXO_MAGIC);
	_cw_assert(a_nxo->type == NXOT_CONDITION);

	condition = (cw_nxoe_condition_t *)a_nxo->o.nxoe;

	_cw_check_ptr(condition);
	_cw_assert(condition->nxoe.magic == _CW_NXOE_MAGIC);
	_cw_assert(condition->nxoe.type == NXOT_CONDITION);

	_cw_check_ptr(a_mutex);
	_cw_assert(a_mutex->magic == _CW_NXO_MAGIC);
	_cw_assert(a_mutex->type == NXOT_MUTEX);

	mutex = (cw_nxoe_mutex_t *)a_mutex->o.nxoe;

	_cw_check_ptr(mutex);
	_cw_assert(mutex->nxoe.magic == _CW_NXOE_MAGIC);
	_cw_assert(mutex->nxoe.type == NXOT_MUTEX);

	cnd_wait(&condition->condition, &mutex->lock);
}

cw_bool_t
nxo_condition_timedwait(cw_nxo_t *a_nxo, cw_nxo_t *a_mutex, const struct
    timespec *a_timeout)
{
	cw_bool_t		retval;
	cw_nxoe_condition_t	*condition;
	cw_nxoe_mutex_t		*mutex;

	_cw_check_ptr(a_nxo);
	_cw_assert(a_nxo->magic == _CW_NXO_MAGIC);
	_cw_assert(a_nxo->type == NXOT_CONDITION);

	condition = (cw_nxoe_condition_t *)a_nxo->o.nxoe;

	_cw_check_ptr(condition);
	_cw_assert(condition->nxoe.magic == _CW_NXOE_MAGIC);
	_cw_assert(condition->nxoe.type == NXOT_CONDITION);

	_cw_check_ptr(a_mutex);
	_cw_assert(a_mutex->magic == _CW_NXO_MAGIC);
	_cw_assert(a_mutex->type == NXOT_MUTEX);

	mutex = (cw_nxoe_mutex_t *)a_mutex->o.nxoe;

	_cw_check_ptr(mutex);
	_cw_assert(mutex->nxoe.magic == _CW_NXOE_MAGIC);
	_cw_assert(mutex->nxoe.type == NXOT_MUTEX);

	retval = cnd_timedwait(&condition->condition, &mutex->lock, a_timeout);
	
	return retval;
}
