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

#ifdef _LIBSTIL_DBG
#define _CW_STILO_MAGIC		0x398754ba
#define _CW_STILOE_MAGIC	0x0fa6e798
#endif

typedef struct cw_stiloe_array_s cw_stiloe_array_t;
typedef struct cw_stiloe_condition_s cw_stiloe_condition_t;
typedef struct cw_stiloe_dict_s cw_stiloe_dict_t;
typedef struct cw_stiloe_hook_s cw_stiloe_hook_t;
typedef struct cw_stiloe_lock_s cw_stiloe_lock_t;
typedef struct cw_stiloe_mstate_s cw_stiloe_mstate_t;
typedef struct cw_stiloe_number_s cw_stiloe_number_t;
typedef struct cw_stiloe_string_s cw_stiloe_string_t;

/*
 * All extended type objects contain a stiloe.  This provides a poor man's
 * inheritance.  Since stil's type system is static, this idiom is adequate.
 */
struct cw_stiloe_s {
#if (defined(_LIBSTIL_DBG) || defined(_LIBSTIL_DEBUG))
	cw_uint32_t	magic;
#endif

	cw_stilot_t	type:4;
	/*
	 * If TRUE, there is a watchpoint set on this object.  In general, this
	 * field is not looked at unless the interpreter has been put into
	 * debugging mode. Note that setting a watchpoint on an extended type
	 * causes modification via *any* reference to be watched.
	 */
	cw_bool_t	watchpoint:1;
	/* If TRUE, this object is black (or gray).  Otherwise it is white. */
	cw_bool_t	black:1;
	/* Allocated locally or globally? */
	cw_bool_t	global:1;
	/*
	 * If TRUE, this object cannot be modified, which means it need not be
	 * locked, even if global.
	 */
	cw_bool_t	immutable:1;
	/*
	 * If TRUE, this stiloe is a reference to another stiloe.
	 */
	cw_bool_t	indirect:1;
	/*
	 * We need a way to add a stiloe to the sequence set if there become
	 * multiple references to this stiloe.
	 */
	cw_stilt_t	*stilt;
};

struct cw_stiloe_array_s {
	cw_stiloe_t	stiloe;
	cw_uint32_t	iterations;	/*
					 * Used for remembering the current
					 * state of reference iteration.
					 */
	union {
		struct {
			cw_stilo_t	stilo;
			cw_uint32_t	beg_offset;
			cw_uint32_t	len;
		}	i;
		struct {
			cw_stilo_t	*arr;
			cw_uint32_t	len;
		}	a;
	}	e;
};

struct cw_stiloe_condition_s {
	cw_stiloe_t	stiloe;
	cw_cnd_t	condition;
};

struct cw_stiloe_dict_s {
	cw_stiloe_t	stiloe;

	/* stiloe_dicto's are allocated from here. */
	cw_pezz_t	*stiloe_dicto_pezz;

	/*
	 * Name/value pairs.  The keys are (cw_stiloe_name_t *), and the values
	 * are (cw_stiloe_dicto_t *).  The stilo from which the key is filched
	 * resides in the stiloe_dicto structure.
	 *
	 * Must be the last field since the hash table is embedded and can vary
	 * in size.
	 */
	cw_ch_t		hash;
};

struct cw_stiloe_dicto_s {
#if (defined(_LIBSTIL_DBG) || defined(_LIBSTIL_DEBUG))
	cw_uint32_t	magic;
#endif

	/* stiloe_dict this stiloe_dicto is contained in. */
	cw_stiloe_dict_t *stiloe_dict;

	cw_stilo_t	name;
	cw_stilo_t	value;
};

struct cw_stiloe_hook_s {
	cw_stiloe_t	stiloe;
	void		*data;
	void		(*exec) (void *);
	void		(*dealloc) (void *);
	cw_stiloe_t	*(*ref_iterator) (void *);
};

struct cw_stiloe_lock_s {
	cw_stiloe_t	stiloe;
	cw_mtx_t	lock;
};

struct cw_stiloe_mstate_s {
	cw_stiloe_t	stiloe;
	cw_uint32_t	accuracy;
	cw_uint32_t	point;
	cw_uint32_t	base;
};

struct cw_stiloe_number_s {
	cw_stiloe_t	stiloe;
	/* Offset in val that the "decimal point" precedes. */
	cw_uint32_t	point;
	/* Base.  Can be from 2 to 36, inclusive. */
	cw_uint32_t	base;
	/* Number of bytes that val points to. */
	cw_uint32_t	val_len;
	/* Offset of most significant non-zero digit. */
	cw_uint32_t	val_msd;
	/*
	 * The least significant digit is at val[0].  Each byte can range in
	 * value from 0 to 35, depending on the base.  This representation is
	 * not compact, but it is easy to work with.
	 */
	cw_uint8_t	*val;
};

struct cw_stiloe_string_s {
	cw_stiloe_t	stiloe;
	cw_uint32_t	iterations;	/*
					 * Used for remembering the current
					 * state of reference iteration.
					 */
	union {
		struct {
			cw_stilo_t	stilo;
			cw_uint32_t	beg_offset;
			cw_uint32_t	len;
		}	i;
		struct {
			cw_uint8_t	*str;
			cw_sint32_t	len;	/* -1 if unset. */
		}	s;
	}	e;
};


/*
 * Prototypes for private methods.
 */
/* stiloe. */
static void	stiloe_p_new(cw_stiloe_t *a_stiloe, cw_stilt_t *a_stilt,
    cw_stilot_t a_type);
static void	stiloe_p_delete(cw_stiloe_t *a_stiloe);

/* no. */
static void	stilo_p_no_print(cw_stilo_t *a_stilo, cw_sint32_t a_fd,
    cw_bool_t a_syntactic, cw_bool_t a_newline);

/* array. */
static void	stilo_p_array_new(cw_stilo_t *a_stilo, cw_stilt_t *a_stilt);
static void	stilo_p_array_delete(cw_stilo_t *a_stilo);
static cw_stiloe_t *stiloe_p_array_ref_iterate(cw_stiloe_t *a_stilo, cw_bool_t
    a_reset);
static void	stilo_p_array_cast(cw_stilo_t *a_stilo, cw_stilot_t a_type);
static void	stilo_p_array_copy(cw_stilo_t *a_to, cw_stilo_t *a_from);
static void	stilo_p_array_print(cw_stilo_t *a_stilo, cw_sint32_t a_fd,
    cw_bool_t a_syntactic, cw_bool_t a_newline);

/* boolean. */
static void	stilo_p_boolean_cast(cw_stilo_t *a_stilo, cw_stilot_t a_type);
static void	stilo_p_boolean_print(cw_stilo_t *a_stilo, cw_sint32_t a_fd,
    cw_bool_t a_syntactic, cw_bool_t a_newline);

