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
#include "../include/libstil/stil_l.h"
#include "../include/libstil/stila_l.h"
#include "../include/libstil/stilo_l.h"
#include "../include/libstil/stilo_array_l.h"
#include "../include/libstil/stilo_boolean_l.h"
#include "../include/libstil/stilo_condition_l.h"
#include "../include/libstil/stilo_dict_l.h"
#include "../include/libstil/stilo_file_l.h"
#include "../include/libstil/stilo_fino_l.h"
#include "../include/libstil/stilo_hook_l.h"
#include "../include/libstil/stilo_integer_l.h"
#include "../include/libstil/stilo_mark_l.h"
#include "../include/libstil/stilo_mutex_l.h"
#include "../include/libstil/stilo_name_l.h"
#include "../include/libstil/stilo_no_l.h"
#include "../include/libstil/stilo_null_l.h"
#include "../include/libstil/stilo_operator_l.h"
#include "../include/libstil/stilo_stack_l.h"
#include "../include/libstil/stilo_string_l.h"
#include "../include/libstil/stilt_l.h"

/*
 * vtable setup for the various operations on stilo's that are polymorphic.
 */
typedef void		cw_stilot_delete_t(cw_stiloe_t *a_stiloe, cw_stil_t
    *a_stil);
typedef cw_stiloe_t	*cw_stilot_ref_iter_t(cw_stiloe_t *a_stiloe,
    cw_bool_t a_reset);
typedef cw_stilte_t	cw_stilot_copy_t(cw_stilo_t *a_to, cw_stilo_t *a_from,
    cw_stilt_t *a_stilt);
typedef cw_stilte_t	cw_stilot_print_t(cw_stilo_t *a_stilo, cw_stilo_t
    *a_file, cw_uint32_t a_depth);

typedef struct cw_stilot_vtable_s cw_stilot_vtable_t;
struct  cw_stilot_vtable_s {
	cw_stilot_delete_t	*delete_f;
	cw_stilot_ref_iter_t	*ref_iter_f;
	cw_stilot_print_t	*print_f;
};

/*
 * The order of these entries must correspond to the type numbering of
 * cw_stilot_t.  NULL pointers are used in entries that should never get called,
 * so that a segfault will occur if such a non-existent function is called.
 */
static const cw_stilot_vtable_t stilot_vtable[] = {
	/* STILOT_NO */
	{NULL,
	 NULL,
	 stilo_l_no_print},	/*
				 * Debugging only, should never end up getting
				 * called during normal operation.
				 */

	/* STILOT_ARRAY */
	{stiloe_l_array_delete,
	 stiloe_l_array_ref_iter,
	 stilo_l_array_print},

	/* STILOT_BOOLEAN */
	{NULL,
	 NULL,
	 stilo_l_boolean_print},

	/* STILOT_CONDITION */
	{stiloe_l_condition_delete,
	 stiloe_l_condition_ref_iter,
	 stilo_l_condition_print},
	
	/* STILOT_DICT */
	{stiloe_l_dict_delete,
	 stiloe_l_dict_ref_iter,
	 stilo_l_dict_print},

	/* STILOT_FILE */
	{stiloe_l_file_delete,
	 stiloe_l_file_ref_iter,
	 stilo_l_file_print},

	/* STILOT_FINO */
	{NULL,
	 NULL,
	 stilo_l_fino_print},

	/* STILOT_HOOK */
	{stiloe_l_hook_delete,
	 stiloe_l_hook_ref_iter,
	 stilo_l_hook_print},

	/* STILOT_INTEGER */
	{NULL,
	 NULL,
	 stilo_l_integer_print},

	/* STILOT_MARK */
	{NULL,
	 NULL,
	 stilo_l_mark_print},

	/* STILOT_MUTEX */
	{stiloe_l_mutex_delete,
	 stiloe_l_mutex_ref_iter,
	 stilo_l_mutex_print},

	/* STILOT_NAME */
	{stiloe_l_name_delete,
	 stiloe_l_name_ref_iter,
	 stilo_l_name_print},

	/* STILOT_NULL */
	{NULL,
	 NULL,
	 stilo_l_null_print},

	/* STILOT_OPERATOR */
	{NULL,
	 NULL,
	 stilo_l_operator_print},

	/* STILOT_STACK */
	{stiloe_l_stack_delete,
	 stiloe_l_stack_ref_iter,
	 stilo_l_stack_print},

	/* STILOT_STRING */
	{stiloe_l_string_delete,
	 stiloe_l_string_ref_iter,
	 stilo_l_string_print}
};

/*
 * stilo.
 */
