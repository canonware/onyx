/* -*- mode: c ; c-file-style: "canonware-c-style" -*-
 ******************************************************************************
 *
 * <Copyright = jasone>
 * <License>
 *
 ******************************************************************************
 *
 * Version: Onyx <Version = onyx>
 *
 ******************************************************************************/

#define CW_NXO_CONDITION_C_

#include "../include/libonyx/libonyx.h"
#include "../include/libonyx/nxa_l.h"
#include "../include/libonyx/nxo_l.h"
#include "../include/libonyx/nxo_condition_l.h"
#include "../include/libonyx/nxo_mutex_l.h"

void
nxo_condition_new(cw_nxo_t *a_nxo, cw_nx_t *a_nx)
{
    cw_nxoe_condition_t *condition;

    condition = (cw_nxoe_condition_t *) nxa_malloc(sizeof(cw_nxoe_condition_t));

    nxoe_l_new(&condition->nxoe, NXOT_CONDITION, FALSE);
    cnd_new(&condition->condition);

    nxo_no_new(a_nxo);
    a_nxo->o.nxoe = (cw_nxoe_t *) condition;
    nxo_p_type_set(a_nxo, NXOT_CONDITION);

    nxa_l_gc_register((cw_nxoe_t *) condition);
}

void
nxo_condition_signal(cw_nxo_t *a_nxo)
{
    cw_nxoe_condition_t *condition;

    cw_check_ptr(a_nxo);
    cw_dassert(a_nxo->magic == CW_NXO_MAGIC);
    cw_assert(nxo_type_get(a_nxo) == NXOT_CONDITION);

    condition = (cw_nxoe_condition_t *) a_nxo->o.nxoe;

    cw_check_ptr(condition);
    cw_dassert(condition->nxoe.magic == CW_NXOE_MAGIC);
    cw_assert(condition->nxoe.type == NXOT_CONDITION);

    cnd_signal(&condition->condition);
}

void
nxo_condition_broadcast(cw_nxo_t *a_nxo)
{
    cw_nxoe_condition_t *condition;

    cw_check_ptr(a_nxo);
    cw_dassert(a_nxo->magic == CW_NXO_MAGIC);
    cw_assert(nxo_type_get(a_nxo) == NXOT_CONDITION);

    condition = (cw_nxoe_condition_t *) a_nxo->o.nxoe;

    cw_check_ptr(condition);
    cw_dassert(condition->nxoe.magic == CW_NXOE_MAGIC);
    cw_assert(condition->nxoe.type == NXOT_CONDITION);

    cnd_broadcast(&condition->condition);
}

void
nxo_condition_wait(cw_nxo_t *a_nxo, cw_nxo_t *a_mutex)
{
    cw_nxoe_condition_t *condition;
    cw_nxoe_mutex_t *mutex;

    cw_check_ptr(a_nxo);
    cw_dassert(a_nxo->magic == CW_NXO_MAGIC);
    cw_assert(nxo_type_get(a_nxo) == NXOT_CONDITION);

    condition = (cw_nxoe_condition_t *) a_nxo->o.nxoe;

    cw_check_ptr(condition);
    cw_dassert(condition->nxoe.magic == CW_NXOE_MAGIC);
    cw_assert(condition->nxoe.type == NXOT_CONDITION);

    cw_check_ptr(a_mutex);
    cw_dassert(a_mutex->magic == CW_NXO_MAGIC);
    cw_assert(nxo_type_get(a_mutex) == NXOT_MUTEX);

    mutex = (cw_nxoe_mutex_t *) a_mutex->o.nxoe;

    cw_check_ptr(mutex);
    cw_dassert(mutex->nxoe.magic == CW_NXOE_MAGIC);
    cw_assert(mutex->nxoe.type == NXOT_MUTEX);

    cnd_wait(&condition->condition, &mutex->lock);
}

cw_bool_t
nxo_condition_timedwait(cw_nxo_t *a_nxo, cw_nxo_t *a_mutex,
			const struct timespec *a_timeout)
{
    cw_bool_t retval;
    cw_nxoe_condition_t *condition;
    cw_nxoe_mutex_t *mutex;

    cw_check_ptr(a_nxo);
    cw_dassert(a_nxo->magic == CW_NXO_MAGIC);
    cw_assert(nxo_type_get(a_nxo) == NXOT_CONDITION);

    condition = (cw_nxoe_condition_t *) a_nxo->o.nxoe;

    cw_check_ptr(condition);
    cw_dassert(condition->nxoe.magic == CW_NXOE_MAGIC);
    cw_assert(condition->nxoe.type == NXOT_CONDITION);

    cw_check_ptr(a_mutex);
    cw_dassert(a_mutex->magic == CW_NXO_MAGIC);
    cw_assert(nxo_type_get(a_mutex) == NXOT_MUTEX);

    mutex = (cw_nxoe_mutex_t *) a_mutex->o.nxoe;

    cw_check_ptr(mutex);
    cw_dassert(mutex->nxoe.magic == CW_NXOE_MAGIC);
    cw_assert(mutex->nxoe.type == NXOT_MUTEX);

    retval = cnd_timedwait(&condition->condition, &mutex->lock, a_timeout);
	
    return retval;
}
