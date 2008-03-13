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

#include <dlfcn.h>

#include "../include/libonyx/libonyx.h"

typedef struct cw_nxm_s cw_nxm_t;

struct cw_nxm_s
{
#ifdef CW_DBG
#define CW_NXM_MAGIC 0x397c8a8f
    uint32_t magic;
#endif
    uint32_t iter;
    void (*pre_unload_hook)(void);
    void *dlhandle;
};

static cw_nxoe_t *
nxm_p_ref_iter(void *a_data, bool a_reset)
{
    return NULL;
}

static bool
nxm_p_delete(void *a_data, uint32_t a_iter)
{
    bool retval;
    cw_nxm_t *nxm = (cw_nxm_t *) a_data;
    
    if (a_iter != nxm->iter)
    {
	retval = true;
	goto RETURN;
    }

    if (nxm->pre_unload_hook != NULL)
    {
	nxm->pre_unload_hook();
    }

    dlclose(nxm->dlhandle);
    nxa_free(a_data, sizeof(cw_nxm_t));

    retval = false;
    RETURN:
    return retval;
}

cw_nxn_t
nxm_new(cw_nxo_t *a_nxo, cw_nxo_t *a_path, cw_nxo_t *a_sym)
{
    cw_nxn_t retval;
    cw_nxm_t *nxm;
    void *handle, *symbol;
    uint32_t pathlen, symlen;
    char *path, *sym;

    cw_check_ptr(a_path);
    cw_assert(nxo_type_get(a_path) == NXOT_STRING);
    cw_check_ptr(a_sym);
    cw_assert(nxo_type_get(a_sym) == NXOT_STRING);

    /* Try to dlopen(). */
    pathlen = nxo_string_len_get(a_path);
    path = cw_malloc(pathlen + 1);
    memcpy(path, nxo_string_get(a_path), pathlen);
    path[pathlen] = '\0';
    handle = dlopen(path, RTLD_LAZY);
    cw_free(path);
    if (handle == NULL)
    {
#ifdef CW_DBG
	fprintf(stderr, "dlopen() error: %s\n", dlerror());
#endif
	retval = NXN_invalidfileaccess;
	goto RETURN;
    }

    /* Try to look up symbol. */
    symlen = nxo_string_len_get(a_sym);
    sym = cw_malloc(symlen + 1);
    memcpy(sym, nxo_string_get(a_sym), symlen);
    sym[symlen] = '\0';
    symbol = dlsym(handle, sym);
    cw_free(sym);
    if (symbol == NULL)
    {
#ifdef CW_DBG
	fprintf(stderr, "dlsym() error: %s\n", dlerror());
#endif
	dlclose(handle);
	retval = NXN_undefined;
	goto RETURN;
    }

    /* Create a handle whose data pointer is a (cw_nxm_t), and whose evaluation
     * function is the symbol we just looked up. */
    nxm = (cw_nxm_t *) nxa_malloc(sizeof(cw_nxm_t));
    /* Set the default iteration for module destruction to 1.  This number can
     * be overridden on a per-module basis in the module initialization code. */
    nxm->iter = 1;
    /* There is no pre-unload hook called by default. */
    nxm->pre_unload_hook = NULL;
    nxm->dlhandle = handle;

    nxo_handle_new(a_nxo, nxm, symbol, nxm_p_ref_iter, nxm_p_delete);
    nxo_dup(nxo_handle_tag_get(a_nxo), a_sym);
    nxo_attr_set(a_nxo, NXOA_EXECUTABLE);

    retval = NXN_ZERO;
    RETURN:
    return retval;
}

uint32_t
nxm_iter_get(cw_nxo_t *a_nxo)
{
    cw_nxm_t *nxm;

    cw_check_ptr(a_nxo);
    cw_dassert(a_nxo->magic == CW_NXO_MAGIC);
    cw_assert(nxo_type_get(a_nxo) == NXOT_HANDLE);

    nxm = nxo_handle_opaque_get(a_nxo);
    cw_dassert(nxm->magic == CW_NXM_MAGIC);

    return nxm->iter;
}

void
nxm_iter_set(cw_nxo_t *a_nxo, uint32_t a_iter)
{
    cw_nxm_t *nxm;

    cw_check_ptr(a_nxo);
    cw_dassert(a_nxo->magic == CW_NXO_MAGIC);
    cw_assert(nxo_type_get(a_nxo) == NXOT_HANDLE);

    nxm = nxo_handle_opaque_get(a_nxo);
    cw_dassert(nxm->magic == CW_NXM_MAGIC);

    nxm->iter = a_iter;
}

void *
nxm_pre_unload_hook_get(cw_nxo_t *a_nxo)
{
    cw_nxm_t *nxm;

    cw_check_ptr(a_nxo);
    cw_dassert(a_nxo->magic == CW_NXO_MAGIC);
    cw_assert(nxo_type_get(a_nxo) == NXOT_HANDLE);

    nxm = nxo_handle_opaque_get(a_nxo);
    cw_dassert(nxm->magic == CW_NXM_MAGIC);

    return nxm->pre_unload_hook;
}

void
nxm_pre_unload_hook_set(cw_nxo_t *a_nxo, void (*a_pre_unload_hook)(void))
{
    cw_nxm_t *nxm;

    cw_check_ptr(a_nxo);
    cw_dassert(a_nxo->magic == CW_NXO_MAGIC);
    cw_assert(nxo_type_get(a_nxo) == NXOT_HANDLE);

    nxm = nxo_handle_opaque_get(a_nxo);
    cw_dassert(nxm->magic == CW_NXM_MAGIC);

    nxm->pre_unload_hook = a_pre_unload_hook;
}
