/******************************************************************************
 *
 * <Copyright = "jasone">
 * <License>
 *
 ******************************************************************************
 *
 * Version: <Version>
 *
 ******************************************************************************/

#include "../include/libstil/libstil.h"

#include <stdarg.h>
#include <ctype.h>

#ifdef _LIBSTIL_DBG
#define _CW_STILO_MAGIC		0x398754ba
#define _CW_STILOE_MAGIC	0x0fa6e798
#endif

typedef struct cw_stiloe_array_s cw_stiloe_array_t;
typedef struct cw_stiloe_condition_s cw_stiloe_condition_t;
typedef struct cw_stiloe_hook_s cw_stiloe_hook_t;
typedef struct cw_stiloe_lock_s cw_stiloe_lock_t;
typedef struct cw_stiloe_mstate_s cw_stiloe_mstate_t;
typedef struct cw_stiloe_name_s cw_stiloe_name_t;
typedef struct cw_stiloe_number_s cw_stiloe_number_t;
typedef struct cw_stiloe_string_s cw_stiloe_string_t;

/*
 * All extended type objects contain a stiloe.  This provides a poor man's
 * inheritance.  Since stil's type system is static, this idiom is adequate.
 */
struct cw_stiloe_s {
#ifdef _LIBSTIL_DBG
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
	 * Modifications must be locked if this is a globally allocated
	 * object.
	 */
	cw_mtx_t	lock;
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
	cw_uint32_t	iterations;	/*
					 * Used for remembering the current
					 * state of reference iteration.
					 */

	union {
		struct {
			cw_stilo_t	stilo;
		}	i;
		struct {
			/*
			 * If non-NULL, the previous reference iteration
			 * returned the key of this dicto, so the value of this
			 * dicto is the next reference to check.
			 */
			cw_stiloe_dicto_t *dicto;
			/*
			 * Initial capacity.  This is only needed when copying
			 * the dict.
			 */
			cw_uint32_t	capacity;
			/*
			 * Name/value pairs.  The keys are (cw_stilo_t *), and
			 * the values are * (cw_stiloe_dicto_t *).  The stilo
			 * that the key points to resides in * the stiloe_dicto
			 * (value) structure.
			 */
			cw_dch_t	hash;
		}	d;
	}	e;
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

/*
 * Size and fullness control for keyed reference hashes.  For each keyed
 * reference, there is a global dictionary with a key that corresponds to the
 * name, so in most cases, these hashes are quite small.
 */
#define _CW_STILO_NAME_KREF_TABLE	  8
#define _CW_STILO_NAME_KREF_GROW	  6
#define _CW_STILO_NAME_KREF_SHRINK	  2

struct cw_stiloe_name_s {
	cw_stiloe_t	stiloe;
	/*
	 * Always the value of the root name stiloe, regardless of whether this
	 * is a reference (local) or the root (global).  This allows fast access
	 * to what is effectively the key for name comparisions.
	 */
	cw_stiloe_name_t *val;
	union {
		/* Thread-specific (local) name.  Indirect object. */
		struct {
			cw_stilo_t	stilo;
		}	i;
		/* Root (global) name.  Direct object. */
		struct {
			/*
			 * If non-NULL, a hash of keyed references to this
			 * object.  Keyed references are used by global
			 * dictionary entries.  This allows a thread to
			 * determine whether an entry exists in a particular
			 * global dictionary without having to lock the entire
			 * dictionary.
			 */
			cw_dch_t	*keyed_refs;
			/*
			 * If TRUE, the string in the key is statically
			 * allocated, and should not be deallocated during
			 * destruction.
			 */
			cw_bool_t	is_static_name;
			/*
			 * name is *not* required to be NULL-terminated, so we
			 * keep track of the length.
			 */
			const cw_uint8_t *name;
			cw_uint32_t	len;
		}	n;
	}	e;
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
			cw_sint32_t	len;
		}	s;
	}	e;
};


/*
 * Prototypes for private methods.
 */

/* stilo. */
static void	stilo_p_new(cw_stilo_t *a_stilo, cw_stilot_t a_type);
static cw_uint32_t stilo_p_hash(const void *a_key);
static cw_bool_t stilo_p_key_comp(const void *a_k1, const void *a_k2);

/* stiloe. */
static void	stiloe_p_new(cw_stiloe_t *a_stiloe, cw_stilot_t a_type);
static void	stiloe_p_delete(cw_stiloe_t *a_stiloe, cw_stilt_t *a_stilt);

#define		stiloe_p_lock(a_stiloe) do {				\
	if ((a_stiloe)->global)						\
		mtx_lock(&(a_stiloe)->lock);				\
} while (0)

#define		stiloe_p_unlock(a_stiloe) do {				\
	if ((a_stiloe)->global)						\
		mtx_unlock(&(a_stiloe)->lock);				\
} while (0)

/* no. */
static void	stilo_p_no_print(cw_stilo_t *a_stilo, cw_sint32_t a_fd,
    cw_bool_t a_syntactic, cw_bool_t a_newline);

/* array. */
static void	stilo_p_array_delete(cw_stilo_t *a_stilo, cw_stilt_t *a_stilt);
static cw_stiloe_t *stiloe_p_array_ref_iterate(cw_stiloe_t *a_stiloe, cw_bool_t
    a_reset);
static void	stilo_p_array_cast(cw_stilo_t *a_stilo, cw_stilot_t a_type);
static void	stilo_p_array_copy(cw_stilo_t *a_to, cw_stilo_t *a_from,
    cw_stilt_t *a_stilt);
static void	stilo_p_array_print(cw_stilo_t *a_stilo, cw_sint32_t a_fd,
    cw_bool_t a_syntactic, cw_bool_t a_newline);

/* boolean. */
static void	stilo_p_boolean_cast(cw_stilo_t *a_stilo, cw_stilot_t a_type);
static void	stilo_p_boolean_print(cw_stilo_t *a_stilo, cw_sint32_t a_fd,
    cw_bool_t a_syntactic, cw_bool_t a_newline);

/* condition. */
static void	stilo_p_condition_delete(cw_stilo_t *a_stilo, cw_stilt_t
    *a_stilt);
static cw_stiloe_t *stiloe_p_condition_ref_iterate(cw_stiloe_t *a_stiloe,
    cw_bool_t a_reset);
static void	stilo_p_condition_cast(cw_stilo_t *a_stilo, cw_stilot_t a_type);
static void	stilo_p_condition_print(cw_stilo_t *a_stilo, cw_sint32_t a_fd,
    cw_bool_t a_syntactic, cw_bool_t a_newline);

/* dict. */
static void	stilo_p_dict_delete(cw_stilo_t *a_stilo, cw_stilt_t *a_stilt);
static cw_stiloe_t *stiloe_p_dict_ref_iterate(cw_stiloe_t *a_stiloe, cw_bool_t
    a_reset);
static void	stilo_p_dict_cast(cw_stilo_t *a_stilo, cw_stilot_t a_type);
static void	stilo_p_dict_copy(cw_stilo_t *a_to, cw_stilo_t *a_from,
    cw_stilt_t *a_stilt);
