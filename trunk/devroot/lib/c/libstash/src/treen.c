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

#include "../include/libstash/libstash.h"

#ifdef _LIBSTASH_DBG
#define _CW_TREEN_MAGIC 0x34561278
#endif

cw_treen_t *
treen_new(cw_treen_t *a_treen, cw_mem_t *a_mem, cw_opaque_dealloc_t
    *a_dealloc_func, void *a_dealloc_arg)
{
	cw_treen_t	*retval;

	if (a_treen != NULL) {
		retval = a_treen;
		memset(retval, 0, sizeof(cw_treen_t));
		retval->dealloc_func = a_dealloc_func;
		retval->dealloc_arg = a_dealloc_arg;
	} else {
		retval = (cw_treen_t *)_cw_mem_malloc(a_mem,
		    sizeof(cw_treen_t));
		if (retval == NULL)
			goto RETURN;
		memset(retval, 0, sizeof(cw_treen_t));
		retval->dealloc_func = (cw_opaque_dealloc_t *)mem_free;
		retval->dealloc_arg = a_mem;
	}

	qr_new(retval, sib_link);

#ifdef _LIBSTASH_DBG
	retval->magic_a = _CW_TREEN_MAGIC;
	retval->size_of = sizeof(cw_treen_t);
	retval->magic_b = _CW_TREEN_MAGIC;
#endif

	RETURN:
	return retval;
}

void
treen_delete(cw_treen_t *a_treen)
{
	cw_treen_t	*child;

	_cw_check_ptr(a_treen);
	_cw_assert(a_treen->magic_a == _CW_TREEN_MAGIC);
	_cw_assert(a_treen->size_of == sizeof(cw_treen_t));
	_cw_assert(a_treen->magic_b == _CW_TREEN_MAGIC);

	/* Recursively delete all subtrees. */
	while ((child = treen_child_get(a_treen)) != NULL)
		treen_delete(child);

	/* Delete self. */
	treen_link(a_treen, NULL);
	if (a_treen->dealloc_func != NULL) {
		_cw_opaque_dealloc(a_treen->dealloc_func, a_treen->dealloc_arg,
		    a_treen);
	}
#ifdef _LIBSTASH_DBG
	else
		memset(a_treen, 0x5a, sizeof(cw_treen_t));
#endif
}

void
treen_link(cw_treen_t *a_treen, cw_treen_t *a_parent)
{
	_cw_check_ptr(a_treen);
	_cw_assert(a_treen->magic_a == _CW_TREEN_MAGIC);
	_cw_assert(a_treen->size_of == sizeof(cw_treen_t));
	_cw_assert(a_treen->magic_b == _CW_TREEN_MAGIC);

	/*
	 * Extract ourselves from any current linkage before linking somewhere
	 * else.
	 */
	if (a_treen->parent != NULL) {
		if (a_treen == a_treen->parent->child) {
			if (treen_sibling_get(a_treen) != a_treen) {
				/*
				 * The parent's child pointer points to a_treen,
				 * and this isn't the only child, so parent's
				 * child pointer needs to be changed to another
				 * child.
				 */
				a_treen->parent->child =
				    treen_sibling_get(a_treen);
			} else {
				/* Last child. */
				a_treen->parent->child = NULL;
			}
		}
		a_treen->parent = NULL;

		qr_remove(a_treen, sib_link);
	}
	if (a_parent != NULL) {
		_cw_assert(_CW_TREEN_MAGIC == a_parent->magic_a);
		_cw_assert(a_parent->size_of == sizeof(cw_treen_t));
		_cw_assert(_CW_TREEN_MAGIC == a_parent->magic_b);

		a_treen->parent = a_parent;

		if (a_parent->child == NULL) {
			/* The parent has no children yet. */
			a_parent->child = a_treen;
		} else {
			cw_treen_t	*sibling = treen_child_get(a_parent);

			qr_meld(sibling, a_treen, sib_link);
		}
	}
}

cw_treen_t *
treen_parent_get(cw_treen_t *a_treen)
{
	_cw_check_ptr(a_treen);
	_cw_assert(a_treen->magic_a == _CW_TREEN_MAGIC);
	_cw_assert(a_treen->size_of == sizeof(cw_treen_t));
	_cw_assert(a_treen->magic_b == _CW_TREEN_MAGIC);

	return a_treen->parent;
}

cw_treen_t *
treen_child_get(cw_treen_t *a_treen)
{
	_cw_check_ptr(a_treen);
	_cw_assert(a_treen->magic_a == _CW_TREEN_MAGIC);
	_cw_assert(a_treen->size_of == sizeof(cw_treen_t));
	_cw_assert(a_treen->magic_b == _CW_TREEN_MAGIC);

	return a_treen->child;
}

cw_treen_t *
treen_sibling_get(cw_treen_t *a_treen)
{
	cw_treen_t	*retval;

	_cw_check_ptr(a_treen);
	_cw_assert(a_treen->magic_a == _CW_TREEN_MAGIC);
	_cw_assert(a_treen->size_of == sizeof(cw_treen_t));
	_cw_assert(a_treen->magic_b == _CW_TREEN_MAGIC);

	retval = qr_next(a_treen, sib_link);

	return retval;
}

void *
treen_data_ptr_get(cw_treen_t *a_treen)
{
	_cw_check_ptr(a_treen);
	_cw_assert(a_treen->magic_a == _CW_TREEN_MAGIC);
	_cw_assert(a_treen->size_of == sizeof(cw_treen_t));
	_cw_assert(a_treen->magic_b == _CW_TREEN_MAGIC);

	return a_treen->data;
}

void
treen_data_ptr_set(cw_treen_t *a_treen, void *a_data)
{
	_cw_check_ptr(a_treen);
	_cw_assert(a_treen->magic_a == _CW_TREEN_MAGIC);
	_cw_assert(a_treen->size_of == sizeof(cw_treen_t));
	_cw_assert(a_treen->magic_b == _CW_TREEN_MAGIC);

	a_treen->data = a_data;
}
