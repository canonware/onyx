/* -*- mode: c ; c-file-style: "canonware-c-style" -*-
 ******************************************************************************
 *
 * <Copyright = jasone>
 * <License>
 *
 ******************************************************************************
 *
 * Version: Slate <Version = slate>
 *
 ******************************************************************************/

#include "../include/modslate.h"

struct cw_funnel
{
    /* Mutex used for entering/leaving funnel. */
    cw_mtx_t mtx;

    /* Reference to =funnel=, prevents premature module unload. */
    cw_nxo_t hook;
};

static const struct cw_modslate_entry modslate_funnel_hooks[] =
{
    /* funnel. */
    MODSLATE_ENTRY(funnel),
    {"funnel?", modslate_funnel_p},
    MODSLATE_ENTRY(funnel_enter),
    MODSLATE_ENTRY(funnel_leave)
};

static cw_nxoe_t *
funnel_p_ref_iter(void *a_data, cw_bool_t a_reset);
static cw_bool_t
funnel_p_delete(void *a_data, cw_nx_t *a_nx, cw_uint32_t a_iter);

void
modslate_funnel_init(cw_nxo_t *a_thread)
{
    modslate_hooks_init(a_thread, modslate_funnel_hooks,
			(sizeof(modslate_funnel_hooks)
			 / sizeof(struct cw_modslate_entry)));
}

static cw_nxoe_t *
funnel_p_ref_iter(void *a_data, cw_bool_t a_reset)
{
    cw_nxoe_t *retval;
    struct cw_funnel *funnel = (struct cw_funnel *) a_data;

    if (a_reset)
    {
	retval = nxo_nxoe_get(&funnel->hook);
    }
    else
    {
	retval = NULL;
    }

    return retval;
}

static cw_bool_t
funnel_p_delete(void *a_data, cw_nx_t *a_nx, cw_uint32_t a_iter)
{
    cw_bool_t retval;
    struct cw_funnel *funnel = (struct cw_funnel *) a_data;

    /* Don't delete until the appropriate GC sweep iteration, so that objects
     * that hold a funnel during deletion can be deleted first. */
    if (a_iter != MODSLATE_GC_ITER_FUNNEL)
    {
	retval = TRUE;
	goto RETURN;
    }

    mtx_delete(&funnel->mtx);
    nxa_free(nx_nxa_get(a_nx), funnel, sizeof(struct cw_funnel));

    retval = FALSE;
    RETURN:
    return retval;
}

/* modslate_funnel_c_{enter,leave}() are rather fragile, since they must be
 * callable from within destructors.  Funnels aren't deleted until a later GC
 * sweep iteration, which means that any object destructor that is run in the
 * earlier sweeps can call these functions.  However, these functions cannot
 * expect even the hook tags to refer to valid objects.  All they can do is
 * access the funnel mutex. */
void
modslate_funnel_c_enter(cw_nxo_t *a_funnel)
{
    struct cw_funnel *funnel;

    cw_assert(nxo_type_get(a_funnel) == NXOT_HOOK);

    funnel = (struct cw_funnel *) nxo_hook_data_get(a_funnel);
    mtx_lock(&funnel->mtx);
}

void
modslate_funnel_c_leave(cw_nxo_t *a_funnel)
{
    struct cw_funnel *funnel;

    cw_assert(nxo_type_get(a_funnel) == NXOT_HOOK);

    funnel = (struct cw_funnel *) nxo_hook_data_get(a_funnel);
    mtx_unlock(&funnel->mtx);
}

void
modslate_funnel(void *a_data, cw_nxo_t *a_thread)
{
    cw_nxo_t *estack, *ostack, *nxo, *tag;
    cw_nx_t *nx;
    struct cw_funnel *funnel;

    estack = nxo_thread_estack_get(a_thread);
    ostack = nxo_thread_ostack_get(a_thread);
    nx = nxo_thread_nx_get(a_thread);

    funnel = (struct cw_funnel *) nxa_malloc(nx_nxa_get(nx),
					     sizeof(struct cw_funnel));

    /* Create a reference to this hook in order to prevent the module from being
     * prematurely unloaded. */
    nxo_no_new(&funnel->hook);
    nxo_dup(&funnel->hook, nxo_stack_get(estack));

    /* Initialize the funnel mutex. */
    mtx_new(&funnel->mtx);

    /* Create a reference to the funnel. */
    nxo = nxo_stack_push(ostack);
    nxo_hook_new(nxo, nx, funnel, NULL, funnel_p_ref_iter, funnel_p_delete);

    /* Set the hook tag. */
    tag = nxo_hook_tag_get(nxo);
    nxo_name_new(tag, nx, "funnel", sizeof("funnel") - 1, FALSE);
    nxo_attr_set(tag, NXOA_EXECUTABLE);
}

/* #object funnel? #boolean */
void
modslate_funnel_p(void *a_data, cw_nxo_t *a_thread)
{
    modslate_hook_p(a_data, a_thread, "funnel");
}

void
modslate_funnel_enter(void *a_data, cw_nxo_t *a_thread)
{
    cw_nxo_t *ostack, *nxo;
    cw_nxn_t error;
    struct cw_funnel *funnel;

    ostack = nxo_thread_ostack_get(a_thread);
    NXO_STACK_GET(nxo, ostack, a_thread);
    error = modslate_hook_type(nxo, "funnel");
    if (error)
    {
	nxo_thread_nerror(a_thread, error);
	return;
    }
    funnel = (struct cw_funnel *) nxo_hook_data_get(nxo);

    mtx_lock(&funnel->mtx);

    nxo_stack_pop(ostack);
}

void
modslate_funnel_leave(void *a_data, cw_nxo_t *a_thread)
{
    cw_nxo_t *ostack, *nxo;
    cw_nxn_t error;
    struct cw_funnel *funnel;

    ostack = nxo_thread_ostack_get(a_thread);
    NXO_STACK_GET(nxo, ostack, a_thread);
    error = modslate_hook_type(nxo, "funnel");
    if (error)
    {
	nxo_thread_nerror(a_thread, error);
	return;
    }
    funnel = (struct cw_funnel *) nxo_hook_data_get(nxo);

    mtx_unlock(&funnel->mtx);

    nxo_stack_pop(ostack);
}
