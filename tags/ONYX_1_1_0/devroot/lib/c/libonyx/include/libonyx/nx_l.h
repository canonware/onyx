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

#ifndef _CW_USE_INLINES
cw_nxoe_t *nx_l_ref_iter(cw_nx_t *a_nx, cw_bool_t a_reset);
void	nx_l_thread_insert(cw_nx_t *a_nx, cw_nxo_t *a_thread);
void	nx_l_thread_remove(cw_nx_t *a_nx, cw_nxo_t *a_thread);
cw_mtx_t *nx_l_name_lock_get(cw_nx_t *a_nx);
cw_dch_t *nx_l_name_hash_get(cw_nx_t *a_nx);
cw_op_t *nx_l_thread_init(cw_nx_t *a_nx);
#endif

#if (defined(_CW_USE_INLINES) || defined(_NX_C_))
_CW_INLINE cw_nxoe_t *
nx_l_ref_iter(cw_nx_t *a_nx, cw_bool_t a_reset)
{
	cw_nxoe_t	*retval;

	_cw_check_ptr(a_nx);
	_cw_dassert(a_nx->magic == _CW_NX_MAGIC);

	if (a_nx->shutdown) {
		/*
		 * Return an empty root set so that the garbage collector will
		 * collect everything.
		 */
		retval = NULL;
		goto RETURN;
	}

	if (a_reset)
		a_nx->ref_iter = 0;

	for (retval = NULL; retval == NULL; a_nx->ref_iter++) {
		switch (a_nx->ref_iter) {
		case 0:
			retval = nxo_nxoe_get(nxa_gcdict_get(&a_nx->nxa));
		case 1:
			retval = nxo_nxoe_get(&a_nx->threadsdict);
			break;
		case 2:
			retval = nxo_nxoe_get(&a_nx->systemdict);
			break;
		case 3:
			retval = nxo_nxoe_get(&a_nx->globaldict);
			break;
		case 4:
			retval = nxo_nxoe_get(&a_nx->envdict);
			break;
		case 5:
			retval = nxo_nxoe_get(&a_nx->stdin_nxo);
			break;
		case 6:
			retval = nxo_nxoe_get(&a_nx->stdout_nxo);
			break;
		case 7:
			retval = nxo_nxoe_get(&a_nx->stderr_nxo);
			break;
		default:
			retval = NULL;
			goto RETURN;
		}
	}

	RETURN:
	return retval;
}

_CW_INLINE void
nx_l_thread_insert(cw_nx_t *a_nx, cw_nxo_t *a_thread)
{
	_cw_check_ptr(a_nx);
	_cw_dassert(a_nx->magic == _CW_NX_MAGIC);

	_cw_check_ptr(a_thread);
	_cw_assert(nxo_type_get(a_thread) == NXOT_THREAD);

	nxo_dict_def(&a_nx->threadsdict, a_nx, a_thread, a_thread);
}

_CW_INLINE void
nx_l_thread_remove(cw_nx_t *a_nx, cw_nxo_t *a_thread)
{
	_cw_check_ptr(a_nx);
	_cw_dassert(a_nx->magic == _CW_NX_MAGIC);

	_cw_check_ptr(a_thread);
	_cw_assert(nxo_type_get(a_thread) == NXOT_THREAD);

	nxo_dict_undef(&a_nx->threadsdict, a_nx, a_thread);
}

_CW_INLINE cw_mtx_t *
nx_l_name_lock_get(cw_nx_t *a_nx)
{
	_cw_check_ptr(a_nx);
	_cw_dassert(a_nx->magic == _CW_NX_MAGIC);

	return &a_nx->name_lock;
}

_CW_INLINE cw_dch_t *
nx_l_name_hash_get(cw_nx_t *a_nx)
{
	_cw_check_ptr(a_nx);
	_cw_dassert(a_nx->magic == _CW_NX_MAGIC);

	return &a_nx->name_hash;
}

_CW_INLINE cw_op_t *
nx_l_thread_init(cw_nx_t *a_nx)
{
	_cw_check_ptr(a_nx);
	_cw_dassert(a_nx->magic == _CW_NX_MAGIC);

	return a_nx->thread_init;
}
#endif	/* (defined(_CW_USE_INLINES) || defined(_NX_C_)) */