/* condition. */
static void	stilo_p_condition_new(cw_stilo_t *a_stilo, cw_stilt_t *a_stilt);
static void	stilo_p_condition_delete(cw_stilo_t *a_stilo);
static cw_stiloe_t *stiloe_p_condition_ref_iterate(cw_stiloe_t *a_stilo,
    cw_bool_t a_reset);
static void	stilo_p_condition_cast(cw_stilo_t *a_stilo, cw_stilot_t a_type);
static void	stilo_p_condition_print(cw_stilo_t *a_stilo, cw_sint32_t a_fd,
    cw_bool_t a_syntactic, cw_bool_t a_newline);

/* dict. */
static void	stilo_p_dict_new(cw_stilo_t *a_stilo, cw_stilt_t *a_stilt);
static void	stilo_p_dict_delete(cw_stilo_t *a_stilo);
static cw_stiloe_t *stiloe_p_dict_ref_iterate(cw_stiloe_t *a_stilo, cw_bool_t
    a_reset);
static void	stilo_p_dict_cast(cw_stilo_t *a_stilo, cw_stilot_t a_type);
static void	stilo_p_dict_copy(cw_stilo_t *a_to, cw_stilo_t *a_from);
static void	stilo_p_dict_print(cw_stilo_t *a_stilo, cw_sint32_t a_fd,
    cw_bool_t a_syntactic, cw_bool_t a_newline);

/* file. */
static void	stilo_p_file_new(cw_stilo_t *a_stilo, cw_stilt_t *a_stilt);
static void	stilo_p_file_delete(cw_stilo_t *a_stilo);
static cw_stiloe_t *stiloe_p_file_ref_iterate(cw_stiloe_t *a_stilo, cw_bool_t
    a_reset);
static void	stilo_p_file_cast(cw_stilo_t *a_stilo, cw_stilot_t a_type);
static void	stilo_p_file_copy(cw_stilo_t *a_to, cw_stilo_t *a_from);
static void	stilo_p_file_print(cw_stilo_t *a_stilo, cw_sint32_t a_fd,
    cw_bool_t a_syntactic, cw_bool_t a_newline);

/* hook. */
static void	stilo_p_hook_new(cw_stilo_t *a_stilo, cw_stilt_t *a_stilt);
static void	stilo_p_hook_delete(cw_stilo_t *a_stilo);
static cw_stiloe_t *stiloe_p_hook_ref_iterate(cw_stiloe_t *a_stilo, cw_bool_t
    a_reset);
static void	stilo_p_hook_cast(cw_stilo_t *a_stilo, cw_stilot_t a_type);
static void	stilo_p_hook_copy(cw_stilo_t *a_to, cw_stilo_t *a_from);
static void	stilo_p_hook_print(cw_stilo_t *a_stilo, cw_sint32_t a_fd,
    cw_bool_t a_syntactic, cw_bool_t a_newline);

/* lock. */
static void	stilo_p_lock_new(cw_stilo_t *a_stilo, cw_stilt_t *a_stilt);
static void	stilo_p_lock_delete(cw_stilo_t *a_stilo);
static cw_stiloe_t *stiloe_p_lock_ref_iterate(cw_stiloe_t *a_stilo, cw_bool_t
    a_reset);
static void	stilo_p_lock_cast(cw_stilo_t *a_stilo, cw_stilot_t a_type);
static void	stilo_p_lock_print(cw_stilo_t *a_stilo, cw_sint32_t a_fd,
    cw_bool_t a_syntactic, cw_bool_t a_newline);

/* mark. */
static void	stilo_p_mark_cast(cw_stilo_t *a_stilo, cw_stilot_t a_type);
static void	stilo_p_mark_print(cw_stilo_t *a_stilo, cw_sint32_t a_fd,
    cw_bool_t a_syntactic, cw_bool_t a_newline);

/* mstate. */
static void	stilo_p_mstate_new(cw_stilo_t *a_stilo, cw_stilt_t *a_stilt);
static void	stilo_p_mstate_delete(cw_stilo_t *a_stilo);
static cw_stiloe_t *stiloe_p_mstate_ref_iterate(cw_stiloe_t *a_stilo, cw_bool_t
    a_reset);
static void	stilo_p_mstate_cast(cw_stilo_t *a_stilo, cw_stilot_t a_type);
static void	stilo_p_mstate_copy(cw_stilo_t *a_to, cw_stilo_t *a_from);
static void	stilo_p_mstate_print(cw_stilo_t *a_stilo, cw_sint32_t a_fd,
    cw_bool_t a_syntactic, cw_bool_t a_newline);

/* name. */
static void	stilo_p_name_new(cw_stilo_t *a_stilo, cw_stilt_t *a_stilt);
static void	stilo_p_name_delete(cw_stilo_t *a_stilo);
static void	stilo_p_name_cast(cw_stilo_t *a_stilo, cw_stilot_t a_type);
static void	stilo_p_name_copy(cw_stilo_t *a_to, cw_stilo_t *a_from);
static void	stilo_p_name_print(cw_stilo_t *a_stilo, cw_sint32_t a_fd,
    cw_bool_t a_syntactic, cw_bool_t a_newline);

/* null. */
static void	stilo_p_null_cast(cw_stilo_t *a_stilo, cw_stilot_t a_type);
static void	stilo_p_null_print(cw_stilo_t *a_stilo, cw_sint32_t a_fd,
    cw_bool_t a_syntactic, cw_bool_t a_newline);

/* number. */
static void	stilo_p_number_new(cw_stilo_t *a_stilo, cw_stilt_t *a_stilt);
static void	stilo_p_number_delete(cw_stilo_t *a_stilo);
static void	stilo_p_number_cast(cw_stilo_t *a_stilo, cw_stilot_t a_type);
static void	stilo_p_number_copy(cw_stilo_t *a_to, cw_stilo_t *a_from);
static void	stilo_p_number_print(cw_stilo_t *a_stilo, cw_sint32_t a_fd,
    cw_bool_t a_syntactic, cw_bool_t a_newline);

/* operator. */
static void	stilo_p_operator_cast(cw_stilo_t *a_stilo, cw_stilot_t a_type);
static void	stilo_p_operator_print(cw_stilo_t *a_stilo, cw_sint32_t a_fd,
    cw_bool_t a_syntactic, cw_bool_t a_newline);

/* string. */
static void	stilo_p_string_new(cw_stilo_t *a_stilo, cw_stilt_t *a_stilt);
static void	stilo_p_string_delete(cw_stilo_t *a_stilo);
static cw_stiloe_t *stiloe_p_string_ref_iterate(cw_stiloe_t *a_stiloe, cw_bool_t
    a_reset);
static void	stilo_p_string_cast(cw_stilo_t *a_stilo, cw_stilot_t a_type);
static void	stilo_p_string_copy(cw_stilo_t *a_to, cw_stilo_t *a_from);
static void	stilo_p_string_print(cw_stilo_t *a_stilo, cw_sint32_t a_fd,
    cw_bool_t a_syntactic, cw_bool_t a_newline);

