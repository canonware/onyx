/* -*- mode: c ; c-file-style: "canonware-c-style" -*-
 ******************************************************************************
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
#include "../include/libonyx/nx_l.h"
#include "../include/libonyx/nxa_l.h"
#include "../include/libonyx/nxo_l.h"
#include "../include/libonyx/nxo_mutex_l.h"

void
nxo_mutex_new(cw_nxo_t *a_nxo, cw_nx_t *a_nx)
{
    cw_nxoe_mutex_t *mutex;

    mutex = (cw_nxoe_mutex_t *) nxa_malloc(nx_nxa_get(a_nx),
					  sizeof(cw_nxoe_mutex_t));

    nxoe_l_new(&mutex->nxoe, NXOT_MUTEX, FALSE);
    mtx_new(&mutex->lock);

    nxo_no_new(a_nxo);
    a_nxo->o.nxoe = (cw_nxoe_t *) mutex;
    nxo_p_type_set(a_nxo, NXOT_MUTEX);

    nxa_l_gc_register(nx_nxa_get(a_nx), (cw_nxoe_t *) mutex);
}

cw_bool_t
nxoe_l_mutex_delete(cw_nxoe_t *a_nxoe, cw_nxa_t *a_nxa, cw_uint32_t a_iter)
{
    cw_nxoe_mutex_t *mutex;

    mutex = (cw_nxoe_mutex_t *) a_nxoe;

    cw_check_ptr(mutex);
    cw_dassert(mutex->nxoe.magic == CW_NXOE_MAGIC);
    cw_assert(mutex->nxoe.type == NXOT_MUTEX);

    mtx_delete(&mutex->lock);

    nxa_free(a_nxa, mutex, sizeof(cw_nxoe_mutex_t));

    return FALSE;
}

cw_nxoe_t *
nxoe_l_mutex_ref_iter(cw_nxoe_t *a_nxo, cw_bool_t a_reset)
{
    return NULL;
}

void
nxo_mutex_lock(cw_nxo_t *a_nxo)
{
    cw_nxoe_mutex_t *mutex;

    cw_check_ptr(a_nxo);
    cw_dassert(a_nxo->magic == CW_NXO_MAGIC);
    cw_assert(nxo_type_get(a_nxo) == NXOT_MUTEX);

    mutex = (cw_nxoe_mutex_t *) a_nxo->o.nxoe;

    cw_check_ptr(mutex);
    cw_dassert(mutex->nxoe.magic == CW_NXOE_MAGIC);
    cw_assert(mutex->nxoe.type == NXOT_MUTEX);

    mtx_lock(&mutex->lock);
}

cw_bool_t
nxo_mutex_trylock(cw_nxo_t *a_nxo)
{
    cw_bool_t retval;
    cw_nxoe_mutex_t *mutex;

    cw_check_ptr(a_nxo);
    cw_dassert(a_nxo->magic == CW_NXO_MAGIC);
    cw_assert(nxo_type_get(a_nxo) == NXOT_MUTEX);

    mutex = (cw_nxoe_mutex_t *) a_nxo->o.nxoe;

    cw_check_ptr(mutex);
    cw_dassert(mutex->nxoe.magic == CW_NXOE_MAGIC);
    cw_assert(mutex->nxoe.type == NXOT_MUTEX);

    retval = mtx_trylock(&mutex->lock);

    return retval;
}

void
nxo_mutex_unlock(cw_nxo_t *a_nxo)
{
    cw_nxoe_mutex_t *mutex;

    cw_check_ptr(a_nxo);
    cw_dassert(a_nxo->magic == CW_NXO_MAGIC);
    cw_assert(nxo_type_get(a_nxo) == NXOT_MUTEX);

    mutex = (cw_nxoe_mutex_t *) a_nxo->o.nxoe;

    cw_check_ptr(mutex);
    cw_dassert(mutex->nxoe.magic == CW_NXOE_MAGIC);
    cw_assert(mutex->nxoe.type == NXOT_MUTEX);

    mtx_unlock(&mutex->lock);
}
