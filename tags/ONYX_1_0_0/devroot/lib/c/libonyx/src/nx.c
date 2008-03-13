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

#define	_NX_C_

#include "../include/libonyx/libonyx.h"
#include "../include/libonyx/envdict_l.h"
#include "../include/libonyx/sprintdict_l.h"
#include "../include/libonyx/systemdict_l.h"
#include "../include/libonyx/nx_l.h"
#include "../include/libonyx/nxo_l.h"
#include "../include/libonyx/nxo_name_l.h"

/* Include generated code. */
#include "nx_nxcode.c"

cw_nx_t *
nx_new(cw_nx_t *a_nx, cw_op_t *a_thread_init, int a_argc, char **a_argv, char
    **a_envp)
{
	cw_nx_t			*retval;
	volatile cw_uint32_t	try_stage = 0;

	xep_begin();
	cw_nx_t	*v_retval;
	xep_try {
		if (a_nx != NULL) {
			retval = a_nx;
			memset(retval, 0, sizeof(cw_nx_t));
			retval->is_malloced = FALSE;
		} else {
			retval = (cw_nx_t *)_cw_malloc(sizeof(cw_nx_t));
			memset(retval, 0, sizeof(cw_nx_t));
			retval->is_malloced = TRUE;
		}
		v_retval = retval;
		try_stage = 1;

#ifdef _CW_DBG
		retval->magic = _CW_NX_MAGIC;
#endif

		/* Initialize the global name cache. */
		mtx_new(&retval->name_lock);
		dch_new(&retval->name_hash, NULL, _CW_LIBONYX_NAME_HASH,
		    _CW_LIBONYX_NAME_HASH / 4 * 3, _CW_LIBONYX_NAME_HASH / 4,
		    nxo_l_name_hash, nxo_l_name_key_comp);
		try_stage = 2;

		/* Initialize the GC (and gcdict by association). */
		nxa_new(&retval->nxa, retval);
		try_stage = 3;

		/* Initialize stdin. */
		nxo_file_new(&retval->stdin_nxo, retval, TRUE);
		nxo_file_fd_wrap(&retval->stdin_nxo, 0);
		nxo_file_buffer_size_set(&retval->stdin_nxo,
		    _CW_LIBONYX_FILE_BUFFER_SIZE);
		try_stage = 4;

		/* Initialize stdout. */
		nxo_file_new(&retval->stdout_nxo, retval, TRUE);
		nxo_file_fd_wrap(&retval->stdout_nxo, 1);
		nxo_file_buffer_size_set(&retval->stdout_nxo,
		    _CW_LIBONYX_FILE_BUFFER_SIZE);
		try_stage = 5;

		/* Initialize stderr. */
		nxo_file_new(&retval->stderr_nxo, retval, TRUE);
		nxo_file_fd_wrap(&retval->stderr_nxo, 2);
		try_stage = 6;

		/* Initialize globaldict. */
		nxo_dict_new(&retval->globaldict, retval, TRUE,
		    _CW_LIBONYX_GLOBALDICT_HASH);
		try_stage = 7;

		/* Initialize sprintdict. */
		sprintdict_l_populate(&retval->sprintdict, retval);
		try_stage = 8;

		/* Initialize envdict. */
		envdict_l_populate(&retval->envdict, retval, a_envp);
		try_stage = 9;

		/* Initialize systemdict. */
		systemdict_l_populate(&retval->systemdict, retval, a_argc,
		    a_argv);
		try_stage = 10;

		/* Initialize threadsdict. */
		nxo_dict_new(&retval->threadsdict, retval, TRUE,
		    _CW_LIBONYX_THREADSDICT_HASH);
		try_stage = 11;

		/* Now that we have an initial thread, activate the GC. */
		nxa_active_set(&retval->nxa, TRUE);

		/* Do soft operator initialization. */
		nx_p_nxcode(retval);

		/*
		 * Set the thread initialization hook pointer.  It's important
		 * to set this after doing soft operator initialization, so that
		 * the hook doesn't get called above while ininitializing
		 * globally visible soft operators.
		 */
		retval->thread_init = a_thread_init;
	}
	xep_catch (_CW_STASHX_OOM) {
		retval = (cw_nx_t *)v_retval;
		switch (try_stage) {
		case 11:
		case 10:
		case 9:
		case 8:
		case 7:
		case 6:
		case 5:
		case 4:
		case 3:
			nxa_delete(&retval->nxa);
		case 2:
			dch_delete(&retval->name_hash);
			mtx_delete(&retval->name_lock);
		case 1:
#ifdef _CW_DBG
			memset(a_nx, 0x5a, sizeof(cw_nx_t));
#endif
			if (retval->is_malloced)
				_cw_free(retval);
			break;
		default:
			_cw_not_reached();
		}
	}
	xep_end();

	return retval;
}

void
nx_delete(cw_nx_t *a_nx)
{
	_cw_check_ptr(a_nx);
	_cw_dassert(a_nx->magic == _CW_NX_MAGIC);

	a_nx->shutdown = TRUE;
	nxa_delete(&a_nx->nxa);
	dch_delete(&a_nx->name_hash);
	mtx_delete(&a_nx->name_lock);

	if (a_nx->is_malloced)
		_cw_free(a_nx);
#ifdef _CW_DBG
	else
		memset(a_nx, 0x5a, sizeof(cw_nx_t));
#endif
}