/*
 * vtable setup for the various operations on stilo's that are polymorphic.
 */
typedef void		cw_stilot_new_t(cw_stilo_t *a_stilo, cw_stilt_t
    *a_stilt);
typedef void		cw_stilot_delete_t(cw_stilo_t *a_stilo);
typedef cw_stiloe_t	*cw_stilot_ref_iterate_t(cw_stiloe_t *a_stiloe,
    cw_bool_t a_reset);
typedef void		cw_stilot_cast_t(cw_stilo_t *a_stilo, cw_stilot_t
    a_type);
typedef void		cw_stilot_copy_t(cw_stilo_t *a_to, cw_stilo_t *a_from);
typedef void		cw_stilot_print_t(cw_stilo_t *a_stilo, cw_sint32_t a_fd,
    cw_bool_t a_syntactic, cw_bool_t a_newline);

typedef struct cw_stilot_vtable_s cw_stilot_vtable_t;
struct  cw_stilot_vtable_s {
	cw_stilot_new_t		*new_f;
	cw_stilot_delete_t	*delete_f;
	cw_stilot_ref_iterate_t	*ref_iterate_f;
	cw_stilot_cast_t	*cast_f;
	cw_stilot_copy_t	*copy_f;
	cw_stilot_print_t	*print_f;
};

/*
 * The order of these entries must correspond to the type numbering of
 * cw_stilot_t.  NULL pointers are used in entries that should never get called,
 * so that a segfault will occur if such a non-existent function is called.
 */
static cw_stilot_vtable_t stilot_vtable[] = {
	/* _CW_STILOT_NOTYPE */
	{NULL,
	 NULL,
	 NULL,
	 NULL,
	 NULL,
	 stilo_p_no_print},	/* XXX Debugging only. */

	/* _CW_STILOT_ARRAYTYPE */
	{stilo_p_array_new,
	 stilo_p_array_delete,
	 stiloe_p_array_ref_iterate,
	 stilo_p_array_cast,
	 stilo_p_array_copy,
	 stilo_p_array_print},

	/* _CW_STILOT_BOOLEANTYPE */
	{NULL,
	 NULL,
	 NULL,
	 stilo_p_boolean_cast,
	 NULL,
	 stilo_p_boolean_print},

	/* _CW_STILOT_CONDITIONTYPE */
	{stilo_p_condition_new,
	 stilo_p_condition_delete,
	 stiloe_p_condition_ref_iterate,
	 stilo_p_condition_cast,
	 NULL,
	 stilo_p_condition_print},
	
	/* _CW_STILOT_DICTTYPE */
	{stilo_p_dict_new,
	 stilo_p_dict_delete,
	 stiloe_p_dict_ref_iterate,
	 stilo_p_dict_cast,
	 stilo_p_dict_copy,
	 stilo_p_dict_print},

	/* _CW_STILOT_FILETYPE */
	{stilo_p_file_new,
	 stilo_p_file_delete,
	 stiloe_p_file_ref_iterate,
	 stilo_p_file_cast,
	 stilo_p_file_copy,
	 stilo_p_file_print},

	/* _CW_STILOT_HOOKTYPE */
	{stilo_p_hook_new,
	 stilo_p_hook_delete,
	 stiloe_p_hook_ref_iterate,
	 stilo_p_hook_cast,
	 stilo_p_hook_copy,
	 stilo_p_hook_print},

	/* _CW_STILOT_LOCKTYPE */
	{stilo_p_lock_new,
	 stilo_p_lock_delete,
	 stiloe_p_lock_ref_iterate,
	 stilo_p_lock_cast,
	 NULL,
	 stilo_p_lock_print},

	/* _CW_STILOT_MARKTYPE */
	{NULL,
	 NULL,
	 NULL,
	 stilo_p_mark_cast,
	 NULL,
	 stilo_p_mark_print},

	/* _CW_STILOT_MSTATETYPE */
	{stilo_p_mstate_new,
	 stilo_p_mstate_delete,
	 stiloe_p_mstate_ref_iterate,
	 stilo_p_mstate_cast,
	 stilo_p_mstate_copy,
	 stilo_p_mstate_print},

	/* _CW_STILOT_NAMETYPE */
	{stilo_p_name_new,
	 stilo_p_name_delete,
	 NULL,
	 stilo_p_name_cast,
	 stilo_p_name_copy,
	 stilo_p_name_print},

	/* _CW_STILOT_NULLTYPE */
	{NULL,
	 NULL,
	 NULL,
	 stilo_p_null_cast,
	 NULL,
	 stilo_p_null_print},

	/* _CW_STILOT_NUMBERTYPE */
	{stilo_p_number_new,
	 stilo_p_number_delete,
	 NULL,
	 stilo_p_number_cast,
	 stilo_p_number_copy,	/* XXX Same as dup. */
	 stilo_p_number_print},

	/* _CW_STILOT_OPERATORTYPE */
	{NULL,
	 NULL,
	 NULL,
	 stilo_p_operator_cast,
	 NULL,
	 stilo_p_operator_print},

	/* _CW_STILOT_STRINGTYPE */
	{stilo_p_string_new,
	 stilo_p_string_delete,
	 stiloe_p_string_ref_iterate,
	 stilo_p_string_cast,
	 stilo_p_string_copy,
	 stilo_p_string_print}
};

/*
 * stilo.
 */
void
stilo_new(cw_stilo_t *a_stilo, cw_stilt_t *a_stilt, cw_stilot_t a_type)
{
	_cw_check_ptr(a_stilo);
	_cw_assert((a_stilt != NULL) || (stilot_vtable[a_type].new_f == NULL));

	memset(a_stilo, 0, sizeof(cw_stilo_t));
	a_stilo->type = a_type;

	/* Only composite types require additional initialization. */
	if (stilot_vtable[a_type].new_f != NULL)
		stilot_vtable[a_type].new_f(a_stilo, a_stilt);

#ifdef _LIBSTIL_DBG
	a_stilo->magic = _CW_STILO_MAGIC;
#endif
}

void
stilo_delete(cw_stilo_t *a_stilo)
{
	_cw_check_ptr(a_stilo);
	_cw_assert(a_stilo->magic == _CW_STILO_MAGIC);

	/*
	 * Delete extended types if they only have one reference.  Otherwise,
	 * the GC is responsible for determining when an object can be deleted.
	 */
	if ((a_stilo->ref_count == 0) && (stilot_vtable[a_stilo->type].delete_f
	    != NULL))
		stilot_vtable[a_stilo->type].delete_f(a_stilo);

#ifdef _LIBSTIL_DBG
	a_stilo->magic = _CW_STILO_MAGIC;
#endif
}

