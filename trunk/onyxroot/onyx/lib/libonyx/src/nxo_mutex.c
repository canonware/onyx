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
#include "../include/libonyx/nx_l.h"
#include "../include/libonyx/nxa_l.h"
#include "../include/libonyx/nxo_l.h"
#include "../include/libonyx/nxo_mutex_l.h"

void
nxo_mutex_new(cw_nxo_t *a_nxo, cw_nx_t *a_nx)
{
	cw_nxoe_mutex_t	*mutex;

	mutex = (cw_nxoe_mutex_t *)_cw_malloc(sizeof(cw_nxoe_mutex_t));

	nxoe_l_new(&mutex->nxoe, NXOT_MUTEX, FALSE);
	mtx_new(&mutex->lock);

	nxo_no_new(a_nxo);
	a_nxo->o.nxoe = (cw_nxoe_t *)mutex;
#ifdef _LIBONYX_DBG
	a_nxo->magic = _CW_NXO_MAGIC;
#endif
	nxo_p_type_set(a_nxo, NXOT_MUTEX);

	nxa_l_gc_register(nx_nxa_get(a_nx), (cw_nxoe_t *)mutex);
}

void
nxoe_l_mutex_delete(cw_nxoe_t *a_nxoe, cw_nx_t *a_nx)
{
	cw_nxoe_mutex_t	*mutex;

	mutex = (cw_nxoe_mutex_t *)a_nxoe;

	_cw_check_ptr(mutex);
	_cw_assert(mutex->nxoe.magic == _CW_NXOE_MAGIC);
	_cw_assert(mutex->nxoe.type == NXOT_MUTEX);

	mtx_delete(&mutex->lock);

	_CW_NXOE_FREE(mutex);
}

cw_nxoe_t *
nxoe_l_mutex_ref_iter(cw_nxoe_t *a_nxo, cw_bool_t a_reset)
{
	return NULL;
}

void
nxo_l_mutex_print(cw_nxo_t *a_thread)
{
	cw_nxo_t		*ostack, *depth, *mutex, *stdout_nxo;
	cw_nxo_threade_t	error;

	ostack = nxo_thread_ostack_get(a_thread);
	NXO_STACK_GET(depth, ostack, a_thread);
	NXO_STACK_DOWN_GET(mutex, ostack, a_thread, depth);
	if (nxo_type_get(depth) != NXOT_INTEGER || nxo_type_get(mutex) !=
	    NXOT_MUTEX) {
		nxo_thread_error(a_thread, NXO_THREADE_TYPECHECK);
		return;
	}
	stdout_nxo = nxo_thread_stdout_get(a_thread);

	error = nxo_file_output(stdout_nxo, "-mutex-");

	if (error) {
		nxo_thread_error(a_thread, error);
		return;
	}

	nxo_stack_npop(ostack, 2);
}

void
nxo_mutex_lock(cw_nxo_t *a_nxo)
{
	cw_nxoe_mutex_t	*mutex;

	_cw_check_ptr(a_nxo);
	_cw_assert(a_nxo->magic == _CW_NXO_MAGIC);
	_cw_assert(nxo_type_get(a_nxo) == NXOT_MUTEX);

	mutex = (cw_nxoe_mutex_t *)a_nxo->o.nxoe;

	_cw_check_ptr(mutex);
	_cw_assert(mutex->nxoe.magic == _CW_NXOE_MAGIC);
	_cw_assert(mutex->nxoe.type == NXOT_MUTEX);

	mtx_lock(&mutex->lock);
}

cw_bool_t
nxo_mutex_trylock(cw_nxo_t *a_nxo)
{
	cw_bool_t	retval;
	cw_nxoe_mutex_t	*mutex;

	_cw_check_ptr(a_nxo);
	_cw_assert(a_nxo->magic == _CW_NXO_MAGIC);
	_cw_assert(nxo_type_get(a_nxo) == NXOT_MUTEX);

	mutex = (cw_nxoe_mutex_t *)a_nxo->o.nxoe;

	_cw_check_ptr(mutex);
	_cw_assert(mutex->nxoe.magic == _CW_NXOE_MAGIC);
	_cw_assert(mutex->nxoe.type == NXOT_MUTEX);

	retval = mtx_trylock(&mutex->lock);

	return retval;
}

void
nxo_mutex_unlock(cw_nxo_t *a_nxo)
{
	cw_nxoe_mutex_t	*mutex;

	_cw_check_ptr(a_nxo);
	_cw_assert(a_nxo->magic == _CW_NXO_MAGIC);
	_cw_assert(nxo_type_get(a_nxo) == NXOT_MUTEX);

	mutex = (cw_nxoe_mutex_t *)a_nxo->o.nxoe;

	_cw_check_ptr(mutex);
	_cw_assert(mutex->nxoe.magic == _CW_NXOE_MAGIC);
	_cw_assert(mutex->nxoe.type == NXOT_MUTEX);

	mtx_unlock(&mutex->lock);
}