static void	stilo_p_dict_print(cw_stilo_t *a_stilo, cw_sint32_t a_fd,
    cw_bool_t a_syntactic, cw_bool_t a_newline);

/* file. */
static void	stilo_p_file_delete(cw_stilo_t *a_stilo, cw_stilt_t *a_stilt);
static cw_stiloe_t *stiloe_p_file_ref_iterate(cw_stiloe_t *a_stiloe, cw_bool_t
    a_reset);
static void	stilo_p_file_cast(cw_stilo_t *a_stilo, cw_stilot_t a_type);
static void	stilo_p_file_copy(cw_stilo_t *a_to, cw_stilo_t *a_from,
    cw_stilt_t *a_stilt);
static void	stilo_p_file_print(cw_stilo_t *a_stilo, cw_sint32_t a_fd,
    cw_bool_t a_syntactic, cw_bool_t a_newline);

/* hook. */
static void	stilo_p_hook_delete(cw_stilo_t *a_stilo, cw_stilt_t *a_stilt);
static cw_stiloe_t *stiloe_p_hook_ref_iterate(cw_stiloe_t *a_stiloe, cw_bool_t
    a_reset);
static void	stilo_p_hook_cast(cw_stilo_t *a_stilo, cw_stilot_t a_type);
static void	stilo_p_hook_copy(cw_stilo_t *a_to, cw_stilo_t *a_from,
    cw_stilt_t *a_stilt);
static void	stilo_p_hook_print(cw_stilo_t *a_stilo, cw_sint32_t a_fd,
    cw_bool_t a_syntactic, cw_bool_t a_newline);

/* lock. */
static void	stilo_p_lock_delete(cw_stilo_t *a_stilo, cw_stilt_t *a_stilt);
static cw_stiloe_t *stiloe_p_lock_ref_iterate(cw_stiloe_t *a_stiloe, cw_bool_t
    a_reset);
static void	stilo_p_lock_cast(cw_stilo_t *a_stilo, cw_stilot_t a_type);
static void	stilo_p_lock_print(cw_stilo_t *a_stilo, cw_sint32_t a_fd,
    cw_bool_t a_syntactic, cw_bool_t a_newline);

/* mark. */
static void	stilo_p_mark_cast(cw_stilo_t *a_stilo, cw_stilot_t a_type);
static void	stilo_p_mark_print(cw_stilo_t *a_stilo, cw_sint32_t a_fd,
    cw_bool_t a_syntactic, cw_bool_t a_newline);

/* mstate. */
static void	stilo_p_mstate_delete(cw_stilo_t *a_stilo, cw_stilt_t *a_stilt);
static cw_stiloe_t *stiloe_p_mstate_ref_iterate(cw_stiloe_t *a_stiloe, cw_bool_t
    a_reset);
static void	stilo_p_mstate_cast(cw_stilo_t *a_stilo, cw_stilot_t a_type);
static void	stilo_p_mstate_copy(cw_stilo_t *a_to, cw_stilo_t *a_from,
    cw_stilt_t *a_stilt);
static void	stilo_p_mstate_print(cw_stilo_t *a_stilo, cw_sint32_t a_fd,
    cw_bool_t a_syntactic, cw_bool_t a_newline);

/* name. */
static void	stilo_p_name_delete(cw_stilo_t *a_stilo, cw_stilt_t *a_stilt);
static cw_stiloe_t *stiloe_p_name_ref_iterate(cw_stiloe_t *a_stiloe, cw_bool_t
    a_reset);
static void	stilo_p_name_cast(cw_stilo_t *a_stilo, cw_stilot_t a_type);
static void	stilo_p_name_copy(cw_stilo_t *a_to, cw_stilo_t *a_from,
    cw_stilt_t *a_stilt);
static void	stilo_p_name_print(cw_stilo_t *a_stilo, cw_sint32_t a_fd,
    cw_bool_t a_syntactic, cw_bool_t a_newline);

static cw_stiloe_name_t *stilo_p_name_gref(cw_stilt_t *a_stilt, const char
    *a_str, cw_uint32_t a_len, cw_bool_t a_is_static);
static cw_bool_t stilo_p_name_kref_insert(cw_stilo_t *a_stilo, cw_stilt_t
    *a_stilt, const cw_stiloe_dict_t *a_dict);
static cw_bool_t stilo_p_name_kref_search(cw_stilo_t *a_stilo, const
    cw_stiloe_dict_t *a_dict);
static cw_bool_t stilo_p_name_kref_remove(cw_stilo_t *a_stilo, cw_stilt_t
    *a_stilt, const cw_stiloe_dict_t *a_dict);

/* null. */
static void	stilo_p_null_cast(cw_stilo_t *a_stilo, cw_stilot_t a_type);
static void	stilo_p_null_print(cw_stilo_t *a_stilo, cw_sint32_t a_fd,
    cw_bool_t a_syntactic, cw_bool_t a_newline);

/* number. */
static void	stilo_p_number_delete(cw_stilo_t *a_stilo, cw_stilt_t *a_stilt);
static cw_stiloe_t *stiloe_p_number_ref_iterate(cw_stiloe_t *a_stiloe, cw_bool_t
    a_reset);
static void	stilo_p_number_cast(cw_stilo_t *a_stilo, cw_stilot_t a_type);
static void	stilo_p_number_copy(cw_stilo_t *a_to, cw_stilo_t *a_from,
    cw_stilt_t *a_stilt);
static void	stilo_p_number_print(cw_stilo_t *a_stilo, cw_sint32_t a_fd,
    cw_bool_t a_syntactic, cw_bool_t a_newline);

/* operator. */
static void	stilo_p_operator_cast(cw_stilo_t *a_stilo, cw_stilot_t a_type);
static void	stilo_p_operator_print(cw_stilo_t *a_stilo, cw_sint32_t a_fd,
    cw_bool_t a_syntactic, cw_bool_t a_newline);

/* string. */
static void	stilo_p_string_delete(cw_stilo_t *a_stilo, cw_stilt_t *a_stilt);
static cw_stiloe_t *stiloe_p_string_ref_iterate(cw_stiloe_t *a_stiloe, cw_bool_t
    a_reset);
static void	stilo_p_string_cast(cw_stilo_t *a_stilo, cw_stilot_t a_type);
static void	stilo_p_string_copy(cw_stilo_t *a_to, cw_stilo_t *a_from,
    cw_stilt_t *a_stilt);
static void	stilo_p_string_print(cw_stilo_t *a_stilo, cw_sint32_t a_fd,
    cw_bool_t a_syntactic, cw_bool_t a_newline);

/*
 * vtable setup for the various operations on stilo's that are polymorphic.
 */
typedef void		cw_stilot_delete_t(cw_stilo_t *a_stilo, cw_stilt_t
    *a_stilt);
typedef cw_stiloe_t	*cw_stilot_ref_iterate_t(cw_stiloe_t *a_stiloe,
    cw_bool_t a_reset);
typedef void		cw_stilot_cast_t(cw_stilo_t *a_stilo, cw_stilot_t
    a_type);
typedef void		cw_stilot_copy_t(cw_stilo_t *a_to, cw_stilo_t *a_from,
    cw_stilt_t *a_stilt);