cw_stilot_t
stilo_type_get(cw_stilo_t *a_stilo)
{
	_cw_check_ptr(a_stilo);
	_cw_assert(a_stilo->magic == _CW_STILO_MAGIC);

	return a_stilo->type;
}

cw_bool_t
stilo_executable_get(cw_stilo_t *a_stilo)
{
	_cw_check_ptr(a_stilo);
	_cw_assert(a_stilo->magic == _CW_STILO_MAGIC);

	return a_stilo->executable;
}

void
stilo_executable_set(cw_stilo_t *a_stilo, cw_bool_t a_executable)
{
	_cw_check_ptr(a_stilo);
	_cw_assert(a_stilo->magic == _CW_STILO_MAGIC);

	a_stilo->executable = a_executable;
}

cw_bool_t
stilo_literal_get(cw_stilo_t *a_stilo)
{
	_cw_check_ptr(a_stilo);
	_cw_assert(a_stilo->magic == _CW_STILO_MAGIC);

	return a_stilo->literal;
}

void
stilo_literal_set(cw_stilo_t *a_stilo, cw_bool_t a_literal)
{
	_cw_check_ptr(a_stilo);
	_cw_assert(a_stilo->magic == _CW_STILO_MAGIC);

	a_stilo->literal = a_literal;
}

void
stilo_cast(cw_stilo_t *a_stilo, cw_stilot_t a_stilot)
{
	_cw_check_ptr(a_stilo);
	_cw_assert(a_stilo->magic == _CW_STILO_MAGIC);

	stilot_vtable[a_stilo->type].cast_f(a_stilo, a_stilot);
}

void
stilo_copy(cw_stilo_t *a_to, cw_stilo_t *a_from)
{
	_cw_check_ptr(a_to);
	_cw_assert(a_to->magic == _CW_STILO_MAGIC);
	_cw_check_ptr(a_from);
	_cw_assert(a_from->magic == _CW_STILO_MAGIC);

	stilot_vtable[a_from->type].copy_f(a_to, a_from);
}

void
stilo_dup(cw_stilo_t *a_to, cw_stilo_t *a_from)
{
	cw_stilot_t	type;

	_cw_check_ptr(a_to);
	_cw_assert(a_to->magic == _CW_STILO_MAGIC);
	_cw_check_ptr(a_from);
	_cw_assert(a_from->magic == _CW_STILO_MAGIC);

	/* Clean up before stomping on the destination stilo. */
	stilo_delete(a_to);

	/* Mark the stilo as invalid. */
	type = a_from->type;
	a_from->type = _CW_STILOT_NOTYPE;

	/* Copy. */
	memcpy(a_to, a_from, sizeof(cw_stilo_t));

	/*
	 * Overflow the reference count, if necessary.  The order of events here
	 * is critical to thread safety (the GC can get screwed up if we do this
	 * in the wrong order).
	 */
	switch (type) {
	case _CW_STILOT_ARRAYTYPE:
	case _CW_STILOT_CONDITIONTYPE:
	case _CW_STILOT_DICTTYPE:
	case _CW_STILOT_HOOKTYPE:
	case _CW_STILOT_LOCKTYPE:
	case _CW_STILOT_STRINGTYPE:
		if (a_from->ref_count == 0) {
			/* Overflow the reference count. */
			a_from->ref_count = 1;
			a_to->ref_count = 1;

			/* XXX Add the stiloe to the GC's sequence set. */
		}
		break;
	case _CW_STILOT_NUMBERTYPE:
		if (a_from->extended) {
			/* XXX Create a duplicate stiloe. */
		}
		break;
	case _CW_STILOT_NOTYPE:
	case _CW_STILOT_BOOLEANTYPE:
	case _CW_STILOT_FILETYPE:
	case _CW_STILOT_MARKTYPE:
	case _CW_STILOT_MSTATETYPE:
	case _CW_STILOT_NAMETYPE:
	case _CW_STILOT_NULLTYPE:
	case _CW_STILOT_OPERATORTYPE:
		break;
	default:
		_cw_error("Programming error");
	}

	/* Reset debug flags on new copy. */
	a_to->breakpoint = FALSE;
	a_to->watchpoint = FALSE;

	/* Mark the the stilo's as valid again. */
	a_from->type = type;
	a_to->type = type;
}

void
stilo_move(cw_stilo_t *a_to, cw_stilo_t *a_from)
{
	_cw_check_ptr(a_to);
	_cw_assert(a_to->magic == _CW_STILO_MAGIC);
	_cw_check_ptr(a_from);
	_cw_assert(a_from->magic == _CW_STILO_MAGIC);

	/* Clean up before stomping on the destination stilo. */
	stilo_delete(a_to);
	
	memcpy(a_to, a_from, sizeof(cw_stilo_t));

	/* Reset the source stilo. */
	stilo_new(a_from, NULL, _CW_STILOT_NOTYPE);
}

void
stilo_print(cw_stilo_t *a_stilo, cw_sint32_t a_fd, cw_bool_t a_syntactic,
    cw_bool_t a_newline)
{
	stilot_vtable[a_stilo->type].print_f(a_stilo, a_fd, a_syntactic,
	    a_newline);
}

cw_stiloe_t *
stilo_l_stiloe_get(cw_stilo_t *a_stilo)
{
	cw_stiloe_t	*retval;

	_cw_check_ptr(a_stilo);
	_cw_assert(a_stilo->magic == _CW_STILO_MAGIC);

	switch (a_stilo->type) {
	case _CW_STILOT_ARRAYTYPE:
	case _CW_STILOT_CONDITIONTYPE:
	case _CW_STILOT_DICTTYPE:
	case _CW_STILOT_HOOKTYPE:
	case _CW_STILOT_LOCKTYPE:
	case _CW_STILOT_MSTATETYPE:
	case _CW_STILOT_STRINGTYPE:
		retval = a_stilo->o.stiloe;
		break;
	default:
		retval = NULL;
	}

	return retval;
}

/*
 * stiloe.
 */
static void
stiloe_p_new(cw_stiloe_t *a_stiloe, cw_stilt_t *a_stilt, cw_stilot_t a_type)
{
	/* Initialize the common section. */
	memset(a_stiloe, 0, sizeof(cw_stiloe_t));

	a_stiloe->type = a_type;
	a_stiloe->stilt = a_stilt;

#ifdef _LIBSTIL_DBG
	a_stiloe->magic = _CW_STILOE_MAGIC;
#endif
}

static void
stiloe_p_delete(cw_stiloe_t *a_stiloe)
{
	_cw_stilt_free(a_stiloe->stilt, a_stiloe);
}

cw_stiloe_t *
stiloe_l_ref_iterate(cw_stiloe_t *a_stiloe, cw_bool_t a_reset)
{
	cw_stiloe_t	*retval;

	_cw_check_ptr(a_stiloe);
	_cw_assert(a_stiloe->magic == _CW_STILOE_MAGIC);

	retval = stilot_vtable[a_stiloe->type].ref_iterate_f(a_stiloe, a_reset);

	return retval;
}

