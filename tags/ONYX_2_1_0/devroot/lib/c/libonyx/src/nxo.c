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

#define	_NXO_C_

#include "../include/libonyx/libonyx.h"
#include "../include/libonyx/nx_l.h"
#include "../include/libonyx/nxa_l.h"
#include "../include/libonyx/nxo_l.h"
#include "../include/libonyx/nxo_array_l.h"
#ifdef _CW_THREADS
#include "../include/libonyx/nxo_condition_l.h"
#endif
#include "../include/libonyx/nxo_dict_l.h"
#include "../include/libonyx/nxo_file_l.h"
#include "../include/libonyx/nxo_hook_l.h"
#ifdef _CW_THREADS
#include "../include/libonyx/nxo_mutex_l.h"
#endif
#include "../include/libonyx/nxo_name_l.h"
#include "../include/libonyx/nxo_operator_l.h"
#include "../include/libonyx/nxo_stack_l.h"
#include "../include/libonyx/nxo_string_l.h"
#include "../include/libonyx/nxo_thread_l.h"

/*
 * vtable setup for the various operations on nxo's that are polymorphic.
 */
typedef void	cw_nxot_delete_t(cw_nxoe_t *a_nxoe, cw_nxa_t *a_nxa);
typedef cw_nxoe_t *cw_nxot_ref_iter_t(cw_nxoe_t *a_nxoe, cw_bool_t a_reset);

typedef struct cw_nxot_vtable_s cw_nxot_vtable_t;
struct  cw_nxot_vtable_s {
	cw_nxot_delete_t	*delete_f;
	cw_nxot_ref_iter_t	*ref_iter_f;
};

/*
 * The order of these entries must correspond to the type numbering of
 * cw_nxot_t.  NULL pointers are used in entries that should never get called,
 * so that a segfault will occur if such a non-existent function is called.
 */
static const cw_nxot_vtable_t nxot_vtable[] = {
	/* NXOT_NO */
	{NULL,
	 NULL},

	/* NXOT_ARRAY */
	{nxoe_l_array_delete,
	 nxoe_l_array_ref_iter},

	/* NXOT_BOOLEAN */
	{NULL,
	 NULL},

#ifdef _CW_THREADS
	/* NXOT_CONDITION */
	{nxoe_l_condition_delete,
	 nxoe_l_condition_ref_iter},
#endif
	
	/* NXOT_DICT */
	{nxoe_l_dict_delete,
	 nxoe_l_dict_ref_iter},

	/* NXOT_FILE */
	{nxoe_l_file_delete,
	 nxoe_l_file_ref_iter},

	/* NXOT_FINO */
	{NULL,
	 NULL},

	/* NXOT_HOOK */
	{nxoe_l_hook_delete,
	 nxoe_l_hook_ref_iter},

	/* NXOT_INTEGER */
	{NULL,
	 NULL},

	/* NXOT_MARK */
	{NULL,
	 NULL},

#ifdef _CW_THREADS
	/* NXOT_MUTEX */
	{nxoe_l_mutex_delete,
	 nxoe_l_mutex_ref_iter},
#endif

	/* NXOT_NAME */
	{nxoe_l_name_delete,
	 nxoe_l_name_ref_iter},

	/* NXOT_NULL */
	{NULL,
	 NULL},

	/* NXOT_OPERATOR */
	{NULL,
	 NULL},

	/* NXOT_PMARK */
	{NULL,
	 NULL},

	/* NXOT_STACK */
	{nxoe_l_stack_delete,
	 nxoe_l_stack_ref_iter},

	/* NXOT_STRING */
	{nxoe_l_string_delete,
	 nxoe_l_string_ref_iter},

	/* NXOT_THREAD */
	{nxoe_l_thread_delete,
	 nxoe_l_thread_ref_iter}
};

/*
 * nxo.
 */