cw_sint32_t
stilo_compare(cw_stilo_t *a_a, cw_stilo_t *a_b)
{
	cw_sint32_t	retval;

	switch (a_a->type) {
	case STILOT_ARRAY:
	case STILOT_CONDITION:
	case STILOT_DICT:
	case STILOT_FILE:
	case STILOT_HOOK:
	case STILOT_MUTEX:
	case STILOT_STACK:
		if (a_a->type == a_b->type && a_a->o.stiloe == a_b->o.stiloe)
			retval = 0;
		else
			retval = 2;
		break;
	case STILOT_OPERATOR:
		if (a_a->type == a_b->type && a_a->o.operator.f ==
		    a_b->o.operator.f)
			retval = 0;
		else
			retval = 2;
		break;
	case STILOT_NAME:
	case STILOT_STRING: {
		const cw_uint8_t	*str_a, *str_b;
		cw_uint32_t		len_a, len_b;
		cw_bool_t		lock_a, lock_b;

		if (a_a->type == STILOT_NAME) {
			str_a = stilo_name_str_get(a_a);
			len_a = stilo_name_len_get(a_a);
			lock_a = FALSE;
		} else {
			str_a = stilo_string_get(a_a);
			len_a = stilo_string_len_get(a_a);
			lock_a = TRUE;
		}
			
		if (a_b->type == STILOT_NAME) {
			str_b = stilo_name_str_get(a_b);
			len_b = stilo_name_len_get(a_b);
			lock_b = FALSE;
		} else if (a_b->type == STILOT_STRING) {
			str_b = stilo_string_get(a_b);
			len_b = stilo_string_len_get(a_b);
			lock_b = TRUE;
		} else {
			retval = 2;
			break;
		}

		if (lock_a)
			stilo_string_lock(a_a);
		if (lock_b)
			stilo_string_lock(a_b);
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
		if (lock_b)
			stilo_string_unlock(a_b);
		if (lock_a)
			stilo_string_unlock(a_a);
		break;
	}
	case STILOT_BOOLEAN:
		if (a_a->type != a_b->type) {
			retval = 2;
			break;
		}

		if (a_a->o.boolean.val == a_b->o.boolean.val)
			retval = 0;
		else
			retval = 1;
		break;
	case STILOT_INTEGER:
		if (a_a->type != a_b->type) {
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
	case STILOT_FINO:
	case STILOT_MARK:
	case STILOT_NULL:
		if (a_a->type == a_b->type)
			retval = 0;
		else
			retval = 2;
		break;
	default:
		_cw_not_reached();
	}
	
	return retval;
}

cw_stiloe_t *
stilo_stiloe_get(cw_stilo_t *a_stilo)
{
	cw_stiloe_t	*retval;

	_cw_check_ptr(a_stilo);
	_cw_assert(a_stilo->magic == _CW_STILO_MAGIC || a_stilo->type ==
	    STILOT_NO);

	switch (a_stilo->type) {
	case STILOT_ARRAY:
	case STILOT_CONDITION:
	case STILOT_DICT:
	case STILOT_FILE:
	case STILOT_HOOK:
	case STILOT_MUTEX:
	case STILOT_NAME:
	case STILOT_STACK:
	case STILOT_STRING:
		retval = a_stilo->o.stiloe;
		break;
	default:
		retval = NULL;
	}

	return retval;
}

cw_bool_t
stilo_lcheck(cw_stilo_t *a_stilo)
{
	cw_bool_t	retval;

	_cw_check_ptr(a_stilo);
	_cw_assert(a_stilo->magic == _CW_STILO_MAGIC);

#ifdef _LIBSTIL_DBG
	switch (a_stilo->type) {
	case STILOT_ARRAY:
	case STILOT_DICT:
	case STILOT_FILE:
	case STILOT_STACK:
	case STILOT_STRING:
		retval = a_stilo->o.stiloe->locking;
		break;
	default:
		_cw_not_reached();
	}
#else
	retval = a_stilo->o.stiloe->locking;
#endif

	return retval;
}

cw_stilte_t
stilo_print(cw_stilo_t *a_stilo, cw_stilo_t *a_file, cw_uint32_t a_depth,
    cw_bool_t a_newline)
{
	cw_stilte_t	retval;

	retval = stilot_vtable[a_stilo->type].print_f(a_stilo, a_file, a_depth);
	if (retval)
		goto RETURN;

	if (a_newline)
		retval = stilo_file_output(a_file, "\n");

	RETURN:
	return retval;
}

/*
 * stiloe.
 */
/* Can be called at any time during stiloe_* initialization. */
void
stiloe_l_new(cw_stiloe_t *a_stiloe, cw_stilot_t a_type, cw_bool_t a_locking)
{
	/* Initialize the common section. */
	memset(a_stiloe, 0, sizeof(cw_stiloe_t));

	qr_new(a_stiloe, link);
	a_stiloe->type = a_type;
	a_stiloe->locking = a_locking;
#ifdef _LIBSTIL_DBG
	a_stiloe->magic = _CW_STILOE_MAGIC;
#endif
}

void
stiloe_l_delete(cw_stiloe_t *a_stiloe, cw_stil_t *a_stil)
{
	_cw_check_ptr(a_stiloe);
	_cw_assert(a_stiloe->magic == _CW_STILOE_MAGIC);

	stilot_vtable[a_stiloe->type].delete_f(a_stiloe, a_stil);
}

cw_stiloe_t *
stiloe_l_ref_iter(cw_stiloe_t *a_stiloe, cw_bool_t a_reset)
{
	cw_stiloe_t	*retval;

	_cw_check_ptr(a_stiloe);
	_cw_assert(a_stiloe->magic == _CW_STILOE_MAGIC);

	retval = stilot_vtable[a_stiloe->type].ref_iter_f(a_stiloe, a_reset);

	return retval;
}