/*
 * no.
 */
static void
stilo_p_no_print(cw_stilo_t *a_stilo, cw_sint32_t a_fd, cw_bool_t a_syntactic,
    cw_bool_t a_newline)
{
	cw_uint8_t	newline = (a_newline) ? '\n' : '\0';

	if (a_syntactic)
		_cw_out_put_f(a_fd, "-notype-[c]", newline);
	else
		_cw_out_put_f(a_fd, "--nostringval--[c]", newline);
}

/*
 * array.
 */
static void
stilo_p_array_new(cw_stilo_t *a_stilo, cw_stilt_t *a_stilt)
{
	cw_stiloe_array_t	*array;

	array = (cw_stiloe_array_t *)_cw_stilt_malloc(a_stilt,
	    sizeof(cw_stiloe_array_t));

	array->iterations = 0;
	array->e.a.arr = NULL;
	array->e.a.len = -1;

	a_stilo->o.stiloe = (cw_stiloe_t *)array;

	stiloe_p_new(a_stilo->o.stiloe, a_stilt, _CW_STILOT_ARRAYTYPE);
}

static void
stilo_p_array_delete(cw_stilo_t *a_stilo)
{
	cw_stiloe_array_t	*array;
	cw_uint32_t		i;

	array = (cw_stiloe_array_t *)a_stilo->o.stiloe;
	_cw_check_ptr(array);
	_cw_assert(array->stiloe.magic == _CW_STILOE_MAGIC);

	if ((array->stiloe.indirect == FALSE) && (array->e.a.len != -1)) {
		for (i = 0; i < array->e.a.len; i++)
			stilo_delete(&array->e.a.arr[i]);
	}
	_cw_stilt_free(array->stiloe.stilt, array->e.a.arr);

	stiloe_p_delete(&array->stiloe);
}

static cw_stiloe_t *
stiloe_p_array_ref_iterate(cw_stiloe_t *a_stiloe, cw_bool_t a_reset)
{
	cw_stiloe_t		*retval;
	cw_stiloe_array_t	*array;

	array = (cw_stiloe_array_t *)a_stiloe;

	if (a_reset)
		array->iterations = 0;

	if (a_stiloe->indirect == FALSE)
		retval = NULL;
	else if (array->iterations < array->e.a.len) {
		retval = array->e.i.stilo.o.stiloe;
		array->iterations++;
	} else {
		retval = NULL;
		array->iterations = 0;
	}

	return retval;
}

static void
stilo_p_array_cast(cw_stilo_t *a_stilo, cw_stilot_t a_type)
{
}

static void
stilo_p_array_copy(cw_stilo_t *a_to, cw_stilo_t *a_from)
{
}

static void
stilo_p_array_print(cw_stilo_t *a_stilo, cw_sint32_t a_fd, cw_bool_t
    a_syntactic, cw_bool_t a_newline)
{
	cw_uint8_t	newline = (a_newline) ? '\n' : '\0';
	cw_stilo_t	*arr;
	cw_uint32_t	nelms, i;
	cw_bool_t	executable;

	executable = stilo_executable_get(a_stilo);
	arr = stilo_array_get(a_stilo);
	nelms = stilo_array_len_get(a_stilo);

	if (a_syntactic) {
		if (executable)
			_cw_out_put_f(a_fd, "{");
		else
			_cw_out_put_f(a_fd, "[[");
		for (i = 0; i < nelms; i++) {
			stilo_print(&arr[i], a_fd, a_syntactic, FALSE);
			if (i < nelms - 1)
				_cw_out_put_f(a_fd, " ");
		}
		if (executable)
			_cw_out_put_f(a_fd, "}[c]", newline);
		else
			_cw_out_put_f(a_fd, "][c]", newline);
	} else
		_cw_out_put_f(a_fd, "--nostringval--[c]", newline);
}

cw_sint32_t
stilo_array_len_get(cw_stilo_t *a_stilo)
{
	cw_sint32_t		retval;
	cw_stiloe_array_t	*array;

	_cw_check_ptr(a_stilo);
	_cw_assert(a_stilo->magic == _CW_STILO_MAGIC);

	array = (cw_stiloe_array_t *)a_stilo->o.stiloe;
	_cw_assert(array->stiloe.magic == _CW_STILOE_MAGIC);
	_cw_assert(array->stiloe.type == _CW_STILOT_ARRAYTYPE);

	if (array->stiloe.indirect == FALSE)
		retval = array->e.a.len;
	else
		retval = array->e.i.len;

	return retval;
}

void
stilo_array_len_set(cw_stilo_t *a_stilo, cw_uint32_t a_len)
{
	cw_stiloe_array_t	*array;
	cw_uint32_t		i;

	_cw_check_ptr(a_stilo);
	_cw_assert(a_stilo->magic == _CW_STILO_MAGIC);
	_cw_assert(a_stilo->type == _CW_STILOT_ARRAYTYPE);
	
	array = (cw_stiloe_array_t *)a_stilo->o.stiloe;
	_cw_assert(array->stiloe.magic == _CW_STILOE_MAGIC);
	_cw_assert(array->stiloe.indirect == FALSE);
	_cw_assert(array->e.a.len == -1);

	if (a_len > 0) {
		array->e.a.arr = (cw_stilo_t
		    *)_cw_stilt_malloc(array->stiloe.stilt, sizeof(cw_stilo_t) *
		    a_len);
		for (i = 0; i < a_len; i++)
			stilo_new(&array->e.a.arr[i], NULL, _CW_STILOT_NOTYPE);
	}
	array->e.a.len = a_len;
}

cw_stilo_t *
stilo_array_el_get(cw_stilo_t *a_stilo, cw_uint32_t a_offset)
{
	cw_stilo_t		*retval;
	cw_stiloe_array_t	*array;

	_cw_check_ptr(a_stilo);
	_cw_assert(a_stilo->magic == _CW_STILO_MAGIC);
	_cw_assert(a_stilo->type == _CW_STILOT_ARRAYTYPE);

	array = (cw_stiloe_array_t *)a_stilo->o.stiloe;
	_cw_assert(array->stiloe.magic == _CW_STILOE_MAGIC);

	if (array->stiloe.indirect == FALSE) {
		_cw_assert(array->e.a.len > a_offset);
		retval = &array->e.a.arr[a_offset];
	} else {
		retval = &stilo_array_el_get(&array->e.i.stilo,
		    a_offset)[array->e.i.beg_offset];
	}

	return retval;
}