typedef void		cw_stilot_print_t(cw_stilo_t *a_stilo, cw_sint32_t a_fd,
    cw_bool_t a_syntactic, cw_bool_t a_newline);

typedef struct cw_stilot_vtable_s cw_stilot_vtable_t;
struct  cw_stilot_vtable_s {
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
	 stilo_p_no_print},	/* XXX Debugging only. */

	/* _CW_STILOT_ARRAYTYPE */
	{stilo_p_array_delete,
	 stiloe_p_array_ref_iterate,
	 stilo_p_array_cast,
	 stilo_p_array_copy,
	 stilo_p_array_print},

	/* _CW_STILOT_BOOLEANTYPE */
	{NULL,
	 NULL,
	 stilo_p_boolean_cast,
	 NULL,
	 stilo_p_boolean_print},

	/* _CW_STILOT_CONDITIONTYPE */
	{stilo_p_condition_delete,
	 stiloe_p_condition_ref_iterate,
	 stilo_p_condition_cast,
	 NULL,
	 stilo_p_condition_print},
	
	/* _CW_STILOT_DICTTYPE */
	{stilo_p_dict_delete,
	 stiloe_p_dict_ref_iterate,
	 stilo_p_dict_cast,
	 stilo_p_dict_copy,
	 stilo_p_dict_print},

	/* _CW_STILOT_FILETYPE */
	{stilo_p_file_delete,
	 stiloe_p_file_ref_iterate,
	 stilo_p_file_cast,
	 stilo_p_file_copy,
	 stilo_p_file_print},

	/* _CW_STILOT_HOOKTYPE */
	{stilo_p_hook_delete,
	 stiloe_p_hook_ref_iterate,
	 stilo_p_hook_cast,
	 stilo_p_hook_copy,
	 stilo_p_hook_print},

	/* _CW_STILOT_LOCKTYPE */
	{stilo_p_lock_delete,
	 stiloe_p_lock_ref_iterate,
	 stilo_p_lock_cast,
	 NULL,
	 stilo_p_lock_print},

	/* _CW_STILOT_MARKTYPE */
	{NULL,
	 NULL,
	 stilo_p_mark_cast,
	 NULL,
	 stilo_p_mark_print},

	/* _CW_STILOT_MSTATETYPE */
	{stilo_p_mstate_delete,
	 stiloe_p_mstate_ref_iterate,
	 stilo_p_mstate_cast,
	 stilo_p_mstate_copy,
	 stilo_p_mstate_print},

	/* _CW_STILOT_NAMETYPE */
	{stilo_p_name_delete,
	 stiloe_p_name_ref_iterate,
	 stilo_p_name_cast,
	 stilo_p_name_copy,
	 stilo_p_name_print},

	/* _CW_STILOT_NULLTYPE */
	{NULL,
	 NULL,
	 stilo_p_null_cast,
	 NULL,
	 stilo_p_null_print},

	/* _CW_STILOT_NUMBERTYPE */
	{stilo_p_number_delete,
	 stiloe_p_number_ref_iterate,
	 stilo_p_number_cast,
	 stilo_p_number_copy,	/* XXX Same as dup. */
	 stilo_p_number_print},

	/* _CW_STILOT_OPERATORTYPE */
	{NULL,
	 NULL,
	 stilo_p_operator_cast,
	 NULL,
	 stilo_p_operator_print},

	/* _CW_STILOT_STRINGTYPE */
	{stilo_p_string_delete,
	 stiloe_p_string_ref_iterate,
	 stilo_p_string_cast,
	 stilo_p_string_copy,
	 stilo_p_string_print}
};

/*
 * stilo.
 */

void
stilo_clobber(cw_stilo_t *a_stilo)
{
/*  	thd_crit_enter(); */
	memset(a_stilo, 0, sizeof(cw_stilo_t));
/*  	thd_crit_leave(); */
}

