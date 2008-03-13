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

#define CW_NXO_C_

#include "../include/libonyx/libonyx.h"
#include "../include/libonyx/nx_l.h"
#include "../include/libonyx/nxa_l.h"
#include "../include/libonyx/nxo_l.h"

/* nxo. */
cw_sint32_t
nxo_compare(const cw_nxo_t *a_a, const cw_nxo_t *a_b)
{
    cw_sint32_t retval;

    switch (nxo_type_get(a_a))
    {
	case NXOT_ARRAY:
#ifdef CW_THREADS
	case NXOT_CONDITION:
#endif
	case NXOT_DICT:
	case NXOT_FILE:
	case NXOT_HOOK:
#ifdef CW_THREADS
	case NXOT_MUTEX:
#endif
	case NXOT_STACK:
	case NXOT_THREAD:
	{
	    if (nxo_type_get(a_a) == nxo_type_get(a_b)
		&& a_a->o.nxoe == a_b->o.nxoe)
	    {
		retval = 0;
	    }
	    else
	    {
		retval = 1;
	    }
	    break;
	}
	case NXOT_OPERATOR:
	{
	    if (nxo_type_get(a_b) == NXOT_OPERATOR
		&& a_a->o.operator.f == a_b->o.operator.f)
	    {
		retval = 0;
	    }
	    else
	    {
		retval = 1;
	    }
	    break;
	}
	case NXOT_NAME:
	case NXOT_STRING:
	{
	    const cw_uint8_t *str_a, *str_b;
	    cw_uint32_t len_a, len_b;
#ifdef CW_THREADS
	    cw_bool_t lock_a, lock_b;
#endif

	    if (nxo_type_get(a_a) == NXOT_NAME)
	    {
		str_a = nxo_name_str_get(a_a);
		len_a = nxo_name_len_get(a_a);
#ifdef CW_THREADS
		lock_a = FALSE;
#endif
	    }
	    else
	    {
		str_a = nxo_string_get(a_a);
		len_a = nxo_string_len_get(a_a);
#ifdef CW_THREADS
		lock_a = TRUE;
#endif
	    }

	    if (nxo_type_get(a_b) == NXOT_NAME)
	    {
		str_b = nxo_name_str_get(a_b);
		len_b = nxo_name_len_get(a_b);
#ifdef CW_THREADS
		lock_b = FALSE;
#endif
	    }
	    else if (nxo_type_get(a_b) == NXOT_STRING)
	    {
		str_b = nxo_string_get(a_b);
		len_b = nxo_string_len_get(a_b);
#ifdef CW_THREADS
		lock_b = TRUE;
#endif
	    }
	    else
	    {
		retval = 2;
		break;
	    }

#ifdef CW_THREADS
	    if (lock_a)
	    {
		nxo_string_lock((cw_nxo_t *) a_a);
	    }
	    if (lock_b)
	    {
		nxo_string_lock((cw_nxo_t *) a_b);
	    }
#endif
	    if (len_a == len_b)
	    {
		retval = strncmp(str_a, str_b, len_a);
	    }
	    else if (len_a < len_b)
	    {
		retval = strncmp(str_a, str_b, len_a);
		if (retval == 0)
		{
		    retval = -1;
		}
	    }
	    else
	    {
		retval = strncmp(str_a, str_b, len_b);
		if (retval == 0)
		{
		    retval = 1;
		}
	    }
#ifdef CW_THREADS
	    if (lock_b)
	    {
		nxo_string_unlock((cw_nxo_t *) a_b);
	    }
	    if (lock_a)
	    {
		nxo_string_unlock((cw_nxo_t *) a_a);
	    }
#endif
	    break;
	}
	case NXOT_BOOLEAN:
	{
	    if (nxo_type_get(a_b) != NXOT_BOOLEAN)
	    {
		retval = 2;
		break;
	    }

	    if (a_a->o.boolean.val == a_b->o.boolean.val)
	    {
		retval = 0;
	    }
	    else
	    {
		retval = 1;
	    }
	    break;
	}
	case NXOT_INTEGER:
	{
	    switch (nxo_type_get(a_b))
	    {
		case NXOT_INTEGER:
		{
		    if (a_a->o.integer.i < a_b->o.integer.i)
		    {
			retval = -1;
		    }
		    else if (a_a->o.integer.i == a_b->o.integer.i)
		    {
			retval = 0;
		    }
		    else
		    {
			retval = 1;
		    }
		    break;
		}
#ifdef CW_REAL
		case NXOT_REAL:
		{
		    if (((cw_nxor_t) a_a->o.integer.i) < a_b->o.real.r)
		    {
			retval = -1;
		    }
		    else if (((cw_nxor_t) a_a->o.integer.i) == a_b->o.real.r)
		    {
			retval = 0;
		    }
		    else
		    {
			retval = 1;
		    }
		    
		    break;
		}
#endif
		default:
		{
		    retval = 2;
		    break;
		}
	    }
	    break;
	}
#ifdef CW_REAL
	case NXOT_REAL:
	{
	    switch (nxo_type_get(a_b))
	    {
		case NXOT_INTEGER:
		{
		    if (a_a->o.real.r < ((cw_nxor_t) a_b->o.integer.i))
		    {
			retval = -1;
		    }
		    else if (a_a->o.real.r == ((cw_nxor_t) a_b->o.integer.i))
		    {
			retval = 0;
		    }
		    else
		    {
			retval = 1;
		    }
		    break;
		}
		case NXOT_REAL:
		{
		    if (a_a->o.real.r < a_b->o.real.r)
		    {
			retval = -1;
		    }
		    else if (a_a->o.real.r == a_b->o.real.r)
		    {
			retval = 0;
		    }
		    else
		    {
			retval = 1;
		    }
		    break;
		}
		default:
		{
		    retval = 2;
		    break;
		}
	    }
	    break;
	}
#endif
	case NXOT_FINO:
	case NXOT_MARK:
	case NXOT_NULL:
	case NXOT_PMARK:
	{
	    if (nxo_type_get(a_a) == nxo_type_get(a_b))
	    {
		retval = 0;
	    }
	    else
	    {
		retval = 2;
	    }
	    break;
	}
	default:
	{
	    cw_not_reached();
	}
    }
	
    return retval;
}