cw_stilo_t *
stilo_array_get(cw_stilo_t *a_stilo)
{
	cw_stilo_t		*retval;
	cw_stiloe_array_t	*array;

	_cw_check_ptr(a_stilo);
	_cw_assert(a_stilo->magic == _CW_STILO_MAGIC);
	_cw_assert(a_stilo->type == _CW_STILOT_ARRAYTYPE);

	array = (cw_stiloe_array_t *)a_stilo->o.stiloe;
	_cw_assert(array->stiloe.magic == _CW_STILOE_MAGIC);

	if (array->stiloe.indirect == FALSE) {
		_cw_assert(array->e.a.len != -1);
		retval = array->e.a.arr;
	} else {
		retval = &stilo_array_get(&array->e.i.stilo)
		    [array->e.i.beg_offset];
	}

	return retval;
}

void
stilo_array_set(cw_stilo_t *a_stilo, cw_uint32_t a_offset, cw_stilo_t *a_arr,
    cw_uint32_t a_len)
{
	cw_stiloe_array_t	*array;
	cw_stilo_t		*arr;
	cw_uint32_t		i;

	_cw_check_ptr(a_stilo);
	_cw_assert(a_stilo->magic == _CW_STILO_MAGIC);
	_cw_assert(a_stilo->type == _CW_STILOT_ARRAYTYPE);

	array = (cw_stiloe_array_t *)a_stilo->o.stiloe;
	_cw_assert(array->stiloe.magic == _CW_STILOE_MAGIC);

	/* Get the array pointer. */
	if (array->stiloe.indirect == FALSE) {
		_cw_assert(array->e.a.len >= a_len);
		arr = array->e.a.arr;
	} else {
		_cw_assert(array->e.i.len > a_len);
		arr = &stilo_array_get(&array->e.i.stilo)
		    [array->e.i.beg_offset];
	}

	/* Set the array. */
	for (i = 0; i < a_len; i++)
		stilo_copy(&arr[i], &a_arr[i]);
}

/*
 * boolean.
 */
static void
stilo_p_boolean_cast(cw_stilo_t *a_stilo, cw_stilot_t a_type)
{
}

static void
stilo_p_boolean_print(cw_stilo_t *a_stilo, cw_sint32_t a_fd, cw_bool_t
    a_syntactic, cw_bool_t a_newline)
{
	cw_uint8_t	newline = (a_newline) ? '\n' : '\0';

	if (a_stilo->o.boolean.val)
		_cw_out_put_f(a_fd, "true[c]", newline);
	else
		_cw_out_put_f(a_fd, "false[c]", newline);
}

/*
 * condition.
 */
static void
stilo_p_condition_new(cw_stilo_t *a_stilo, cw_stilt_t *a_stilt)
{
}

static void
stilo_p_condition_delete(cw_stilo_t *a_stilo)
{
}

static cw_stiloe_t *
stiloe_p_condition_ref_iterate(cw_stiloe_t *a_stilo, cw_bool_t a_reset)
{
	return NULL;	/* XXX */
}

static void
stilo_p_condition_cast(cw_stilo_t *a_stilo, cw_stilot_t a_type)
{
}

static void
stilo_p_condition_print(cw_stilo_t *a_stilo, cw_sint32_t a_fd, cw_bool_t
    a_syntactic, cw_bool_t a_newline)
{
	cw_uint8_t	newline = (a_newline) ? '\n' : '\0';

	if (a_syntactic)
		_cw_out_put_f(a_fd, "-condition-[c]", newline);
	else
		_cw_out_put_f(a_fd, "--nostringval--[c]", newline);
}

/*
 * dict.
 */
static void
stilo_p_dict_new(cw_stilo_t *a_stilo, cw_stilt_t *a_stilt)
{
}

static void
stilo_p_dict_delete(cw_stilo_t *a_stilo)
{
}

static cw_stiloe_t *
stiloe_p_dict_ref_iterate(cw_stiloe_t *a_stilo, cw_bool_t a_reset)
{
	return NULL;	/* XXX */
}

static void
stilo_p_dict_cast(cw_stilo_t *a_stilo, cw_stilot_t a_type)
{
}

static void
stilo_p_dict_copy(cw_stilo_t *a_to, cw_stilo_t *a_from)
{
}

static void
stilo_p_dict_print(cw_stilo_t *a_stilo, cw_sint32_t a_fd, cw_bool_t a_syntactic,
    cw_bool_t a_newline)
{
	cw_uint8_t	newline = (a_newline) ? '\n' : '\0';
	
	if (a_syntactic)
		_cw_out_put_f(a_fd, "-dict-[c]", newline);
	else
		_cw_out_put_f(a_fd, "--nostringval--[c]", newline);
}

cw_bool_t
stilo_dict_def(cw_stilo_t *a_stilo, cw_stilo_t *a_key, cw_stilo_t *a_val)
{
	return TRUE; /* XXX */
}

cw_bool_t
stilo_dict_undef(cw_stilo_t *a_stilo, cw_stilo_t *a_key)
{
	return TRUE; /* XXX */
}

cw_stilo_t *
stilo_dict_lookup(cw_stilo_t *a_stilo, cw_stilo_t *a_key)
{
	return NULL; /* XXX */
}

cw_bool_t
stilo_dict_copy(cw_stilo_t *a_from, cw_stilo_t *a_to)
{
	return TRUE; /* XXX */
}

cw_uint32_t
stilo_dict_count(cw_stilo_t *a_stilo)
{
	return 0xffffffff; /* XXX */
}

cw_stilo_t *
stilo_dict_iterate(cw_stilo_t *a_stilo)
{
	return NULL; /* XXX */
}

/*
 * file.
 */
static void
stilo_p_file_new(cw_stilo_t *a_stilo, cw_stilt_t *a_stilt)
{
}

static void
stilo_p_file_delete(cw_stilo_t *a_stilo)
{
}

static cw_stiloe_t *
stiloe_p_file_ref_iterate(cw_stiloe_t *a_stilo, cw_bool_t a_reset)
{
	return NULL;	/* XXX */
}

static void
stilo_p_file_cast(cw_stilo_t *a_stilo, cw_stilot_t a_type)
{
}

static void
stilo_p_file_copy(cw_stilo_t *a_to, cw_stilo_t *a_from)
{
}

static void
stilo_p_file_print(cw_stilo_t *a_stilo, cw_sint32_t a_fd, cw_bool_t a_syntactic,
    cw_bool_t a_newline)
{
	cw_uint8_t	newline = (a_newline) ? '\n' : '\0';
	
	if (a_syntactic)
		_cw_out_put_f(a_fd, "-file-[c]", newline);
	else
		_cw_out_put_f(a_fd, "--nostringval--[c]", newline);
}

/*
 * hook.
 */
static void
stilo_p_hook_new(cw_stilo_t *a_stilo, cw_stilt_t *a_stilt)
{
}

static void
stilo_p_hook_delete(cw_stilo_t *a_stilo)
{
}

static cw_stiloe_t *
stiloe_p_hook_ref_iterate(cw_stiloe_t *a_stilo, cw_bool_t a_reset)
{
	return NULL;	/* XXX */
}

