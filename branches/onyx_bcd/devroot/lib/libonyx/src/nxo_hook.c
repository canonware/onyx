/* -*- mode: c ; c-file-style: "canonware-c-style" -*-
 ******************************************************************************
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
#include "../include/libonyx/nxa_l.h"
#include "../include/libonyx/nxo_l.h"
#include "../include/libonyx/nxo_hook_l.h"
#include "../include/libonyx/nxo_name_l.h"

void
nxo_hook_new(cw_nxo_t *a_nxo, cw_nx_t *a_nx, void *a_data,
	     cw_nxo_hook_eval_t *a_eval_f, cw_nxo_hook_ref_iter_t *a_ref_iter_f,
	     cw_nxo_hook_delete_t *a_delete_f)
{
    cw_nxoe_hook_t *hook;

    hook = (cw_nxoe_hook_t *) nxa_malloc(nx_nxa_get(a_nx),
					 sizeof(cw_nxoe_hook_t));

    nxoe_l_new(&hook->nxoe, NXOT_HOOK, FALSE);
    nxo_null_new(&hook->tag);
    hook->data = a_data;
    hook->eval_f = a_eval_f;
    hook->ref_iter_f = a_ref_iter_f;
    hook->delete_f = a_delete_f;

    nxo_no_new(a_nxo);
    a_nxo->o.nxoe = (cw_nxoe_t *) hook;
    nxo_p_type_set(a_nxo, NXOT_HOOK);

    nxa_l_gc_register(nx_nxa_get(a_nx), (cw_nxoe_t *) hook);
}

cw_bool_t
nxoe_l_hook_delete(cw_nxoe_t *a_nxoe, cw_nxa_t *a_nxa, cw_uint32_t a_iter)
{
    cw_bool_t retval;
    cw_nxoe_hook_t *hook;

    hook = (cw_nxoe_hook_t *) a_nxoe;

    cw_check_ptr(hook);
    cw_dassert(hook->nxoe.magic == CW_NXOE_MAGIC);
    cw_assert(hook->nxoe.type == NXOT_HOOK);

    if (hook->delete_f != NULL)
    {
	retval = hook->delete_f(hook->data, nxa_nx_get(a_nxa), a_iter);
    }
    else
    {
	retval = FALSE;
    }

    if (retval == FALSE)
    {
	nxa_free(a_nxa, hook, sizeof(cw_nxoe_hook_t));
    }

    return retval;
}

cw_nxoe_t *
nxoe_l_hook_ref_iter(cw_nxoe_t *a_nxoe, cw_bool_t a_reset)
{
    cw_nxoe_t *retval;
    cw_nxoe_hook_t *hook;

    hook = (cw_nxoe_hook_t *) a_nxoe;

    if (a_reset)
    {
	hook->ref_iter = 0;
    }

    switch (hook->ref_iter)
    {
	case 0:
	{
	    hook->ref_iter++;
	    retval = nxo_nxoe_get(&hook->tag);
	    if (retval != NULL)
	    {
		break;
	    }
	}
	case 1:
	{
	    hook->ref_iter++;
	    if (hook->ref_iter_f != NULL)
	    {
		retval = hook->ref_iter_f(hook->data, TRUE);
	    }
	    else
	    {
		retval = NULL;
	    }
	    break;
	}
	case 2:
	{
	    retval = hook->ref_iter_f(hook->data, FALSE);
	    break;
	}
	default:
	{
	    cw_not_reached();
	}
    }

    return retval;
}

cw_nxo_t *
nxo_hook_tag_get(cw_nxo_t *a_nxo)
{
    cw_nxo_t *retval;
    cw_nxoe_hook_t *hook;

    cw_check_ptr(a_nxo);
    cw_dassert(a_nxo->magic == CW_NXO_MAGIC);
    cw_assert(nxo_type_get(a_nxo) == NXOT_HOOK);

    hook = (cw_nxoe_hook_t *) a_nxo->o.nxoe;

    cw_check_ptr(hook);
    cw_dassert(hook->nxoe.magic == CW_NXOE_MAGIC);
    cw_assert(hook->nxoe.type == NXOT_HOOK);

    retval = &hook->tag;

    return retval;
}

void *
nxo_hook_data_get(cw_nxo_t *a_nxo)
{
    void *retval;
    cw_nxoe_hook_t *hook;

    cw_check_ptr(a_nxo);
    cw_dassert(a_nxo->magic == CW_NXO_MAGIC);
    cw_assert(nxo_type_get(a_nxo) == NXOT_HOOK);

    hook = (cw_nxoe_hook_t *) a_nxo->o.nxoe;

    cw_check_ptr(hook);
    cw_dassert(hook->nxoe.magic == CW_NXOE_MAGIC);
    cw_assert(hook->nxoe.type == NXOT_HOOK);

    retval = hook->data;

    return retval;
}

void
nxo_hook_data_set(cw_nxo_t *a_nxo, void *a_data)
{
    cw_nxoe_hook_t *hook;

    cw_check_ptr(a_nxo);
    cw_dassert(a_nxo->magic == CW_NXO_MAGIC);
    cw_assert(nxo_type_get(a_nxo) == NXOT_HOOK);

    hook = (cw_nxoe_hook_t *) a_nxo->o.nxoe;

    cw_check_ptr(hook);
    cw_dassert(hook->nxoe.magic == CW_NXOE_MAGIC);
    cw_assert(hook->nxoe.type == NXOT_HOOK);

    hook->data = a_data;
}

void
nxo_hook_eval(cw_nxo_t *a_nxo, cw_nxo_t *a_thread)
{
    cw_nxoe_hook_t *hook;

    cw_check_ptr(a_nxo);
    cw_dassert(a_nxo->magic == CW_NXO_MAGIC);
    cw_assert(nxo_type_get(a_nxo) == NXOT_HOOK);

    hook = (cw_nxoe_hook_t *) a_nxo->o.nxoe;

    cw_check_ptr(hook);
    cw_dassert(hook->nxoe.magic == CW_NXOE_MAGIC);
    cw_assert(hook->nxoe.type == NXOT_HOOK);

    if (hook->eval_f != NULL)
    {
	hook->eval_f(hook->data, a_thread);
    }
    else
    {
	cw_nxo_t *ostack, *nxo;

	/* This hook can't be executed, so push it onto ostack just like a
	 * normal literal object would be. */
	ostack = nxo_thread_ostack_get(a_thread);
	nxo = nxo_stack_push(ostack);
	nxo_dup(nxo, a_nxo);
    }
}
