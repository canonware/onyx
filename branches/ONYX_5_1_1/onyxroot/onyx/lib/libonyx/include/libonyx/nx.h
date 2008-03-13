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

struct cw_nx_s
{
#ifdef CW_DBG
    uint32_t magic;
#define CW_NX_MAGIC 0xae9678fd
#endif

    bool is_malloced;

    /* Linkage for nxa's list of nx's. */
    ql_elm(cw_nx_t) link;

    /* Initial maximum estack depth for new threads. */
    cw_nxoi_t maxestack;

    /* Initial setting for tail call optimization. */
    bool tailopt;

    /* Dictionaries. */
    cw_nxo_t threadsdict;
    cw_nxo_t systemdict;
    cw_nxo_t globaldict;

    /* Files. */
    cw_nxo_t stdin_nxo;
    cw_nxo_t stdout_nxo;
    cw_nxo_t stderr_nxo;

    /* Thread initialization hook. */
    cw_op_t *thread_init;

    /* Thread start hook. */
    cw_thread_start_t *thread_start;
};

/* nx. */
cw_nx_t *
nx_new(cw_nx_t *a_nx, cw_op_t *a_thread_init,
       cw_thread_start_t *a_thread_start);

void
nx_delete(cw_nx_t *a_nx);

void
nx_maxestack_set(cw_nx_t *a_nx, cw_nxoi_t a_maxestack);

void
nx_tailopt_set(cw_nx_t *a_nx, bool a_tailopt);

void
nx_stdin_set(cw_nx_t *a_nx, cw_nxo_t *a_stdin);

void
nx_stdout_set(cw_nx_t *a_nx, cw_nxo_t *a_stdout);

void
nx_stderr_set(cw_nx_t *a_nx, cw_nxo_t *a_stderr);

#ifndef CW_USE_INLINES
cw_nxoi_t
nx_maxestack_get(cw_nx_t *a_nx);

bool
nx_tailopt_get(cw_nx_t *a_nx);

cw_nxo_t *
nx_threadsdict_get(cw_nx_t *a_nx);

cw_nxo_t *
nx_systemdict_get(cw_nx_t *a_nx);

cw_nxo_t *
nx_globaldict_get(cw_nx_t *a_nx);

cw_nxo_t *
nx_stdin_get(cw_nx_t *a_nx);

cw_nxo_t *
nx_stdout_get(cw_nx_t *a_nx);

cw_nxo_t *
nx_stderr_get(cw_nx_t *a_nx);
#endif

#if (defined(CW_USE_INLINES) || defined(CW_NX_C_))
CW_INLINE cw_nxoi_t
nx_maxestack_get(cw_nx_t *a_nx)
{
    cw_check_ptr(a_nx);
    cw_dassert(a_nx->magic == CW_NX_MAGIC);

    return a_nx->maxestack;
}

CW_INLINE bool
nx_tailopt_get(cw_nx_t *a_nx)
{
    cw_check_ptr(a_nx);
    cw_dassert(a_nx->magic == CW_NX_MAGIC);

    return a_nx->tailopt;
}

CW_INLINE cw_nxo_t *
nx_threadsdict_get(cw_nx_t *a_nx)
{
    cw_check_ptr(a_nx);
    cw_dassert(a_nx->magic == CW_NX_MAGIC);

    return &a_nx->threadsdict;
}

CW_INLINE cw_nxo_t *
nx_systemdict_get(cw_nx_t *a_nx)
{
    cw_check_ptr(a_nx);
    cw_dassert(a_nx->magic == CW_NX_MAGIC);

    return &a_nx->systemdict;
}

CW_INLINE cw_nxo_t *
nx_globaldict_get(cw_nx_t *a_nx)
{
    cw_check_ptr(a_nx);
    cw_dassert(a_nx->magic == CW_NX_MAGIC);

    return &a_nx->globaldict;
}

CW_INLINE cw_nxo_t *
nx_stdin_get(cw_nx_t *a_nx)
{
    cw_check_ptr(a_nx);
    cw_dassert(a_nx->magic == CW_NX_MAGIC);

    return &a_nx->stdin_nxo;
}

CW_INLINE cw_nxo_t *
nx_stdout_get(cw_nx_t *a_nx)
{
    cw_check_ptr(a_nx);
    cw_dassert(a_nx->magic == CW_NX_MAGIC);

    return &a_nx->stdout_nxo;
}

CW_INLINE cw_nxo_t *
nx_stderr_get(cw_nx_t *a_nx)
{
    cw_check_ptr(a_nx);
    cw_dassert(a_nx->magic == CW_NX_MAGIC);

    return &a_nx->stderr_nxo;
}
#endif /* (defined(CW_USE_INLINES) || defined(CW_NX_C_)) */