static void
stilo_p_hook_cast(cw_stilo_t *a_stilo, cw_stilot_t a_type)
{
}

static void
stilo_p_hook_copy(cw_stilo_t *a_to, cw_stilo_t *a_from)
{
}

static void
stilo_p_hook_print(cw_stilo_t *a_stilo, cw_sint32_t a_fd, cw_bool_t a_syntactic,
    cw_bool_t a_newline)
{
	cw_uint8_t	newline = (a_newline) ? '\n' : '\0';
	
	if (a_syntactic)
		_cw_out_put_f(a_fd, "-hook-[c]", newline);
	else
		_cw_out_put_f(a_fd, "--nostringval--[c]", newline);
}

/*
 * lock.
 */
static void
stilo_p_lock_new(cw_stilo_t *a_stilo, cw_stilt_t *a_stilt)
{
}

static void
stilo_p_lock_delete(cw_stilo_t *a_stilo)
{
}

static cw_stiloe_t *
stiloe_p_lock_ref_iterate(cw_stiloe_t *a_stilo, cw_bool_t a_reset)
{
	return NULL;	/* XXX */
}

static void
stilo_p_lock_cast(cw_stilo_t *a_stilo, cw_stilot_t a_type)
{
}

static void
stilo_p_lock_print(cw_stilo_t *a_stilo, cw_sint32_t a_fd, cw_bool_t a_syntactic,
    cw_bool_t a_newline)
{
	cw_uint8_t	newline = (a_newline) ? '\n' : '\0';
	
	if (a_syntactic)
		_cw_out_put_f(a_fd, "-lock-[c]", newline);
	else
		_cw_out_put_f(a_fd, "--nostringval--[c]", newline);
}

/*
 * mark.
 */
static void
stilo_p_mark_cast(cw_stilo_t *a_stilo, cw_stilot_t a_type)
{
}

static void
stilo_p_mark_print(cw_stilo_t *a_stilo, cw_sint32_t a_fd, cw_bool_t a_syntactic,
    cw_bool_t a_newline)
{
	cw_uint8_t	newline = (a_newline) ? '\n' : '\0';
	
	if (a_syntactic)
		_cw_out_put_f(a_fd, "-mark-[c]", newline);
	else
		_cw_out_put_f(a_fd, "--nostringval--[c]", newline);
}

/*
 * mstate.
 */
static void
stilo_p_mstate_new(cw_stilo_t *a_stilo, cw_stilt_t *a_stilt)
{
}

static void
stilo_p_mstate_delete(cw_stilo_t *a_stilo)
{
}

static cw_stiloe_t *
stiloe_p_mstate_ref_iterate(cw_stiloe_t *a_stilo, cw_bool_t a_reset)
{
	return NULL;	/* XXX */
}

static void
stilo_p_mstate_cast(cw_stilo_t *a_stilo, cw_stilot_t a_type)
{
}

static void
stilo_p_mstate_copy(cw_stilo_t *a_to, cw_stilo_t *a_from)
{
}

static void
stilo_p_mstate_print(cw_stilo_t *a_stilo, cw_sint32_t a_fd, cw_bool_t
    a_syntactic, cw_bool_t a_newline)
{
	cw_uint8_t	newline = (a_newline) ? '\n' : '\0';
	
	if (a_syntactic)
		_cw_out_put_f(a_fd, "-mstate-[c]", newline);
	else
		_cw_out_put_f(a_fd, "--nostringval--[c]", newline);
}

/*
 * name.
 */
static void
stilo_p_name_new(cw_stilo_t *a_stilo, cw_stilt_t *a_stilt)
{
}

static void
stilo_p_name_delete(cw_stilo_t *a_stilo)
{
	if (a_stilo->indirect_name)
		stiltn_unref(a_stilo->o.name.s.stilt, a_stilo->o.name.stiln);
	else {
		stil_stiln_unref(stilt_get_stil(a_stilo->o.name.s.stilt),
		    a_stilo->o.name.stiln, a_stilo->o.name.s.key);
	}
}

static void
stilo_p_name_cast(cw_stilo_t *a_stilo, cw_stilot_t a_type)
{
}

static void
stilo_p_name_copy(cw_stilo_t *a_to, cw_stilo_t *a_from)
{
}

static void
stilo_p_name_print(cw_stilo_t *a_stilo, cw_sint32_t a_fd, cw_bool_t
    a_syntactic, cw_bool_t a_newline)
{
	cw_uint8_t	newline = (a_newline) ? '\n' : '\0';
	
	if (a_syntactic)
		_cw_out_put_f(a_fd, "-XXX name-[c]", newline);
	else
		_cw_out_put_f(a_fd, "/-XXX name-[c]", newline);
}

/*
 * null.
 */
static void
stilo_p_null_cast(cw_stilo_t *a_stilo, cw_stilot_t a_type)
{
}

static void
stilo_p_null_print(cw_stilo_t *a_stilo, cw_sint32_t a_fd, cw_bool_t a_syntactic,
    cw_bool_t a_newline)
{
	cw_uint8_t	newline = (a_newline) ? '\n' : '\0';
	
	if (a_syntactic)
		_cw_out_put_f(a_fd, "null[c]", newline);
	else
		_cw_out_put_f(a_fd, "--nostringval--[c]", newline);
}

/*
 * number.
 */
static void
stilo_p_number_new(cw_stilo_t *a_stilo, cw_stilt_t *a_stilt)
{
}

static void
stilo_p_number_delete(cw_stilo_t *a_stilo)
{
}

static void
stilo_p_number_cast(cw_stilo_t *a_stilo, cw_stilot_t a_type)
{
}

static void
stilo_p_number_copy(cw_stilo_t *a_to, cw_stilo_t *a_from)
{
}

static void
stilo_p_number_print(cw_stilo_t *a_stilo, cw_sint32_t a_fd, cw_bool_t
    a_syntactic, cw_bool_t a_newline)
{
	cw_uint8_t	newline = (a_newline) ? '\n' : '\0';
	
	if (a_syntactic)
		_cw_out_put_f(a_fd, "-XXX number-[c]", newline);
	else
		_cw_out_put_f(a_fd, "-XXX number-[c]", newline);
}

/*
 * operator.
 */
static void
stilo_p_operator_cast(cw_stilo_t *a_stilo, cw_stilot_t a_type)
{
}

static void
stilo_p_operator_print(cw_stilo_t *a_stilo, cw_sint32_t a_fd, cw_bool_t
    a_syntactic, cw_bool_t a_newline)
{
	cw_uint8_t	newline = (a_newline) ? '\n' : '\0';
	
	if (a_syntactic)
		_cw_out_put_f(a_fd, "-operator-[c]", newline);
	else
		_cw_out_put_f(a_fd, "--nostringval--[c]", newline);
}

/*
 * string.
 */
