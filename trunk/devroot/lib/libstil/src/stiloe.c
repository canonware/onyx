/****************************************************************************
 *
 * <Copyright = "jasone">
 * <License>
 *
 ****************************************************************************
 *
 * Version: <Version>
 *
 ****************************************************************************/

#include "../include/libstil/libstil.h"

cw_stiloe_t	*stiloe_l_array_new(cw_stilt_t *a_stilt);
void		stiloe_l_array_delete(cw_stiloe_t *a_stiloe);
cw_stiloe_t	*stiloe_l_array_ref_iterate(cw_stiloe_t *a_stiloe, cw_bool_t
    a_reset);
cw_stiloe_t	*stiloe_l_string_new(cw_stilt_t *a_stilt);
void		stiloe_l_string_delete(cw_stiloe_t *a_stiloe);
cw_stiloe_t	*stiloe_l_string_ref_iterate(cw_stiloe_t *a_stiloe, cw_bool_t
    a_reset);

typedef cw_stiloe_t	*cw_stiloe_new_t(cw_stilt_t *a_stilt);
typedef void		cw_stiloe_delete_t(cw_stiloe_t *a_stiloe);
typedef cw_stiloe_t	*cw_stiloe_ref_iterate_t(cw_stiloe_t *a_stiloe,
    cw_bool_t a_reset);

typedef struct cw_stiloe_vtable_s cw_stiloe_vtable_t;
struct  cw_stiloe_vtable_s {
	cw_stiloe_new_t		*new_f;
	cw_stiloe_delete_t	*delete_f;
	cw_stiloe_ref_iterate_t	*ref_iterate_f;
};

/*
 * The order of these entries must correspond to the type numbering of
 * cw_stilot_t.  NULL pointers are used in entries that should never get called,
 * so that a segfault will occur if such a non-existent function is called.
 */
static cw_stiloe_vtable_t stiloe_vtable[] = {
	/* _CW_STILOT_NOTYPE */
	{NULL,			NULL,			NULL},

	/* _CW_STILOT_ARRAYTYPE */
	{stiloe_l_array_new,	stiloe_l_array_delete,	stiloe_l_array_ref_iterate},

	/* _CW_STILOT_BOOLEANTYPE */
	{NULL,			NULL,			NULL},

	/* _CW_STILOT_CONDITIONTYPE */
	{NULL,			NULL,			NULL},
	
	/* _CW_STILOT_DICTTYPE */
	{NULL,			NULL,			NULL},

	/* _CW_STILOT_FILETYPE */
	{NULL,			NULL,			NULL},

	/* _CW_STILOT_HOOKTYPE */
	{NULL,			NULL,			NULL},

	/* _CW_STILOT_LOCKTYPE */
	{NULL,			NULL,			NULL},

	/* _CW_STILOT_MARKTYPE */
	{NULL,			NULL,			NULL},

	/* _CW_STILOT_MSTATETYPE */
	{NULL,			NULL,			NULL},

	/* _CW_STILOT_NAMETYPE */
	{NULL,			NULL,			NULL},

	/* _CW_STILOT_NULLTYPE */
	{NULL,			NULL,			NULL},

	/* _CW_STILOT_NUMBERTYPE */
	{NULL,			NULL,			NULL},

	/* _CW_STILOT_OPERATORTYPE */
	{NULL,			NULL,			NULL},

	/* _CW_STILOT_STRINGTYPE */
	{stiloe_l_string_new,	stiloe_l_string_delete,	stiloe_l_string_ref_iterate}
};

cw_stiloe_t *
stiloe_new(cw_stilt_t *a_stilt, cw_stilot_t a_type)
{
	cw_stiloe_t	*retval;

	/* Allocate space and initialize type-specific variables. */
	retval = stiloe_vtable[a_type].new_f(a_stilt);

	/* Initialize the common section. */
	memset(retval, 0, sizeof(cw_stiloe_t));

	retval->type = a_type;
	retval->stilt = a_stilt;

#ifdef _LIBSTIL_DBG
	retval->magic = _CW_STILOE_MAGIC;
#endif
	return retval;
}

void
stiloe_delete(cw_stiloe_t *a_stiloe)
{
	_cw_check_ptr(a_stiloe);
	_cw_assert(a_stiloe->magic == _CW_STILOE_MAGIC);

	stiloe_vtable[a_stiloe->type].delete_f(a_stiloe);

	_cw_stilt_free(a_stiloe->stilt, a_stiloe);
}

cw_stiloe_t *
stiloe_ref_iterate(cw_stiloe_t *a_stiloe, cw_bool_t a_reset)
{
	_cw_check_ptr(a_stiloe);
	_cw_assert(a_stiloe->magic == _CW_STILOE_MAGIC);

	return stiloe_vtable[a_stiloe->type].ref_iterate_f(a_stiloe, a_reset);
}

void
stiloe_gc_register(cw_stiloe_t *a_stiloe)
{
	_cw_check_ptr(a_stiloe);
	_cw_assert(a_stiloe->magic == _CW_STILOE_MAGIC);

	/* XXX */
}
