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
#include "../include/libstil/envdict_l.h"
#include "../include/libstil/systemdict_l.h"
#include "../include/libstil/stilo_l.h"
#include "../include/libstil/stilo_name_l.h"

#ifdef _LIBSTIL_DBG
#define _CW_STIL_MAGIC 0xae9678fd
#endif

static void stil_p_soft_init(cw_stil_t *a_stil);

cw_stil_t *
stil_new(cw_stil_t *a_stil, int a_argc, char **a_argv, char **a_envp,
    cw_stilo_file_read_t *a_stdin, cw_stilo_file_write_t *a_stdout,
    cw_stilo_file_write_t *a_stderr, void *a_arg, cw_op_t *a_thread_init)
{
	cw_stil_t		*retval;
	volatile cw_uint32_t	try_stage = 0;

	xep_begin();
	cw_stil_t	*v_retval;
	xep_try {
		if (a_stil != NULL) {
			retval = a_stil;
			memset(retval, 0, sizeof(cw_stil_t));
			retval->is_malloced = FALSE;
		} else {
			retval = (cw_stil_t *)_cw_malloc(sizeof(cw_stil_t));
			memset(retval, 0, sizeof(cw_stil_t));
			retval->is_malloced = TRUE;
		}
		v_retval = retval;
		try_stage = 1;

		/* Initialize the global name cache. */
		mtx_new(&retval->name_lock);
		dch_new(&retval->name_hash, NULL, _LIBSTIL_NAME_HASH,
		    _LIBSTIL_NAME_HASH / 4 * 3, _LIBSTIL_NAME_HASH / 4,
		    stilo_l_name_hash, stilo_l_name_key_comp);
		try_stage = 2;

		/* Initialize the GC (and gcdict by association). */
		stila_new(&retval->stila, retval);
		try_stage = 3;

		/* Initialize stdin. */
		stilo_file_new(&retval->stdin_stilo, retval, TRUE);
		if (a_stdin == NULL)
			stilo_file_fd_wrap(&retval->stdin_stilo, 0);
		else {
			stilo_file_interactive(&retval->stdin_stilo, a_stdin,
			    NULL, a_arg);
		}
		stilo_file_buffer_size_set(&retval->stdin_stilo,
		    _LIBSTIL_FILE_BUFFER_SIZE);
		try_stage = 4;

		/* Initialize stdout. */
		stilo_file_new(&retval->stdout_stilo, retval, TRUE);
		if (a_stdout == NULL)
			stilo_file_fd_wrap(&retval->stdout_stilo, 1);
		else {
			stilo_file_interactive(&retval->stdout_stilo, NULL,
			    a_stdout, a_arg);
		}
		stilo_file_buffer_size_set(&retval->stdout_stilo,
		    _LIBSTIL_FILE_BUFFER_SIZE);
		try_stage = 5;

		/* Initialize stderr. */
		stilo_file_new(&retval->stderr_stilo, retval, TRUE);
		if (a_stderr == NULL)
			stilo_file_fd_wrap(&retval->stderr_stilo, 2);
		else {
			stilo_file_interactive(&retval->stderr_stilo, NULL,
			    a_stderr, a_arg);
		}
		try_stage = 6;

		/* Initialize globaldict. */
		stilo_dict_new(&retval->globaldict, retval, TRUE,
		    _LIBSTIL_GLOBALDICT_HASH);
		try_stage = 7;

		/* Initialize envdict. */
		envdict_l_populate(&retval->envdict, retval, a_envp);
		try_stage = 8;

		/* Initialize systemdict. */
		systemdict_l_populate(&retval->systemdict, retval, a_argc,
		    a_argv);
		try_stage = 9;

		/* Initialize threadsdict. */
		stilo_dict_new(&retval->threadsdict, retval, TRUE,
		    _LIBSTIL_THREADSDICT_HASH);
		try_stage = 10;

		/* Set the thread initialization hook pointer. */
		retval->thread_init = a_thread_init;

		/* Now that we have an initial thread, activate the GC. */
		stila_active_set(&retval->stila, TRUE);

		/* Do soft operator initialization. */
		stil_p_soft_init(retval);
	}
	xep_catch (_CW_STASHX_OOM) {
		retval = (cw_stil_t *)v_retval;
		switch (try_stage) {
		case 10:
		case 9:
		case 8:
		case 7:
		case 6:
		case 5:
		case 4:
		case 3:
			stila_delete(&retval->stila);
		case 2:
			dch_delete(&retval->name_hash);
			mtx_delete(&retval->name_lock);
		case 1:
			if (retval->is_malloced)
				_cw_free(retval);
			break;
		default:
			_cw_not_reached();
		}
	}
	xep_end();

#ifdef _LIBSTIL_DBG
	retval->magic = _CW_STIL_MAGIC;
#endif

	return retval;
}

void
stil_delete(cw_stil_t *a_stil)
{
	_cw_check_ptr(a_stil);
	_cw_assert(a_stil->magic == _CW_STIL_MAGIC);
	
	/*
	 * Flush stdout.  There's nowhere to report an error, so don't even
	 * check whether an error occurs.
	 */
	stilo_file_buffer_flush(&a_stil->stdout_stilo);

	stila_delete(&a_stil->stila);
	dch_delete(&a_stil->name_hash);
	mtx_delete(&a_stil->name_lock);

	if (a_stil->is_malloced)
		_cw_free(a_stil);
#ifdef _LIBSTIL_DBG
	else
		memset(a_stil, 0x5a, sizeof(cw_stil_t));
#endif
}

cw_stiloe_t *
stil_l_ref_iter(cw_stil_t *a_stil, cw_bool_t a_reset)
{
	cw_stiloe_t	*retval;

	_cw_check_ptr(a_stil);
	_cw_assert(a_stil->magic == _CW_STIL_MAGIC);

	if (a_reset)
		a_stil->ref_iter = 0;

	for (retval = NULL; retval == NULL; a_stil->ref_iter++) {
		switch (a_stil->ref_iter) {
		case 0:
			retval = stilo_stiloe_get(&a_stil->threadsdict);
			break;
		case 1:
			retval = stilo_stiloe_get(&a_stil->systemdict);
			break;
		case 2:
			retval = stilo_stiloe_get(&a_stil->globaldict);
			break;
		case 3:
			retval = stilo_stiloe_get(&a_stil->envdict);
			break;
		case 4:
			retval = stilo_stiloe_get(&a_stil->stdin_stilo);
			break;
		case 5:
			retval = stilo_stiloe_get(&a_stil->stdout_stilo);
			break;
		case 6:
			retval = stilo_stiloe_get(&a_stil->stderr_stilo);
			break;
		default:
			retval = NULL;
			goto RETURN;
		}
	}

	RETURN:
	return retval;
}

/* Include generated code. */
#include "softop.c"