cw_sint32_t
nxo_compare(cw_nxo_t *a_a, cw_nxo_t *a_b)
{
	cw_sint32_t	retval;

	switch (nxo_type_get(a_a)) {
	case NXOT_ARRAY:
#ifdef _CW_THREADS
	case NXOT_CONDITION:
#endif
	case NXOT_DICT:
	case NXOT_FILE:
	case NXOT_HOOK:
#ifdef _CW_THREADS
	case NXOT_MUTEX:
#endif
	case NXOT_STACK:
	case NXOT_THREAD:
		if (nxo_type_get(a_a) == nxo_type_get(a_b) && a_a->o.nxoe ==
		    a_b->o.nxoe)
			retval = 0;
		else
			retval = 2;
		break;
	case NXOT_OPERATOR:
		if (nxo_type_get(a_b) == NXOT_OPERATOR && a_a->o.operator.f
		    == a_b->o.operator.f
#ifdef _CW_USE_INLINES
		    /*
		     * Fast operators have NULL function pointers, so take care
		     * to check fast operators for equality.  Doing this
		     * additional test unconditionally would be safe, but there
		     * is no need to do it unless inlines are used.
		     */
		    && nxo_l_operator_fast_op_nxn(a_a) ==
		    nxo_l_operator_fast_op_nxn(a_b)
#endif
		    )
			retval = 0;
		else
			retval = 2;
		break;
	case NXOT_NAME:
	case NXOT_STRING: {
		const cw_uint8_t	*str_a, *str_b;
		cw_uint32_t		len_a, len_b;
#ifdef _CW_THREADS
		cw_bool_t		lock_a, lock_b;
#endif

		if (nxo_type_get(a_a) == NXOT_NAME) {
			str_a = nxo_name_str_get(a_a);
			len_a = nxo_name_len_get(a_a);
#ifdef _CW_THREADS
			lock_a = FALSE;
#endif
		} else {
			str_a = nxo_string_get(a_a);
			len_a = nxo_string_len_get(a_a);
#ifdef _CW_THREADS
			lock_a = TRUE;
#endif
		}
			
		if (nxo_type_get(a_b) == NXOT_NAME) {
			str_b = nxo_name_str_get(a_b);
			len_b = nxo_name_len_get(a_b);
#ifdef _CW_THREADS
			lock_b = FALSE;
#endif
		} else if (nxo_type_get(a_b) == NXOT_STRING) {
			str_b = nxo_string_get(a_b);
			len_b = nxo_string_len_get(a_b);
#ifdef _CW_THREADS
			lock_b = TRUE;
#endif
		} else {
			retval = 2;
			break;
		}

#ifdef _CW_THREADS
		if (lock_a)
			nxo_string_lock(a_a);
		if (lock_b)
			nxo_string_lock(a_b);
#endif
		if (len_a == len_b)
			retval = strncmp(str_a, str_b, len_a);
		else if (len_a < len_b) {
			retval = strncmp(str_a, str_b, len_a);
			if (retval == 0)
				retval = -1;
		} else {
			retval = strncmp(str_a, str_b, len_b);
			if (retval == 0)
				retval = 1;
		}
#ifdef _CW_THREADS
		if (lock_b)
			nxo_string_unlock(a_b);
		if (lock_a)
			nxo_string_unlock(a_a);
#endif
		break;
	}
	case NXOT_BOOLEAN:
		if (nxo_type_get(a_a) != nxo_type_get(a_b)) {
			retval = 2;
			break;
		}

		if (a_a->o.boolean.val == a_b->o.boolean.val)
			retval = 0;
		else
			retval = 1;
		break;
	case NXOT_INTEGER:
		if (nxo_type_get(a_a) != nxo_type_get(a_b)) {
			retval = 2;
			break;
		}

		if (a_a->o.integer.i < a_b->o.integer.i)
			retval = -1;
		else if (a_a->o.integer.i == a_b->o.integer.i)
			retval = 0;
		else
			retval = 1;
		break;
	case NXOT_FINO:
	case NXOT_MARK:
	case NXOT_NULL:
	case NXOT_PMARK:
		if (nxo_type_get(a_a) == nxo_type_get(a_b))
			retval = 0;
		else
			retval = 2;
		break;
	default:
		_cw_not_reached();
	}
	
	return retval;
}

cw_nxoe_t *
nxo_nxoe_get(cw_nxo_t *a_nxo)
{
	cw_nxoe_t	*retval;

	_cw_check_ptr(a_nxo);
	_cw_assert(a_nxo->magic == _CW_NXO_MAGIC || nxo_type_get(a_nxo) ==
	    NXOT_NO);

	switch (nxo_type_get(a_nxo)) {
	case NXOT_ARRAY:
#ifdef _CW_THREADS
	case NXOT_CONDITION:
#endif
	case NXOT_DICT:
	case NXOT_FILE:
	case NXOT_HOOK:
#ifdef _CW_THREADS
	case NXOT_MUTEX:
#endif
	case NXOT_NAME:
	case NXOT_STACK:
	case NXOT_STRING:
	case NXOT_THREAD:
		retval = a_nxo->o.nxoe;
		break;
	default:
		retval = NULL;
	}

	return retval;
}

#ifdef _CW_THREADS
cw_bool_t
nxo_lcheck(cw_nxo_t *a_nxo)
{
	cw_bool_t	retval;

	_cw_check_ptr(a_nxo);
	_cw_dassert(a_nxo->magic == _CW_NXO_MAGIC);

#ifdef _CW_DBG
	switch (nxo_type_get(a_nxo)) {
	case NXOT_ARRAY:
	case NXOT_DICT:
	case NXOT_FILE:
	case NXOT_STACK:
	case NXOT_STRING:
		retval = a_nxo->o.nxoe->locking;
		break;
	default:
		_cw_not_reached();
	}
#else
	retval = a_nxo->o.nxoe->locking;
#endif

	return retval;
}
#endif

/*
 * nxoe.
 */
/* Can be called at any time during nxoe_* initialization. */
void
nxoe_l_new(cw_nxoe_t *a_nxoe, cw_nxot_t a_type, cw_bool_t a_locking)
{
	/* Initialize the common section. */
	memset(a_nxoe, 0, sizeof(cw_nxoe_t));

	qr_new(a_nxoe, link);
	a_nxoe->type = a_type;
#ifdef _CW_THREADS
	a_nxoe->locking = a_locking;
#endif
#ifdef _CW_DBG
	a_nxoe->magic = _CW_NXOE_MAGIC;
#endif
}

void
nxoe_l_delete(cw_nxoe_t *a_nxoe, cw_nxa_t *a_nxa)
{
	_cw_check_ptr(a_nxoe);
	_cw_dassert(a_nxoe->magic == _CW_NXOE_MAGIC);

	nxot_vtable[a_nxoe->type].delete_f(a_nxoe, a_nxa);
}

cw_nxoe_t *
nxoe_l_ref_iter(cw_nxoe_t *a_nxoe, cw_bool_t a_reset)
{
	cw_nxoe_t	*retval;

	_cw_check_ptr(a_nxoe);
	_cw_dassert(a_nxoe->magic == _CW_NXOE_MAGIC);

	retval = nxot_vtable[a_nxoe->type].ref_iter_f(a_nxoe, a_reset);

	return retval;
}
