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
#include "../include/libstil/stilo_l.h"

#ifdef _LIBSTIL_DBG
#define _CW_STIL_MAGIC 0xae9678fd
#endif

/*
 * Size and fullness control of initial name cache hash table.  We know for sure
 * that there will be about 175 names referenced by systemdict, threaddict,
 * errordict, and currenterror to begin with.
 */
#define _CW_STIL_NAME_BASE_TABLE	512
#define _CW_STIL_NAME_BASE_GROW		400
#define _CW_STIL_NAME_BASE_SHRINK	128

/*
 * Size and fullness control of initial root set for global VM.  Global VM is
 * empty to begin with.
 */
#define _CW_STIL_ROOTS_BASE_TABLE	 32
#define _CW_STIL_ROOTS_BASE_GROW	 24
#define _CW_STIL_ROOTS_BASE_SHRINK	  8

/*
 * Initial size of globaldict.  This is a bit arbitrary, and some applications
 * could benefit from making it larger or smaller.
 */
#define	_CW_STIL_GLOBALDICT_SIZE	 64

/*
 * Size of buffers for stdin and stdout.  stderr isn't buffered.
 */
#define	_CW_STIL_STDIN_BUFFER_SIZE	512
#define	_CW_STIL_STDOUT_BUFFER_SIZE	512

cw_stil_t *
stil_new(cw_stil_t *a_stil, cw_stilo_file_read_t *a_stdin,
    cw_stilo_file_write_t *a_stdout, cw_stilo_file_write_t *a_stderr, void
    *a_arg)
{
	cw_stil_t		*retval;
	cw_stilt_t		stilt;
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
		try_stage = 1;

		v_retval = retval;
		stila_new(&retval->stila, retval);
		try_stage = 2;

		ql_new(&retval->stilt_head);
		mtx_new(&retval->name_lock);
		dch_new(&retval->name_hash, NULL, _CW_STIL_NAME_BASE_TABLE,
		    _CW_STIL_NAME_BASE_GROW, _CW_STIL_NAME_BASE_SHRINK,
		    stilo_l_name_hash, stilo_l_name_key_comp);
		try_stage = 3;

		/*
		 * Create a temporary thread in order to be able to initialize
		 * systemdict, stdin, stdout, and stderr, and destroy the thread
		 * as soon as we're done.
		 */

		/* XXX OOM in dictionary initialization isn't handled. */

		/* Initialize systemdict, since stilt_new() will access it. */
		stilo_no_new(&retval->systemdict);
		stilt_new(&stilt, retval);
		try_stage = 4;

		stilt_setglobal(&stilt, TRUE);

		/* Initialize globaldict. */
		stilo_dict_new(&retval->globaldict, retval,
		    _CW_STIL_GLOBALDICT_SIZE);

		/* Initialize stdin. */
		stilo_file_new(&retval->stdin_stilo, retval);
		if (a_stdin == NULL)
			stilo_file_fd_wrap(&retval->stdin_stilo, 0);
		else {
			stilo_file_interactive(&retval->stdin_stilo, a_stdin,
			    NULL, a_arg);
		}
		stilo_file_buffer_size_set(&retval->stdin_stilo,
		    _CW_STIL_STDIN_BUFFER_SIZE);

		/* Initialize stdout. */
		stilo_file_new(&retval->stdout_stilo, retval);
		if (a_stdout == NULL)
			stilo_file_fd_wrap(&retval->stdout_stilo, 1);
		else {
			stilo_file_interactive(&retval->stdout_stilo, NULL,
			    a_stdout, a_arg);
		}
		stilo_file_buffer_size_set(&retval->stdout_stilo,
		    _CW_STIL_STDOUT_BUFFER_SIZE);

		/* Initialize stderr. */
		stilo_file_new(&retval->stderr_stilo, retval);
		if (a_stderr == NULL)
			stilo_file_fd_wrap(&retval->stderr_stilo, 2);
		else {
			stilo_file_interactive(&retval->stderr_stilo, NULL,
			    a_stderr, a_arg);
		}

		/* Finish systemdict initialization. */
		systemdict_populate(&retval->systemdict, &stilt);

		stilt_delete(&stilt);
	}
	xep_catch (_CW_XEPV_OOM) {
		retval = (cw_stil_t *)v_retval;
		switch (try_stage) {
		case 4:
			stilt_delete(&stilt);
		case 3:
			dch_delete(&retval->name_hash);
			mtx_delete(&retval->name_lock);
		case 2:
			stila_delete(&retval->stila);
		case 1:
			if (retval->is_malloced)
				_cw_free(retval);
			break;
		default:
			_cw_not_reached();
		}
	}
	xep_end();

	mtx_new(&retval->lock);

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
	
	/* Flush stdout. */
	/* XXX Check return? */
	stilo_file_buffer_flush(&a_stil->stdout_stilo);

	stila_delete(&a_stil->stila);
	dch_delete(&a_stil->name_hash);
	mtx_delete(&a_stil->name_lock);
	mtx_delete(&a_stil->lock);

	if (a_stil->is_malloced)
		_cw_free(a_stil);
#ifdef _LIBSTIL_DBG
	else
		memset(a_stil, 0x5a, sizeof(cw_stil_t));
#endif
}

cw_stilt_t *
stil_l_ref_iter(cw_stil_t *a_stil, cw_bool_t a_reset)
{
	cw_stilt_t	*retval;

	_cw_check_ptr(a_stil);
	_cw_assert(a_stil->magic == _CW_STIL_MAGIC);

	if (a_reset)
		ql_first(&a_stil->ref_iter) = ql_first(&a_stil->stilt_head);

	/*
	 * Iterate through the stilt's.
	 */
	retval = ql_first(&a_stil->ref_iter);

	if (retval != NULL) {
		ql_first(&a_stil->ref_iter) = ql_next(&a_stil->stilt_head,
		    retval, link);
	}

	return retval;
}