cw_nxoe_t *
nxo_nxoe_get(const cw_nxo_t *a_nxo)
{
    cw_nxoe_t *retval;

    cw_check_ptr(a_nxo);
    cw_assert(a_nxo->magic == CW_NXO_MAGIC || nxo_type_get(a_nxo) == NXOT_NO);

    switch (nxo_type_get(a_nxo))
    {
	case NXOT_ARRAY:
#ifdef CW_THREADS
	case NXOT_CONDITION:
#endif
	case NXOT_DICT:
	case NXOT_FILE:
	case NXOT_HOOK:
#ifdef CW_THREADS
	case NXOT_MUTEX:
#endif
	case NXOT_NAME:
	case NXOT_STACK:
	case NXOT_STRING:
	case NXOT_THREAD:
	{
	    retval = a_nxo->o.nxoe;
	    break;
	}
	default:
	{
	    retval = NULL;
	}
    }

    return retval;
}

#ifdef CW_THREADS
cw_bool_t
nxo_lcheck(cw_nxo_t *a_nxo)
{
    cw_bool_t retval;

    cw_check_ptr(a_nxo);
    cw_dassert(a_nxo->magic == CW_NXO_MAGIC);

#ifdef CW_DBG
    switch (nxo_type_get(a_nxo))
    {
	case NXOT_ARRAY:
	case NXOT_DICT:
	case NXOT_FILE:
	case NXOT_STACK:
	case NXOT_STRING:
	{
	    retval = a_nxo->o.nxoe->locking;
	    break;
	}
	default:
	{
	    cw_not_reached();
	}
    }
#else
    retval = a_nxo->o.nxoe->locking;
#endif

    return retval;
}
#endif

/* nxoe. */
/* Can be called at any time during nxoe_* initialization. */
void
nxoe_l_new(cw_nxoe_t *a_nxoe, cw_nxot_t a_type, cw_bool_t a_locking)
{
    /* Initialize the common section. */
    memset(a_nxoe, 0, sizeof(cw_nxoe_t));

    qr_new(a_nxoe, link);
    a_nxoe->type = a_type;
#ifdef CW_THREADS
    a_nxoe->locking = a_locking;
#endif
#ifdef CW_DBG
    a_nxoe->magic = CW_NXOE_MAGIC;
#endif
}
