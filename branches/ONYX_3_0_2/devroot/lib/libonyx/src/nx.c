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
#ifdef CW_POSIX
#include "../include/libonyx/envdict_l.h"
#endif
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
nx_new(cw_nx_t *a_nx, cw_op_t *a_thread_init, int a_argc, char **a_argv,
       char **a_envp)
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
	    retval = (cw_nx_t *) cw_malloc(sizeof(cw_nx_t));
	    memset(retval, 0, sizeof(cw_nx_t));
	    retval->is_malloced = TRUE;
	}
	v_retval = retval;
	try_stage = 1;

#ifdef CW_DBG
	retval->magic = CW_NX_MAGIC;
#endif

	/* Initialize the GC. */
	nxa_l_new(&retval->nxa, retval);
	try_stage = 2;

	/* Initialize the global name cache. */
#ifdef CW_THREADS
	mtx_new(&retval->name_lock);
#endif
	dch_new(&retval->name_hash, (cw_opaque_alloc_t *) nxa_malloc_e,
		(cw_opaque_dealloc_t *) nxa_free_e, &retval->nxa,
		CW_LIBONYX_NAME_HASH, CW_LIBONYX_NAME_HASH / 4 * 3,
		CW_LIBONYX_NAME_HASH / 4, nxo_l_name_hash,
		nxo_l_name_key_comp);
	try_stage = 3;

	/* Initialize gcdict. */
	gcdict_l_populate(nxa_gcdict_get(&retval->nxa), &retval->nxa);
	try_stage = 4;

	/* Initialize stdin. */
	nxo_file_new(&retval->stdin_nxo, retval, TRUE);
#ifdef CW_POSIX_FILE
	nxo_file_fd_wrap(&retval->stdin_nxo, 0);
#endif
	nxo_file_buffer_size_set(&retval->stdin_nxo,
				 CW_LIBONYX_FILE_BUFFER_SIZE);
	try_stage = 5;

	/* Initialize stdout. */
	nxo_file_new(&retval->stdout_nxo, retval, TRUE);
#ifdef CW_POSIX_FILE
	nxo_file_fd_wrap(&retval->stdout_nxo, 1);
#endif
	nxo_file_buffer_size_set(&retval->stdout_nxo,
				 CW_LIBONYX_FILE_BUFFER_SIZE);
	try_stage = 6;

	/* Initialize stderr. */
	nxo_file_new(&retval->stderr_nxo, retval, TRUE);
#ifdef CW_POSIX_FILE
	nxo_file_fd_wrap(&retval->stderr_nxo, 2);
#endif
	try_stage = 7;

	/* Initialize globaldict. */
	nxo_dict_new(&retval->globaldict, retval, TRUE,
		     CW_LIBONYX_GLOBALDICT_HASH);
	try_stage = 8;

#ifdef CW_POSIX
	/* Initialize envdict. */
	envdict_l_populate(&retval->envdict, retval, a_envp);
	try_stage = 9;
#endif

	/* Initialize systemdict. */
	systemdict_l_populate(&retval->systemdict, retval, a_argc,
			      a_argv);
	try_stage = 10;

	/* Initialize threadsdict. */
	nxo_dict_new(&retval->threadsdict, retval, TRUE,
		     CW_LIBONYX_THREADSDICT_HASH);
	try_stage = 11;

	/* Now that we have an initial thread, activate the GC. */
	nxa_active_set(&retval->nxa, TRUE);

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
	    case 11:
	    case 10:
#ifdef CW_POSIX
	    case 9:
#endif
	    case 8:
	    case 7:
	    case 6:
	    case 5:
	    case 4:
	    case 3:
	    {
		nxa_l_shutdown(&retval->nxa);
		dch_delete(&retval->name_hash);
#ifdef CW_THREADS
		mtx_delete(&retval->name_lock);
#endif
	    }
	    case 2:
	    {
		nxa_l_delete(&retval->nxa);
	    }
	    case 1:
	    {
#ifdef CW_DBG
		memset(a_nx, 0x5a, sizeof(cw_nx_t));
#endif
		if (retval->is_malloced)
		cw_free(retval);
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

    a_nx->shutdown = TRUE;
    /* All objects must be destroyed before name_hash is deleted, in order to
     * avoid a circular shutdown dependency. */
    nxa_l_shutdown(&a_nx->nxa);
    dch_delete(&a_nx->name_hash);
#ifdef CW_THREADS
    mtx_delete(&a_nx->name_lock);
#endif
    nxa_l_delete(&a_nx->nxa);

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