void
stilo_delete(cw_stilo_t *a_stilo, cw_stilt_t *a_stilt)
{
	_cw_check_ptr(a_stilo);
	_cw_assert(a_stilo->magic == _CW_STILO_MAGIC);

	if (stilot_vtable[a_stilo->type].delete_f != NULL)
		stilot_vtable[a_stilo->type].delete_f(a_stilo, a_stilt);

	stilo_clobber(a_stilo);
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
stilo_copy(cw_stilo_t *a_to, cw_stilo_t *a_from, cw_stilt_t *a_stilt)
{
	_cw_check_ptr(a_to);
	_cw_assert(a_to->magic == _CW_STILO_MAGIC);
	_cw_assert(a_to->type == _CW_STILOT_NOTYPE);
	_cw_check_ptr(a_from);
	_cw_assert(a_from->magic == _CW_STILO_MAGIC);

	stilot_vtable[a_from->type].copy_f(a_to, a_from, a_stilt);
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
	stilo_clobber(a_to);

	/* Mark the stilo as invalid. */
	type = a_from->type;
	a_from->type = _CW_STILOT_NOTYPE;

	/* Copy. */
	memcpy(a_to, a_from, sizeof(cw_stilo_t));

	/* Numbers are not composite, so handle them specially. */
	if ((type == _CW_STILOT_NUMBERTYPE) && (a_from->extended)) {
		/* XXX Create a duplicate stiloe. */
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
	stilo_clobber(a_to);
	
	memcpy(a_to, a_from, sizeof(cw_stilo_t));

	/* Reset the source stilo. */
	stilo_clobber(a_from);
	stilo_no_new(a_from);
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

/* Call before other initialization. */
static void
stilo_p_new(cw_stilo_t *a_stilo, cw_stilot_t a_type)
{
	_cw_check_ptr(a_stilo);

	memset(a_stilo, 0, sizeof(cw_stilo_t));
	a_stilo->type = a_type;

#ifdef _LIBSTIL_DBG
	a_stilo->magic = _CW_STILO_MAGIC;
#endif
}

/* Hash any stilo, but optimize for name hashing. */
static cw_uint32_t
stilo_p_hash(const void *a_key)
{
	cw_uint32_t	retval;
	cw_stilo_t	*key = (cw_stilo_t *)a_key;

	_cw_check_ptr(key);
	_cw_assert(key->magic == _CW_STILO_MAGIC);

	if (key->type == _CW_STILOT_NAMETYPE) {
		cw_stiloe_name_t	*name;

		/* Chase down the name. */
		name = (cw_stiloe_name_t *)key->o.stiloe;
		name = name->val;
		_cw_assert(name == name->val);

		retval = stilo_name_hash((void *)name);
	} else {
		/* XXX Implement. */
		_cw_error("Unimplemented");
	}

	return retval;
}

/* Compare stilo's, but optimize for name comparison. */
static cw_bool_t
stilo_p_key_comp(const void *a_k1, const void *a_k2)
{
	cw_bool_t	retval;
	cw_stilo_t	*k1 = (cw_stilo_t *)a_k1;
	cw_stilo_t	*k2 = (cw_stilo_t *)a_k2;

	_cw_check_ptr(k1);
	_cw_assert(k1->magic == _CW_STILO_MAGIC);
	_cw_check_ptr(k2);
	_cw_assert(k2->magic == _CW_STILO_MAGIC);

	if ((k1->type == _CW_STILOT_NAMETYPE) && (k1->type ==
	    _CW_STILOT_NAMETYPE)) {
		cw_stiloe_name_t	*n1, *n2;
		
		/* Chase down the names. */
		n1 = (cw_stiloe_name_t *)k1->o.stiloe;
		n1 = n1->val;
		_cw_assert(n1 == n1->val);
		n2 = (cw_stiloe_name_t *)k2->o.stiloe;
		n2 = n2->val;
		_cw_assert(n2 == n2->val);

		retval = stilo_name_key_comp((void *)n1, (void *)n2);
	} else {
		/* XXX Implement. */
		_cw_error("Unimplemented");
	}

	return retval;
}

/*
 * stiloe.
 */

/* Can be called at any time during stiloe_* initialization. */
static void
stiloe_p_new(cw_stiloe_t *a_stiloe, cw_stilot_t a_type)
{
	/* Initialize the common section. */
	memset(a_stiloe, 0, sizeof(cw_stiloe_t));

	a_stiloe->type = a_type;
	mtx_new(&a_stiloe->lock);
#ifdef _LIBSTIL_DBG
	a_stiloe->magic = _CW_STILOE_MAGIC;
#endif
}

static void
stiloe_p_delete(cw_stiloe_t *a_stiloe, cw_stilt_t *a_stilt)
{
	mtx_delete(&a_stiloe->lock);
	_cw_stilt_free(a_stilt, a_stiloe);
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
void
stilo_no_new(cw_stilo_t *a_stilo)
{
	stilo_p_new(a_stilo, _CW_STILOT_NOTYPE);
}

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
void
stilo_array_new(cw_stilo_t *a_stilo, cw_stilt_t *a_stilt, cw_uint32_t a_len)
{
	cw_stiloe_array_t	*array;
	cw_uint32_t		i;

	stilo_p_new(a_stilo, _CW_STILOT_ARRAYTYPE);

	array = (cw_stiloe_array_t *)_cw_stilt_malloc(a_stilt,
	    sizeof(cw_stiloe_array_t));

	array->iterations = 0;
	array->e.a.len = a_len;
	if (array->e.a.len > 0) {
		array->e.a.arr = (cw_stilo_t *)_cw_stilt_malloc(a_stilt,
		    sizeof(cw_stilo_t) * array->e.a.len);
		for (i = 0; i < array->e.a.len; i++)
			stilo_no_new(&array->e.a.arr[i]);
	}
	a_stilo->o.stiloe = (cw_stiloe_t *)array;

	stiloe_p_new(a_stilo->o.stiloe, _CW_STILOT_ARRAYTYPE);
}

static void
stilo_p_array_delete(cw_stilo_t *a_stilo, cw_stilt_t *a_stilt)
{
	cw_stiloe_array_t	*array;
	cw_uint32_t		i;

	array = (cw_stiloe_array_t *)a_stilo->o.stiloe;
	_cw_check_ptr(array);
	_cw_assert(array->stiloe.magic == _CW_STILOE_MAGIC);

	if ((array->stiloe.indirect == FALSE)) {
		for (i = 0; i < array->e.a.len; i++)
			stilo_delete(&array->e.a.arr[i], a_stilt);
	}
	_cw_stilt_free(a_stilt, array->e.a.arr);

	stiloe_p_delete(&array->stiloe, a_stilt);
}

static cw_stiloe_t *
stiloe_p_array_ref_iterate(cw_stiloe_t *a_stiloe, cw_bool_t a_reset)
{
	cw_stiloe_t		*retval;
	cw_stiloe_array_t	*array;

	array = (cw_stiloe_array_t *)a_stiloe;

	if (a_reset)
		array->iterations = 0;

	if (a_stiloe->indirect) {
		if (array->iterations == 0) {
			retval = stilo_l_stiloe_get(&array->e.i.stilo);
			array->iterations++;
		} else {
			retval = NULL;
			array->iterations = 0;
		}
	} else {
		retval = NULL;
		while (retval == NULL && array->iterations < array->e.a.len) {
			retval =
			    stilo_l_stiloe_get(&array->e.a.arr[array->iterations]);
			array->iterations++;
		}

		if (retval == NULL)
			array->iterations = 0;
	}

	return retval;
}

static void
stilo_p_array_cast(cw_stilo_t *a_stilo, cw_stilot_t a_type)
{
}

static void
stilo_p_array_copy(cw_stilo_t *a_to, cw_stilo_t *a_from, cw_stilt_t *a_stilt)
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
		retval = array->e.a.arr;
	} else {
		retval = &stilo_array_get(&array->e.i.stilo)
		    [array->e.i.beg_offset];
	}

	return retval;
}

void
stilo_array_set(cw_stilo_t *a_stilo, cw_uint32_t a_offset, cw_stilo_t *a_arr,
    cw_uint32_t a_len, cw_stilt_t *a_stilt)
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
		stilo_copy(&arr[i], &a_arr[i], a_stilt);
}

/*
 * boolean.
 */
void
stilo_boolean_new(cw_stilo_t *a_stilo, cw_bool_t a_val)
{
	stilo_p_new(a_stilo, _CW_STILOT_BOOLEANTYPE);
	a_stilo->o.boolean.val = a_val;
}


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

cw_bool_t
stilo_boolean_get(cw_stilo_t *a_stilo)
{
	_cw_check_ptr(a_stilo);
	_cw_assert(a_stilo->magic == _CW_STILO_MAGIC);
	_cw_assert(a_stilo->type == _CW_STILOT_BOOLEANTYPE);

	return a_stilo->o.boolean.val;
}

void
stilo_boolean_set(cw_stilo_t *a_stilo, cw_bool_t a_val)
{
	_cw_check_ptr(a_stilo);
	_cw_assert(a_stilo->magic == _CW_STILO_MAGIC);
	_cw_assert(a_stilo->type == _CW_STILOT_BOOLEANTYPE);

	a_stilo->o.boolean.val = a_val;
}

/*
 * condition.
 */
void
stilo_condition_new(cw_stilo_t *a_stilo, cw_stilt_t *a_stilt)
{
	stilo_p_new(a_stilo, _CW_STILOT_CONDITIONTYPE);
}

static void
stilo_p_condition_delete(cw_stilo_t *a_stilo, cw_stilt_t *a_stilt)
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

void
stilo_dict_new(cw_stilo_t *a_stilo, cw_stilt_t *a_stilt, cw_uint32_t
    a_dict_size)
{
	cw_stiloe_dict_t	*dict;

	stilo_p_new(a_stilo, _CW_STILOT_DICTTYPE);

	dict = (cw_stiloe_dict_t *)_cw_stilt_malloc(a_stilt,
	    sizeof(cw_stiloe_dict_t));

	dict->iterations = 0;
	dict->e.d.dicto = NULL;
	dict->e.d.capacity = a_dict_size;

	/*
	 * Don't create a dict smaller than 16, since rounding errors for
	 * calculating the grow/shrink points can cause severe performance
	 * problems if the dict grows significantly.
	 *
	 * Don't let the table get more than 80% full, or less than 25% full,
	 * when shrinking.
	 *
	 * XXX Magic numbers.
	 */
	if (a_dict_size > 16) {
		dch_new(&dict->e.d.hash, stilt_mem_get(a_stilt), a_dict_size *
		    1.25, a_dict_size, a_dict_size / 4, stilo_p_hash,
		    stilo_p_key_comp);
	} else {
		dch_new(&dict->e.d.hash, stilt_mem_get(a_stilt), 20, 16, 4,
		    stilo_p_hash, stilo_p_key_comp);
	}

	a_stilo->o.stiloe = (cw_stiloe_t *)dict;

	stiloe_p_new(a_stilo->o.stiloe, _CW_STILOT_DICTTYPE);
}

static void
stilo_p_dict_delete(cw_stilo_t *a_stilo, cw_stilt_t *a_stilt)
{
	cw_stiloe_dict_t	*dict;

	_cw_check_ptr(dict);
	_cw_assert(dict->stiloe.magic == _CW_STILOE_MAGIC);

	dict = (cw_stiloe_dict_t *)a_stilo->o.stiloe;

	if (dict->stiloe.indirect == FALSE) {
		cw_stiloe_dicto_t	*dicto;
		cw_chi_t		*chi;

		while (dch_remove_iterate(&dict->e.d.hash, NULL, (void
		    **)&dicto, &chi) == FALSE) {
			stilo_delete(&dicto->key, a_stilt);
			stilo_delete(&dicto->val, a_stilt);
			_cw_stilt_chi_put(a_stilt, chi);
		}
	}
}

static cw_stiloe_t *
stiloe_p_dict_ref_iterate(cw_stiloe_t *a_stiloe, cw_bool_t a_reset)
{
	cw_stiloe_t		*retval;
	cw_stiloe_dict_t	*dict;

	dict = (cw_stiloe_dict_t *)a_stiloe;

	if (a_reset) {
		dict->iterations = 0;
		if (a_stiloe->indirect == FALSE)
			dict->e.d.dicto = NULL;
	}

	if (a_stiloe->indirect) {
		if (dict->iterations == 0) {
			retval = stilo_l_stiloe_get(&dict->e.i.stilo);
			dict->iterations++;
		} else {
			retval = NULL;
			dict->iterations = 0;
		}
	} else {
		retval = NULL;
		while (retval == NULL && dict->iterations <
		    dch_count(&dict->e.d.hash)) {
			if (dict->e.d.dicto == NULL) {
				/* Key. */
				dch_get_iterate(&dict->e.d.hash, NULL,(void
				    **)&dict->e.d.dicto);
				retval =
				    stilo_l_stiloe_get(&dict->e.d.dicto->key);
			} else {
				/* Value. */
				retval = 
				    stilo_l_stiloe_get(&dict->e.d.dicto->val);
				dict->iterations++;
				dict->e.d.dicto = NULL;
			}
		}
	}

	return retval;
}

static void
stilo_p_dict_cast(cw_stilo_t *a_stilo, cw_stilot_t a_type)
{
}

static void
stilo_p_dict_copy(cw_stilo_t *a_to, cw_stilo_t *a_from, cw_stilt_t *a_stilt)
{
	cw_stiloe_dict_t	*to, *from;
	cw_uint32_t		i, count;
	cw_stiloe_dicto_t	*dicto_to, *dicto_from;
	cw_chi_t		*chi;

	from = (cw_stiloe_dict_t *)a_from->o.stiloe;
	if (from->stiloe.indirect) {
		from = (cw_stiloe_dict_t *)from->e.i.stilo.o.stiloe;
		/* Only one level of indirection is allowed. */
		_cw_assert(from->stiloe.indirect == FALSE);
	}

	/* Deep (but not recursive) copy. */
	stilo_dict_new(a_to, a_stilt, from->e.d.capacity);
	to = (cw_stiloe_dict_t *)a_to->o.stiloe;

	stiloe_p_lock(&from->stiloe);
	for (i = 0, count = dch_count(&from->e.d.hash); i < count; i++) {
		/* Get a dicto. */
		dch_get_iterate(&from->e.d.hash, NULL, (void **)&dicto_from);

		/* Allocate and copy. */
		dicto_to = _cw_stilt_dicto_get(a_stilt);
		stilo_no_new(&dicto_to->key);
		stilo_dup(&dicto_to->key, &dicto_from->key);
		stilo_no_new(&dicto_to->val);
		stilo_dup(&dicto_to->val, &dicto_from->val);
		chi = _cw_stilt_chi_get(a_stilt);

		/* Insert. */
		dch_insert(&to->e.d.hash, &dicto_to->key, dicto_to, chi);
	}
	stiloe_p_unlock(&from->stiloe);
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

void
stilo_dict_def(cw_stilo_t *a_stilo, cw_stilt_t *a_stilt, cw_stilo_t *a_key,
    cw_stilo_t *a_val)
{
	cw_stiloe_dict_t	*dict;

	_cw_check_ptr(a_stilo);
	_cw_assert(a_stilo->magic == _CW_STILO_MAGIC);
	_cw_assert(a_stilo->type == _CW_STILOT_DICTTYPE);
	/* XXX Make sure setglobal is correct. */

	dict = (cw_stiloe_dict_t *)a_stilo->o.stiloe;

	if (dict->stiloe.indirect)
		stilo_dict_def(&dict->e.i.stilo, a_stilt, a_key, a_val);
	else {
		cw_stiloe_dicto_t	*dicto;

		stiloe_p_lock(&dict->stiloe);
		if (dch_search(&dict->e.d.hash, (void *)a_key, (void **)&dicto)
		    == FALSE) {
			/* a_key is already defined. */
			stilo_move(&dicto->val, a_val);

			/*
			 * If (a_key == &dicto->val), things will break badly.
			 * However, I can't think of a way that this could
			 * possibly happen in real use, so just assert.
			 */
			_cw_assert(a_key != &dicto->val);
			stilo_delete(a_key, a_stilt);
		} else {
			cw_chi_t	*chi;

			/* Allocate and initialize. */
			dicto = _cw_stilt_dicto_get(a_stilt);
			chi = _cw_stilt_chi_get(a_stilt);
			stilo_no_new(&dicto->key);
			stilo_move(&dicto->key, a_key);
			stilo_no_new(&dicto->val);
			stilo_move(&dicto->val, a_val);

			/* Insert. */
			dch_insert(&dict->e.d.hash, (void *)&dicto->key,
			    (void *)dicto, chi);
		}
		stiloe_p_unlock(&dict->stiloe);
	}
}

void
stilo_dict_undef(cw_stilo_t *a_stilo, cw_stilt_t *a_stilt, const cw_stilo_t
    *a_key)
{
	cw_stiloe_dict_t	*dict;

	_cw_check_ptr(a_stilo);
	_cw_assert(a_stilo->magic == _CW_STILO_MAGIC);
	_cw_assert(a_stilo->type == _CW_STILOT_DICTTYPE);

	dict = (cw_stiloe_dict_t *)a_stilo->o.stiloe;

	if (dict->stiloe.indirect)
		stilo_dict_undef(&dict->e.i.stilo, a_stilt, a_key);
	else {
		cw_stiloe_dicto_t	*dicto;
		cw_chi_t		*chi;

		stiloe_p_lock(&dict->stiloe);
		if (dch_remove(&dict->e.d.hash, (void *)a_key, NULL, (void
		    **)&dicto, &chi) == FALSE) {
			stilo_delete(&dicto->key, a_stilt);
			stilo_delete(&dicto->val, a_stilt);
			_cw_stilt_dicto_put(a_stilt, dicto);
			_cw_stilt_chi_put(a_stilt, chi);
		}
		stiloe_p_unlock(&dict->stiloe);
	}
}

cw_bool_t
stilo_dict_lookup(cw_stilo_t *a_stilo, cw_stilt_t *a_stilt, const cw_stilo_t
    *a_key, cw_stilo_t *r_stilo)
{
	cw_bool_t		retval;
	cw_stiloe_dict_t	*dict;

	_cw_check_ptr(a_stilo);
	_cw_assert(a_stilo->magic == _CW_STILO_MAGIC);
	_cw_assert(a_stilo->type == _CW_STILOT_DICTTYPE);

	dict = (cw_stiloe_dict_t *)a_stilo->o.stiloe;

	if (dict->stiloe.indirect) {
		retval = stilo_dict_lookup(&dict->e.i.stilo, a_stilt, a_key,
		    r_stilo);
	} else {
		cw_stiloe_dicto_t	*dicto;

		stiloe_p_lock(&dict->stiloe);
		if (dch_search(&dict->e.d.hash, (void *)a_key, (void **)&dicto)
		    == FALSE) {
			stilo_no_new(r_stilo);
			stilo_dup(r_stilo, &dicto->val);
			retval = FALSE;
		} else
			retval = TRUE;
		stiloe_p_unlock(&dict->stiloe);
	}
	
	return retval;
}

cw_uint32_t
stilo_dict_count(cw_stilo_t *a_stilo)
{
	cw_uint32_t		retval;
	cw_stiloe_dict_t	*dict;

	_cw_check_ptr(a_stilo);
	_cw_assert(a_stilo->magic == _CW_STILO_MAGIC);
	_cw_assert(a_stilo->type == _CW_STILOT_DICTTYPE);

	dict = (cw_stiloe_dict_t *)a_stilo->o.stiloe;

	stiloe_p_lock(&dict->stiloe);
	if (dict->stiloe.indirect)
		retval = stilo_dict_count(&dict->e.i.stilo);
	else
		retval = dch_count(&dict->e.d.hash);
	stiloe_p_unlock(&dict->stiloe);

	return retval;
}

cw_bool_t
stilo_dict_iterate(cw_stilo_t *a_stilo, cw_stilt_t *a_stilt, cw_stilo_t
    *r_stilo)
{
	cw_bool_t		retval;
	cw_stiloe_dict_t	*dict;
	cw_stilo_t		*stilo;
	
	_cw_check_ptr(a_stilo);
	_cw_assert(a_stilo->magic == _CW_STILO_MAGIC);
	_cw_assert(a_stilo->type == _CW_STILOT_DICTTYPE);

	dict = (cw_stiloe_dict_t *)a_stilo->o.stiloe;

	stiloe_p_lock(&dict->stiloe);
	if (dict->stiloe.indirect)
		retval = stilo_dict_iterate(&dict->e.i.stilo, a_stilt, r_stilo);
	else {
		dch_get_iterate(&dict->e.d.hash, (void **)&stilo, NULL);
		stilo_no_new(r_stilo);
		stilo_dup(r_stilo, stilo);
		retval = FALSE;
	}
	stiloe_p_unlock(&dict->stiloe);

	return retval;
}

/*
 * file.
 */
void
stilo_file_new(cw_stilo_t *a_stilo, cw_stilt_t *a_stilt)
{
	stilo_p_new(a_stilo, _CW_STILOT_FILETYPE);
}

static void
stilo_p_file_delete(cw_stilo_t *a_stilo, cw_stilt_t *a_stilt)
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
stilo_p_file_copy(cw_stilo_t *a_to, cw_stilo_t *a_from, cw_stilt_t *a_stilt)
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
void
stilo_hook_new(cw_stilo_t *a_stilo, cw_stilt_t *a_stilt)
{
	stilo_p_new(a_stilo, _CW_STILOT_HOOKTYPE);
}

static void
stilo_p_hook_delete(cw_stilo_t *a_stilo, cw_stilt_t *a_stilt)
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
stilo_p_hook_copy(cw_stilo_t *a_to, cw_stilo_t *a_from, cw_stilt_t *a_stilt)
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
void
stilo_lock_new(cw_stilo_t *a_stilo, cw_stilt_t *a_stilt)
{
	stilo_p_new(a_stilo, _CW_STILOT_LOCKTYPE);
}

static void
stilo_p_lock_delete(cw_stilo_t *a_stilo, cw_stilt_t *a_stilt)
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
void
stilo_mark_new(cw_stilo_t *a_stilo)
{
	stilo_p_new(a_stilo, _CW_STILOT_MARKTYPE);
}

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
void
stilo_mstate_new(cw_stilo_t *a_stilo, cw_stilt_t *a_stilt)
{
	stilo_p_new(a_stilo, _CW_STILOT_MSTATETYPE);
}

static void
stilo_p_mstate_delete(cw_stilo_t *a_stilo, cw_stilt_t *a_stilt)
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
stilo_p_mstate_copy(cw_stilo_t *a_to, cw_stilo_t *a_from, cw_stilt_t *a_stilt)
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
/* Hash {name string, length}. */
cw_uint32_t
stilo_name_hash(const void *a_key)
{
	cw_uint32_t		retval, i;
	cw_stiloe_name_t	*key = (cw_stiloe_name_t *)a_key;
	const char		*str;

	_cw_check_ptr(a_key);

	for (i = 0, str = key->e.n.name, retval = 0; i < key->e.n.len;
	    i++, str++)
		retval = retval * 33 + *str;

	return retval;
}

/* Compare keys {name string, length}. */
cw_bool_t
stilo_name_key_comp(const void *a_k1, const void *a_k2)
{
	cw_stiloe_name_t	*k1 = (cw_stiloe_name_t *)a_k1;
	cw_stiloe_name_t	*k2 = (cw_stiloe_name_t *)a_k2;
	size_t			len;

	_cw_check_ptr(a_k1);
	_cw_check_ptr(a_k2);

	if (k1->e.n.len > k2->e.n.len)
		len = k1->e.n.len;
	else
		len = k2->e.n.len;

#if (0)
	{
		cw_bool_t	equal;

		equal = strncmp((char *)k1->e.n.name, (char *)k2->e.n.name, len)
		    ? FALSE : TRUE;

		_cw_out_put_e("\"");
		_cw_out_put_n(k1->e.n.len, "[s]", k1->e.n.name);
		_cw_out_put("\" [c]= \"", equal ? '=' : '!');
		_cw_out_put_n(k2->e.n.len, "[s]", k2->e.n.name);
		_cw_out_put("\"\n");
	}
#endif

	return strncmp((char *)k1->e.n.name, (char *)k2->e.n.name, len) ? FALSE
	    : TRUE;
}

/*
 * a_stilt's allocation mode (local/global) is used to determine how the object
 * is allocated.
 */
void
stilo_name_new(cw_stilo_t *a_stilo, cw_stilt_t *a_stilt, const cw_uint8_t
    *a_name, cw_uint32_t a_len, cw_bool_t a_is_static)
{
	cw_stiloe_name_t	*name, key;

	stilo_p_new(a_stilo, _CW_STILOT_NAMETYPE);

	/* Fake up a key so that we can search the hash tables. */
	key.e.n.name = a_name;
	key.e.n.len = a_len;

	if (stilt_currentglobal(a_stilt) == FALSE) {
		cw_stilnt_t		*stilnt;
		cw_stiloe_name_t	*gname;

		/*
		 * Look in the per-thread name cache for a cached reference to
		 * the name.  If there is no cached reference, check the global
		 * hash for the name.  Create the name in the global hash if
		 * necessary, then create a cached reference if necessary.
		 */
		stilnt = stilt_stilnt_get(a_stilt);
		if (dch_search(&stilnt->hash, (void *)&key, (void **)&name)) {
			/* Not found in the per-thread name cache. */
			gname = stilo_p_name_gref(a_stilt, a_name, a_len,
			    a_is_static);

			name = (cw_stiloe_name_t *)_cw_stilt_malloc(a_stilt,
			    sizeof(cw_stiloe_name_t));

			name->val = gname;

			/* stilo-internal initialization. */
			stilo_p_new(&name->e.i.stilo, _CW_STILOT_NAMETYPE);
			name->e.i.stilo.o.stiloe = (cw_stiloe_t *)gname;

			stiloe_p_new(&name->stiloe, _CW_STILOT_NAMETYPE);
			name->stiloe.indirect = TRUE;

			/*
			 * Insert a cached entry for this thread.
			 */
			dch_insert(&stilnt->hash, (void *)gname, (void
			    **)name, _cw_stilt_chi_get(a_stilt));
		}
	} else
		name = stilo_p_name_gref(a_stilt, a_name, a_len, a_is_static);

/*  	thd_crit_enter(); */
	a_stilo->o.stiloe = (cw_stiloe_t *)name;
/*  	thd_crit_leave(); */
}

static void
stilo_p_name_delete(cw_stilo_t *a_stilo, cw_stilt_t *a_stilt)
{
	/* Do nothing. */
}

static cw_stiloe_t *
stiloe_p_name_ref_iterate(cw_stiloe_t *a_stiloe, cw_bool_t a_reset)
{
#if (0)
#ifdef _LIBSTIL_DBG
	cw_stiloe_name_t	*name;
	
	name = (cw_stiloe_name_t *)a_stilo->o.stiloe->val;
	if (name->e.n.keyed_refs != NULL) {
		cw_uint32_t	i;
		void		*key;

		_cw_out_put_e("Name \"");
		_cw_out_put_n(name->e.n.len, "[s]", name->e.n.name);
		_cw_out_put("\" still exists with [i] keyed reference[s]:",
		    dch_count(name->e.n.keyed_refs),
		    (dch_count(name->e.n.keyed_refs) == 1) ? "" : "s");
		for (i = 0; i < dch_count(name->e.n.keyed_refs); i++) {
			dch_get_iterate(name->e.n.keyed_refs, &key, NULL);
			_cw_out_put(" 0x[p]", key);
		}
		_cw_out_put("\n");
		abort();
	}
#endif
#endif
	return NULL;	/* XXX */
}

static void
stilo_p_name_cast(cw_stilo_t *a_stilo, cw_stilot_t a_type)
{
}

static void
stilo_p_name_copy(cw_stilo_t *a_to, cw_stilo_t *a_from, cw_stilt_t *a_stilt)
{
}

static void
stilo_p_name_print(cw_stilo_t *a_stilo, cw_sint32_t a_fd, cw_bool_t
    a_syntactic, cw_bool_t a_newline)
{
	cw_stiloe_name_t	*name;

	/* Chase down the name. */
	name = (cw_stiloe_name_t *)a_stilo->o.stiloe;
	name = name->val;
	_cw_assert(name == name->val);
	
	if (a_syntactic)
		_cw_out_put_f(a_fd, "/");

/*  	_cw_out_put_f(a_fd, "0x[p], len [i]\n", name->e.n.name, name->e.n.len); */
	_cw_out_put_fn(a_fd, name->e.n.len, "[s]", name->e.n.name);

	if (a_newline)
		_cw_out_put_f(a_fd, "\n");
}

static cw_stiloe_name_t *
stilo_p_name_gref(cw_stilt_t *a_stilt, const char *a_str, cw_uint32_t a_len,
    cw_bool_t a_is_static)
{
	cw_stiloe_name_t	*retval, key;
	cw_stilng_t		*stilng;

/*  	_cw_out_put_e("Reference ([s] VM) \"", stilt_currentglobal(a_stilt) ? */
/*  	    "global" : "local"); */
/*  	_cw_out_put_n(a_len, "[s]", a_str); */
/*  	_cw_out_put("\" (len [i])\n", a_len); */

	/* Fake up a key so that we can search the hash tables. */
	key.e.n.name = a_str;
	key.e.n.len = a_len;

	stilng = stil_stilng_get(stilt_stil_get(a_stilt));

	/*
	 * Look in the global hash for the name.  If the name doesn't exist,
	 * create it.
	 */
	mtx_lock(&stilng->lock);
	if (dch_search(&stilng->hash, (void *)&key, (void **)&retval)) {
		cw_stilag_t	*stilag;

		/*
		 * Not found in the global hash.  Create, initialize, and insert
		 * a new entry.
		 */
		stilag = stil_stilag_get(stilt_stil_get(a_stilt));
		retval = (cw_stiloe_name_t *)_cw_stilag_malloc(stilag,
		    sizeof(cw_stiloe_name_t));
		memset(retval, 0, sizeof(cw_stiloe_name_t));

		retval->val = retval;

		retval->e.n.is_static_name = a_is_static;
		retval->e.n.len = a_len;
	
		if (a_is_static == FALSE) {
			/* This should be allocated from global space. */
			retval->e.n.name =
			    _cw_stilag_malloc(stil_stilag_get(stilt_stil_get(a_stilt)),
			    a_len);
			/*
			 * Cast away the const here; it's the only place that
			 * the string is allowed to be modified, and this cast
			 * is better than dropping the const altogether.
			 */
			memcpy((cw_uint8_t *)retval->e.n.name, a_str, a_len);
		} else
			retval->e.n.name = a_str;

		stiloe_p_new(&retval->stiloe, _CW_STILOT_NAMETYPE);

		dch_insert(&stilng->hash, (void *)retval, (void
		    **)retval, _cw_stilt_chi_get(a_stilt));
	}
	mtx_unlock(&stilng->lock);

	return retval;
}

/* Insert a keyed reference. */
static cw_bool_t
stilo_p_name_kref_insert(cw_stilo_t *a_stilo, cw_stilt_t *a_stilt, const
    cw_stiloe_dict_t *a_dict)
{
	cw_bool_t		retval;
	cw_stiloe_name_t	*name;

	/* Chase down the name. */
	name = (cw_stiloe_name_t *)a_stilo->o.stiloe;
	name = name->val;

	stiloe_p_lock(&name->stiloe);
	if (name->e.n.keyed_refs == NULL) {
		/* No keyed references.  Create the hash. */
		name->e.n.keyed_refs = dch_new(NULL,
		    stilag_mem_get(stil_stilag_get(stilt_stil_get(a_stilt))),
		    _CW_STILO_NAME_KREF_TABLE, _CW_STILO_NAME_KREF_GROW,
		    _CW_STILO_NAME_KREF_SHRINK, ch_direct_hash,
		    ch_direct_key_comp);
		/* XXX Check dch_new() return. */
	}

	if (dch_insert(name->e.n.keyed_refs, (void *)a_dict, NULL,
	    _cw_stilt_chi_get(a_stilt))) {
		retval = TRUE;
		goto RETURN;
	}

	retval = FALSE;
	RETURN:
	stiloe_p_unlock(&name->stiloe);
	return retval;
}

/* Search for a keyed reference matching a_dict. */
static cw_bool_t
stilo_p_name_kref_search(cw_stilo_t *a_stilo, const cw_stiloe_dict_t *a_dict)
{
	cw_bool_t		retval;
	cw_stiloe_name_t	*name;

	/* Chase down the name. */
	name = (cw_stiloe_name_t *)a_stilo->o.stiloe;
	name = name->val;

	stiloe_p_lock(&name->stiloe);
	if ((name->e.n.keyed_refs == NULL) || dch_search(name->e.n.keyed_refs,
	    (void *)a_dict, NULL)) {
		/* Not found. */
		retval = TRUE;
	} else {
		/* Found. */
		retval = TRUE;
	}
	stiloe_p_unlock(&name->stiloe);

	return retval;
}

/* Remove a keyed reference. */
static cw_bool_t
stilo_p_name_kref_remove(cw_stilo_t *a_stilo, cw_stilt_t *a_stilt, const
    cw_stiloe_dict_t *a_dict)
{
	cw_bool_t		retval;
	cw_chi_t		*chi;
	cw_stiloe_name_t	*name;

	/* Chase down the name. */
	name = (cw_stiloe_name_t *)a_stilo->o.stiloe;
	name = name->val;

	stiloe_p_lock(&name->stiloe);
	_cw_check_ptr(name->e.n.keyed_refs);

	if ((retval = dch_remove(name->e.n.keyed_refs, (void *)a_dict, NULL,
	    NULL, &chi)) == FALSE)
		/* XXX Need stilag, not a_stilt. */
		_cw_stilt_chi_put(a_stilt, chi);

	/* If there are no more keyed references, delete the hash. */
	if (dch_count(name->e.n.keyed_refs) == 0) {
		dch_delete(name->e.n.keyed_refs);
		name->e.n.keyed_refs = NULL;
	}
	stiloe_p_unlock(&name->stiloe);

	return retval;
}

/*
 * null.
 */
void
stilo_null_new(cw_stilo_t *a_stilo)
{
	stilo_p_new(a_stilo, _CW_STILOT_NULLTYPE);
}

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
void
stilo_number_new(cw_stilo_t *a_stilo, cw_stilt_t *a_stilt)
{
	stilo_p_new(a_stilo, _CW_STILOT_NUMBERTYPE);
}

static void
stilo_p_number_delete(cw_stilo_t *a_stilo, cw_stilt_t *a_stilt)
{
}

static cw_stiloe_t *
stiloe_p_number_ref_iterate(cw_stiloe_t *a_stilo, cw_bool_t a_reset)
{
	/* Numbers never have any references. */
	return NULL;
}

static void
stilo_p_number_cast(cw_stilo_t *a_stilo, cw_stilot_t a_type)
{
}

static void
stilo_p_number_copy(cw_stilo_t *a_to, cw_stilo_t *a_from, cw_stilt_t *a_stilt)
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
void
stilo_operator_new(cw_stilo_t *a_stilo, cw_op_t *a_op)
{
	stilo_p_new(a_stilo, _CW_STILOT_OPERATORTYPE);
	a_stilo->o.operator.f = a_op;
}

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
void
stilo_string_new(cw_stilo_t *a_stilo, cw_stilt_t *a_stilt, cw_uint32_t a_len)
{
	cw_stiloe_string_t	*string;

	stilo_p_new(a_stilo, _CW_STILOT_STRINGTYPE);

	string = (cw_stiloe_string_t *)_cw_stilt_malloc(a_stilt,
	    sizeof(cw_stiloe_string_t));

	string->iterations = 0;
	string->e.s.len = a_len;

	if (string->e.s.len > 0) {
		string->e.s.str = (cw_uint8_t
		    *)_cw_stilt_malloc(a_stilt, string->e.s.len);
	} else
		string->e.s.str = NULL;

	a_stilo->o.stiloe = (cw_stiloe_t *)string;

	stiloe_p_new(a_stilo->o.stiloe, _CW_STILOT_STRINGTYPE);
}

static void
stilo_p_string_delete(cw_stilo_t *a_stilo, cw_stilt_t *a_stilt)
{
	cw_stiloe_string_t	*string;

	string = (cw_stiloe_string_t *)a_stilo->o.stiloe;
	_cw_check_ptr(string);
	_cw_assert(string->stiloe.magic == _CW_STILOE_MAGIC);

	if (string->stiloe.indirect == FALSE)
		_cw_stilt_free(a_stilt, string->e.s.str);

	stiloe_p_delete(&string->stiloe, a_stilt);
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
stilo_p_string_copy(cw_stilo_t *a_to, cw_stilo_t *a_from, cw_stilt_t *a_stilt)
{
}

static void
stilo_p_string_print(cw_stilo_t *a_stilo, cw_sint32_t a_fd, cw_bool_t
    a_syntactic, cw_bool_t a_newline)
{
	cw_uint8_t	newline = (a_newline) ? '\n' : '\0';
	cw_uint8_t	*str;
	cw_sint32_t	len;
	cw_uint32_t	i;

	str = stilo_string_get(a_stilo);
	len = stilo_string_len_get(a_stilo);

	if (a_syntactic) {
		_cw_out_put_f(a_fd, "\"");
		for (i = 0; i < len; i++) {
			switch (str[i]) {
			case '\n':
				_cw_out_put_f(a_fd, "\\n");
				break;
			case '\r':
				_cw_out_put_f(a_fd, "\\r");
				break;
			case '\t':
				_cw_out_put_f(a_fd, "\\t");
				break;
			case '\b':
				_cw_out_put_f(a_fd, "\\b");
				break;
			case '\f':
				_cw_out_put_f(a_fd, "\\f");
				break;
			case '\\':
				_cw_out_put_f(a_fd, "\\\\");
				break;
			case '"':
				_cw_out_put_f(a_fd, "\\\"");
				break;
			default:
				if (isprint(str[i]))
					_cw_out_put_f(a_fd, "[c]", str[i]);
				else {
					_cw_out_put_f(a_fd, "\\x[i|b:16]",
					    str[i]);
				}
				break;
			}
		}
		_cw_out_put_f(a_fd, "\"");
	} else {
		if (len > 0)
			_cw_out_put_fn(a_fd, len, "[s]", str);
	}
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

	if (string->stiloe.indirect == FALSE)
		retval = string->e.s.str;
	else {
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