static void
stilo_p_string_new(cw_stilo_t *a_stilo, cw_stilt_t *a_stilt)
{
	cw_stiloe_string_t	*string;

	string = (cw_stiloe_string_t *)_cw_stilt_malloc(a_stilt,
	    sizeof(cw_stiloe_string_t));

	string->iterations = 0;
	string->e.s.str = NULL;
	string->e.s.len = -1;

	a_stilo->o.stiloe = (cw_stiloe_t *)string;

	stiloe_p_new(a_stilo->o.stiloe, a_stilt, _CW_STILOT_STRINGTYPE);
}

static void
stilo_p_string_delete(cw_stilo_t *a_stilo)
{
	cw_stiloe_string_t	*string;

	string = (cw_stiloe_string_t *)a_stilo->o.stiloe;
	_cw_check_ptr(string);
	_cw_assert(string->stiloe.magic == _CW_STILOE_MAGIC);

	if ((string->stiloe.indirect == FALSE) && (string->e.s.len != -1))
		_cw_stilt_free(string->stiloe.stilt, string->e.s.str);

	stiloe_p_delete(&string->stiloe);
}

static cw_stiloe_t *
stiloe_p_string_ref_iterate(cw_stiloe_t *a_stiloe, cw_bool_t a_reset)
{
	cw_stiloe_t		*retval;
	cw_stiloe_string_t	*string;

	string = (cw_stiloe_string_t *)a_stiloe;

	if (a_reset)
		string->iterations = 0;

	if (a_stiloe->indirect == FALSE)
		retval = NULL;
	else if (string->iterations == 0) {
		retval = string->e.i.stilo.o.stiloe;
		string->iterations++;
	} else {
		retval = NULL;
		string->iterations = 0;
	}

	return retval;
}

static void
stilo_p_string_cast(cw_stilo_t *a_stilo, cw_stilot_t a_type)
{
}

static void
stilo_p_string_copy(cw_stilo_t *a_to, cw_stilo_t *a_from)
{
}

static void
stilo_p_string_print(cw_stilo_t *a_stilo, cw_sint32_t a_fd, cw_bool_t
    a_syntactic, cw_bool_t a_newline)
{
	cw_uint8_t	newline = (a_newline) ? '\n' : '\0';
	cw_uint8_t	*str;
	cw_sint32_t	len;

	str = stilo_string_get(a_stilo);
	len = stilo_string_len_get(a_stilo);

	if (a_syntactic)
		_cw_out_put_f(a_fd, "(");
	if (len > 0)
		_cw_out_put_fn(a_fd, len, "[s]", str);
	if (a_syntactic)
		_cw_out_put_f(a_fd, ")");
	_cw_out_put_f(a_fd, "[c]", newline);
}

cw_sint32_t
stilo_string_len_get(cw_stilo_t *a_stilo)
{
	cw_sint32_t		retval;
	cw_stiloe_string_t	*string;

	_cw_check_ptr(a_stilo);
	_cw_assert(a_stilo->magic == _CW_STILO_MAGIC);
	_cw_assert(a_stilo->type == _CW_STILOT_STRINGTYPE);

	string = (cw_stiloe_string_t *)a_stilo->o.stiloe;
	_cw_assert(string->stiloe.magic == _CW_STILOE_MAGIC);

	if (string->stiloe.indirect == FALSE)
		retval = string->e.s.len;
	else
		retval = string->e.i.len;

	return retval;
}

void
stilo_string_len_set(cw_stilo_t *a_stilo, cw_uint32_t a_len)
{
	cw_stiloe_string_t	*string;

	_cw_check_ptr(a_stilo);
	_cw_assert(a_stilo->magic == _CW_STILO_MAGIC);
	_cw_assert(a_stilo->type == _CW_STILOT_STRINGTYPE);

	string = (cw_stiloe_string_t *)a_stilo->o.stiloe;
	_cw_assert(string->stiloe.magic == _CW_STILOE_MAGIC);
	_cw_assert(string->stiloe.indirect == FALSE);
	_cw_assert(string->e.s.len == -1);

	if (a_len > 0) {
		string->e.s.str = (cw_uint8_t
		    *)_cw_stilt_malloc(string->stiloe.stilt, a_len);
	}
	string->e.s.len = a_len;
}

cw_uint8_t *
stilo_string_el_get(cw_stilo_t *a_stilo, cw_uint32_t a_offset)
{
	cw_uint8_t		*retval;
	cw_stiloe_string_t	*string;

	_cw_check_ptr(a_stilo);
	_cw_assert(a_stilo->magic == _CW_STILO_MAGIC);
	_cw_assert(a_stilo->type == _CW_STILOT_STRINGTYPE);

	string = (cw_stiloe_string_t *)a_stilo->o.stiloe;
	_cw_assert(string->stiloe.magic == _CW_STILOE_MAGIC);

	if (string->stiloe.indirect == FALSE) {
		_cw_assert(string->e.s.len > a_offset);
		retval = &string->e.s.str[a_offset];
	} else {
		retval = &stilo_string_el_get(&string->e.i.stilo,
		    a_offset)[string->e.i.beg_offset];
	}

	return retval;
}

cw_uint8_t *
stilo_string_get(cw_stilo_t *a_stilo)
{
	cw_uint8_t		*retval;
	cw_stiloe_string_t	*string;

	_cw_check_ptr(a_stilo);
	_cw_assert(a_stilo->magic == _CW_STILO_MAGIC);
	_cw_assert(a_stilo->type == _CW_STILOT_STRINGTYPE);

	string = (cw_stiloe_string_t *)a_stilo->o.stiloe;
	_cw_assert(string->stiloe.magic == _CW_STILOE_MAGIC);

	if (string->stiloe.indirect == FALSE) {
		_cw_assert(string->e.s.len != -1);
		retval = string->e.s.str;
	} else {
		retval = &stilo_string_get(&string->e.i.stilo)
		    [string->e.i.beg_offset];
	}

	return retval;
}

void
stilo_string_set(cw_stilo_t *a_stilo, cw_uint32_t a_offset, const cw_uint8_t
    *a_str, cw_uint32_t a_len)
{
	cw_stiloe_string_t	*string;
	cw_uint8_t		*str;

	_cw_check_ptr(a_stilo);
	_cw_assert(a_stilo->magic == _CW_STILO_MAGIC);
	_cw_assert(a_stilo->type == _CW_STILOT_STRINGTYPE);

	string = (cw_stiloe_string_t *)a_stilo->o.stiloe;
	_cw_assert(string->stiloe.magic == _CW_STILOE_MAGIC);

	/* Get the string pointer. */
	if (string->stiloe.indirect == FALSE) {
		_cw_assert(string->e.s.len >= a_len);
		str = string->e.s.str;
	} else {
		_cw_assert(string->e.i.len >= a_len);
		str = &stilo_string_get(&string->e.i.stilo)
		    [string->e.i.beg_offset];
	}

	/* Set the string. */
	memcpy(str, a_str, a_len);
}
