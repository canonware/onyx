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

#define	CW_NX_C_

#include "../include/libonyx/libonyx.h"
#include "../include/libonyx/gcdict_l.h"
#include "../include/libonyx/systemdict_l.h"
#include "../include/libonyx/nx_l.h"
#include "../include/libonyx/nxa_l.h"
#include "../include/libonyx/nxo_l.h"
#include "../include/libonyx/nxo_name_l.h"

/* Prototype for automatically generated function. */
void
nx_p_nxcode(cw_nx_t *a_nx);

cw_nx_t *
nx_new(cw_nx_t *a_nx, cw_op_t *a_thread_init)
{
    cw_nx_t *retval;
    volatile cw_uint32_t try_stage = 0;

    xep_begin();
    volatile cw_nx_t *v_retval;
    xep_try
    {
	if (a_nx != NULL)
	{
	    retval = a_nx;
	    memset(retval, 0, sizeof(cw_nx_t));
	    retval->is_malloced = FALSE;
	}
	else
	{
	    retval = (cw_nx_t *) cw_calloc(1, sizeof(cw_nx_t));
	    retval->is_malloced = TRUE;
	}
	v_retval = retval;
	try_stage = 1;

#ifdef CW_DBG
	retval->magic = CW_NX_MAGIC;
#endif

	/* Initialize the internals such that if a collection happens before
	 * we're done, reference iteration will work correctly. */
	nxo_no_new(&retval->gcdict);
	nxo_no_new(&retval->threadsdict);
	nxo_no_new(&retval->systemdict);
	nxo_no_new(&retval->globaldict);
	nxo_no_new(&retval->stdin_nxo);
	nxo_no_new(&retval->stdout_nxo);
	nxo_no_new(&retval->stderr_nxo);

	/* Insert this nx into nxa's list of nx's in the root set. */
	ql_elm_new(retval, link);
	nxa_l_nx_insert(retval);
	try_stage = 2;

	/* Initialize globaldict. */
	nxo_dict_new(&retval->globaldict, TRUE, CW_LIBONYX_GLOBALDICT_HASH);

	/* Use stdin_nxo and stdout_nxo as temporaries for the dictionary
	 * population functions.  This is the only place where such temporaries
	 * are needed, so embedding additional fields into cw_nx_t (and
	 * reporting them during reference iteration) just for this purpose
	 * would be wasteful. */

	/* Initialize threadsdict. */
	nxo_dict_new(&retval->threadsdict, TRUE, CW_LIBONYX_THREADSDICT_HASH);

	/* Initialize gcdict. */
	gcdict_l_populate(&retval->gcdict, &retval->stdin_nxo,
			  &retval->stdout_nxo);

	/* Initialize systemdict.  This must happen after the other dict
	 * initializations, since references to them are inserted into
	 * systemdict. */
	systemdict_l_populate(&retval->systemdict, &retval->stdin_nxo,
			      &retval->stdout_nxo, retval);

	/* Initialize stdin. */
	nxo_file_new(&retval->stdin_nxo, TRUE);
#ifdef CW_POSIX_FILE
	nxo_file_fd_wrap(&retval->stdin_nxo, 0, FALSE);
#endif
	nxo_file_buffer_size_set(&retval->stdin_nxo,
				 CW_LIBONYX_FILE_BUFFER_SIZE);

	/* Initialize stdout. */
	nxo_file_new(&retval->stdout_nxo, TRUE);
#ifdef CW_POSIX_FILE
	nxo_file_fd_wrap(&retval->stdout_nxo, 1, FALSE);
#endif
	nxo_file_buffer_size_set(&retval->stdout_nxo,
				 CW_LIBONYX_FILE_BUFFER_SIZE);

	/* Initialize stderr. */
	nxo_file_new(&retval->stderr_nxo, TRUE);
#ifdef CW_POSIX_FILE
	nxo_file_fd_wrap(&retval->stderr_nxo, 2, FALSE);
#endif

	/* Do soft operator initialization. */
	nx_p_nxcode(retval);

	/* Set the thread initialization hook pointer.  It's important to set
	 * this after doing soft operator initialization, so that the hook
	 * doesn't get called above while ininitializing globally visible soft
	 * operators. */
	retval->thread_init = a_thread_init;
    }
    xep_catch (CW_ONYXX_OOM)
    {
	retval = (cw_nx_t *) v_retval;
	switch (try_stage)
	{
	    case 2:
	    {
		nxa_l_nx_remove(retval);
	    }
	    case 1:
	    {
#ifdef CW_DBG
		memset(a_nx, 0x5a, sizeof(cw_nx_t));
#endif
		if (retval->is_malloced)
		{
		    cw_free(retval);
		}
		break;
	    }
	    default:
	    {
		cw_not_reached();
	    }
	}
    }
    xep_end();

    return retval;
}

void
nx_delete(cw_nx_t *a_nx)
{
    cw_check_ptr(a_nx);
    cw_dassert(a_nx->magic == CW_NX_MAGIC);

    nxa_l_nx_remove(a_nx);

    if (a_nx->is_malloced)
    {
	cw_free(a_nx);
    }
#ifdef CW_DBG
    else
    {
	memset(a_nx, 0x5a, sizeof(cw_nx_t));
    }
#endif
}

void
nx_stdin_set(cw_nx_t *a_nx, cw_nxo_t *a_stdin)
{
    cw_check_ptr(a_nx);
    cw_dassert(a_nx->magic == CW_NX_MAGIC);

    nxo_dup(&a_nx->stdin_nxo, a_stdin);
}

void
nx_stdout_set(cw_nx_t *a_nx, cw_nxo_t *a_stdout)
{
    cw_check_ptr(a_nx);
    cw_dassert(a_nx->magic == CW_NX_MAGIC);

    nxo_dup(&a_nx->stdout_nxo, a_stdout);
}

void
nx_stderr_set(cw_nx_t *a_nx, cw_nxo_t *a_stderr)
{
    cw_check_ptr(a_nx);
    cw_dassert(a_nx->magic == CW_NX_MAGIC);

    nxo_dup(&a_nx->stderr_nxo, a_stderr);
}
