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
#include <errno.h>
#include <fcntl.h>
#include <sys/stat.h>

#ifdef _LIBSTIL_DBG
#define _CW_STILOE_MAGIC	0x0fa6e798
#endif

typedef struct cw_stiloe_array_s cw_stiloe_array_t;
typedef struct cw_stiloe_condition_s cw_stiloe_condition_t;
typedef struct cw_stiloe_file_s cw_stiloe_file_t;
typedef struct cw_stiloe_hook_s cw_stiloe_hook_t;
typedef struct cw_stiloe_lock_s cw_stiloe_lock_t;
typedef struct cw_stiloe_name_s cw_stiloe_name_t;
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

struct cw_stiloe_file_s {
	cw_stiloe_t	stiloe;
	cw_stilt_t	*stilt;

	/* File descriptor number.  -1: Invalid, -2: Wrapped. */
	cw_sint32_t	fd;

	/* Buffering. */
	cw_uint8_t	*buffer;
	cw_uint32_t	buffer_size;
	enum {
		BUFFER_EMPTY,
		BUFFER_READ,
		BUFFER_WRITE
	}		buffer_mode;
	cw_uint32_t	buffer_offset;

	/* Used for wrapped files. */
	cw_stil_read_t	*read_f;
	cw_stil_write_t	*write_f;
	void		*arg;
	cw_sint64_t	position;
};

struct cw_stiloe_hook_s {
	cw_stiloe_t	stiloe;
	void		*data;
	void		(*exec) (void *);
	void		(*copy) (void *);
	void		(*get) (void *);
	void		(*dealloc) (void *);
	cw_stiloe_t	*(*ref_iterator) (void *);
};

struct cw_stiloe_lock_s {
	cw_stiloe_t	stiloe;
	cw_mtx_t	lock;
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
/* Call before other initialization. */
#ifdef _LIBSTIL_DBG
#define	stilo_p_new(a_stilo, a_type) do {				\
	memset((a_stilo), 0, sizeof(cw_stilo_t));			\
	(a_stilo)->type = (a_type);					\
	(a_stilo)->magic = _CW_STILO_MAGIC;				\
} while (0)
#else
#define	stilo_p_new(a_stilo, a_type) do {				\
	memset((a_stilo), 0, sizeof(cw_stilo_t));			\
	(a_stilo)->type = (a_type);					\
} while (0)
#endif
static cw_uint32_t stilo_p_hash(const void *a_key);
static cw_bool_t stilo_p_key_comp(const void *a_k1, const void *a_k2);

/* stiloe. */
static void	stiloe_p_new(cw_stiloe_t *a_stiloe, cw_stilot_t a_type);
/*  static void	stiloe_p_delete(cw_stiloe_t *a_stiloe, cw_stilt_t *a_stilt); */

#define		stiloe_p_lock(a_stiloe) do {				\
	if ((a_stiloe)->global)						\
		mtx_lock(&(a_stiloe)->lock);				\
} while (0)

#define		stiloe_p_unlock(a_stiloe) do {				\
	if ((a_stiloe)->global)						\
		mtx_unlock(&(a_stiloe)->lock);				\
} while (0)

/* no. */
static cw_stilte_t stilo_p_no_print(cw_stilo_t *a_stilo, cw_stilt_t *a_stilt,
    cw_stilo_t *a_file, cw_bool_t a_syntactic, cw_bool_t a_newline);

/* array. */
static void	stilo_p_array_delete(cw_stilo_t *a_stilo, cw_stilt_t *a_stilt);
static cw_stiloe_t *stiloe_p_array_ref_iterate(cw_stiloe_t *a_stiloe, cw_bool_t
    a_reset);
static cw_stilte_t stilo_p_array_print(cw_stilo_t *a_stilo, cw_stilt_t *a_stilt,
    cw_stilo_t *a_file, cw_bool_t a_syntactic, cw_bool_t a_newline);

/* boolean. */
static cw_stilte_t stilo_p_boolean_print(cw_stilo_t *a_stilo, cw_stilt_t
    *a_stilt, cw_stilo_t *a_file, cw_bool_t a_syntactic, cw_bool_t a_newline);

/* condition. */
static void	stilo_p_condition_delete(cw_stilo_t *a_stilo, cw_stilt_t
    *a_stilt);
static cw_stiloe_t *stiloe_p_condition_ref_iterate(cw_stiloe_t *a_stiloe,
    cw_bool_t a_reset);
static cw_stilte_t stilo_p_condition_print(cw_stilo_t *a_stilo, cw_stilt_t
    *a_stilt, cw_stilo_t *a_file, cw_bool_t a_syntactic, cw_bool_t a_newline);

/* dict. */
static void	stilo_p_dict_delete(cw_stilo_t *a_stilo, cw_stilt_t *a_stilt);
static cw_stiloe_t *stiloe_p_dict_ref_iterate(cw_stiloe_t *a_stiloe, cw_bool_t
    a_reset);
static cw_stilte_t stilo_p_dict_print(cw_stilo_t *a_stilo, cw_stilt_t *a_stilt,
    cw_stilo_t *a_file, cw_bool_t a_syntactic, cw_bool_t a_newline);

/* file. */
static void	stilo_p_file_delete(cw_stilo_t *a_stilo, cw_stilt_t *a_stilt);
static cw_stiloe_t *stiloe_p_file_ref_iterate(cw_stiloe_t *a_stiloe, cw_bool_t
    a_reset);
static cw_stilte_t stilo_p_file_print(cw_stilo_t *a_stilo, cw_stilt_t *a_stilt,
    cw_stilo_t *a_file, cw_bool_t a_syntactic, cw_bool_t a_newline);

/* hook. */
static void	stilo_p_hook_delete(cw_stilo_t *a_stilo, cw_stilt_t *a_stilt);
static cw_stiloe_t *stiloe_p_hook_ref_iterate(cw_stiloe_t *a_stiloe, cw_bool_t
    a_reset);
static cw_stilte_t stilo_p_hook_print(cw_stilo_t *a_stilo, cw_stilt_t *a_stilt,
    cw_stilo_t *a_file, cw_bool_t a_syntactic, cw_bool_t a_newline);

/* integer. */
static cw_stilte_t stilo_p_integer_print(cw_stilo_t *a_stilo, cw_stilt_t
    *a_stilt, cw_stilo_t *a_file, cw_bool_t a_syntactic, cw_bool_t a_newline);

/* lock. */
static void	stilo_p_lock_delete(cw_stilo_t *a_stilo, cw_stilt_t *a_stilt);
static cw_stiloe_t *stiloe_p_lock_ref_iterate(cw_stiloe_t *a_stiloe, cw_bool_t
    a_reset);
static cw_stilte_t stilo_p_lock_print(cw_stilo_t *a_stilo, cw_stilt_t *a_stilt,
    cw_stilo_t *a_file, cw_bool_t a_syntactic, cw_bool_t a_newline);

/* mark. */
static cw_stilte_t stilo_p_mark_print(cw_stilo_t *a_stilo, cw_stilt_t *a_stilt,
    cw_stilo_t *a_file, cw_bool_t a_syntactic, cw_bool_t a_newline);

/* name. */
static void	stilo_p_name_delete(cw_stilo_t *a_stilo, cw_stilt_t *a_stilt);
static cw_stiloe_t *stiloe_p_name_ref_iterate(cw_stiloe_t *a_stiloe, cw_bool_t
    a_reset);
static cw_stilte_t stilo_p_name_print(cw_stilo_t *a_stilo, cw_stilt_t *a_stilt,
    cw_stilo_t *a_file, cw_bool_t a_syntactic, cw_bool_t a_newline);

static cw_stiloe_name_t *stilo_p_name_gref(cw_stilt_t *a_stilt, const char
    *a_str, cw_uint32_t a_len, cw_bool_t a_is_static);
static void stilo_p_name_kref_insert(cw_stilo_t *a_stilo, cw_stilt_t
    *a_stilt, const cw_stiloe_dict_t *a_dict);
static cw_bool_t stilo_p_name_kref_search(const cw_stilo_t *a_stilo, const
    cw_stiloe_dict_t *a_dict);
static cw_bool_t stilo_p_name_kref_remove(const cw_stilo_t *a_stilo, cw_stilt_t
    *a_stilt, const cw_stiloe_dict_t *a_dict);

/* null. */
static cw_stilte_t stilo_p_null_print(cw_stilo_t *a_stilo, cw_stilt_t *a_stilt,
    cw_stilo_t *a_file, cw_bool_t a_syntactic, cw_bool_t a_newline);

/* operator. */
static cw_stilte_t stilo_p_operator_print(cw_stilo_t *a_stilo, cw_stilt_t
    *a_stilt, cw_stilo_t *a_file, cw_bool_t a_syntactic, cw_bool_t a_newline);

/* string. */
static void	stilo_p_string_delete(cw_stilo_t *a_stilo, cw_stilt_t *a_stilt);
static cw_stiloe_t *stiloe_p_string_ref_iterate(cw_stiloe_t *a_stiloe, cw_bool_t
    a_reset);
static cw_stilte_t stilo_p_string_print(cw_stilo_t *a_stilo, cw_stilt_t
    *a_stilt, cw_stilo_t *a_file, cw_bool_t a_syntactic, cw_bool_t a_newline);

/*
 * vtable setup for the various operations on stilo's that are polymorphic.
 */
typedef void		cw_stilot_delete_t(cw_stilo_t *a_stilo, cw_stilt_t
    *a_stilt);
typedef cw_stiloe_t	*cw_stilot_ref_iterate_t(cw_stiloe_t *a_stiloe,
    cw_bool_t a_reset);
typedef cw_stilte_t	cw_stilot_copy_t(cw_stilo_t *a_to, cw_stilo_t *a_from,
    cw_stilt_t *a_stilt);
typedef cw_stilte_t	cw_stilot_print_t(cw_stilo_t *a_stilo, cw_stilt_t
    *a_stilt, cw_stilo_t *a_file, cw_bool_t a_syntactic, cw_bool_t a_newline);

typedef struct cw_stilot_vtable_s cw_stilot_vtable_t;
struct  cw_stilot_vtable_s {
	cw_stilot_delete_t	*delete_f;
	cw_stilot_ref_iterate_t	*ref_iterate_f;
	cw_stilot_print_t	*print_f;
};

/*
 * The order of these entries must correspond to the type numbering of
 * cw_stilot_t.  NULL pointers are used in entries that should never get called,
 * so that a segfault will occur if such a non-existent function is called.
 */
static cw_stilot_vtable_t stilot_vtable[] = {
	/* STILOT_NO */
	{NULL,
	 NULL,
	 stilo_p_no_print},	/* XXX Debugging only. */

	/* STILOT_ARRAY */
	{stilo_p_array_delete,
	 stiloe_p_array_ref_iterate,
	 stilo_p_array_print},

	/* STILOT_BOOLEAN */
	{NULL,
	 NULL,
	 stilo_p_boolean_print},

	/* STILOT_CONDITION */
	{stilo_p_condition_delete,
	 stiloe_p_condition_ref_iterate,
	 stilo_p_condition_print},
	
	/* STILOT_DICT */
	{stilo_p_dict_delete,
	 stiloe_p_dict_ref_iterate,
	 stilo_p_dict_print},

	/* STILOT_FILE */
	{stilo_p_file_delete,
	 stiloe_p_file_ref_iterate,
	 stilo_p_file_print},

	/* STILOT_HOOK */
	{stilo_p_hook_delete,
	 stiloe_p_hook_ref_iterate,
	 stilo_p_hook_print},

	/* STILOT_INTEGER */
	{NULL,
	 NULL,
	 stilo_p_integer_print},

	/* STILOT_LOCK */
	{stilo_p_lock_delete,
	 stiloe_p_lock_ref_iterate,
	 stilo_p_lock_print},

	/* STILOT_MARK */
	{NULL,
	 NULL,
	 stilo_p_mark_print},

	/* STILOT_NAME */
	{stilo_p_name_delete,
	 stiloe_p_name_ref_iterate,
	 stilo_p_name_print},

	/* STILOT_NULL */
	{NULL,
	 NULL,
	 stilo_p_null_print},

	/* STILOT_OPERATOR */
	{NULL,
	 NULL,
	 stilo_p_operator_print},

	/* STILOT_STRING */
	{stilo_p_string_delete,
	 stiloe_p_string_ref_iterate,
	 stilo_p_string_print}
};

/*
 * stilo.
 */
void
stilo_delete(cw_stilo_t *a_stilo, cw_stilt_t *a_stilt)
{
	_cw_check_ptr(a_stilo);
	_cw_assert(a_stilo->magic == _CW_STILO_MAGIC);

	if (stilot_vtable[a_stilo->type].delete_f != NULL)
		stilot_vtable[a_stilo->type].delete_f(a_stilo, a_stilt);
}

cw_sint32_t
stilo_compare(cw_stilo_t *a_a, cw_stilo_t *a_b, cw_stilt_t *a_stilt)
{
	cw_sint32_t	retval;

	switch (a_a->type) {
	case STILOT_ARRAY:
	case STILOT_CONDITION:
	case STILOT_DICT:
	case STILOT_FILE:
	case STILOT_HOOK:
	case STILOT_LOCK:
	case STILOT_OPERATOR:
		if (a_a->type == a_b->type && a_a->o.stiloe == a_b->o.stiloe)
			retval = 0;
		else
			retval = 2;
		break;
	case STILOT_NAME:
	case STILOT_STRING: {
		const cw_uint8_t	*str_a, *str_b;
		cw_uint32_t		len_a, len_b;

		if (a_a->type == STILOT_NAME) {
			str_a = stilo_name_str_get(a_a);
			len_a = stilo_name_len_get(a_a);
		} else {
			str_a = stilo_string_get(a_a);
			len_a = stilo_string_len_get(a_a);
		}
			
		if (a_b->type == STILOT_NAME) {
			str_b = stilo_name_str_get(a_b);
			len_b = stilo_name_len_get(a_b);
		} else if (a_b->type == STILOT_STRING) {
			str_b = stilo_string_get(a_b);
			len_b = stilo_string_len_get(a_b);
		} else {
			retval = 2;
			break;
		}

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

cw_bool_t
stilo_global_get(cw_stilo_t *a_stilo)
{
	cw_bool_t	retval;

	_cw_check_ptr(a_stilo);
	_cw_assert(a_stilo->magic == _CW_STILO_MAGIC);

#ifdef _LIBSTIL_DBG
	switch (a_stilo->type) {
	case STILOT_ARRAY:
	case STILOT_CONDITION:
	case STILOT_DICT:
	case STILOT_FILE:
	case STILOT_HOOK:
	case STILOT_LOCK:
	case STILOT_NAME:
	case STILOT_STRING:
		retval = a_stilo->o.stiloe->global;
		break;
	default:
		_cw_not_reached();
	}
#else
	retval = a_stilo->o.stiloe->global;
#endif

	return retval;
}

cw_stilte_t
stilo_print(cw_stilo_t *a_stilo, cw_stilt_t *a_stilt, cw_stilo_t *a_file,
    cw_bool_t a_syntactic, cw_bool_t a_newline)
{
	return stilot_vtable[a_stilo->type].print_f(a_stilo, a_stilt, a_file,
	    a_syntactic, a_newline);
}

cw_stiloe_t *
stilo_l_stiloe_get(cw_stilo_t *a_stilo)
{
	cw_stiloe_t	*retval;

	_cw_check_ptr(a_stilo);
	_cw_assert(a_stilo->magic == _CW_STILO_MAGIC);

	switch (a_stilo->type) {
	case STILOT_ARRAY:
	case STILOT_CONDITION:
	case STILOT_DICT:
	case STILOT_HOOK:
	case STILOT_LOCK:
	case STILOT_STRING:
		retval = a_stilo->o.stiloe;
		break;
	default:
		retval = NULL;
	}

	return retval;
}

/* Hash any stilo, but optimize for name hashing. */
static cw_uint32_t
stilo_p_hash(const void *a_key)
{
	cw_uint32_t	retval;
	cw_stilo_t	*key = (cw_stilo_t *)a_key;

	_cw_check_ptr(key);
	_cw_assert(key->magic == _CW_STILO_MAGIC);

	if (key->type == STILOT_NAME) {
		cw_stiloe_name_t	*name;

		name = (cw_stiloe_name_t *)key->o.stiloe;
		retval = ch_direct_hash((void *)name->val);
		/* XXX Assumes 32 bit architecture. */
/*  		retval = (cw_uint32_t)name->val; */
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

	if ((k1->type == STILOT_NAME) && (k1->type ==
	    STILOT_NAME)) {
		cw_stiloe_name_t	*n1, *n2;
		
		/* Chase down the names. */
		n1 = (cw_stiloe_name_t *)k1->o.stiloe;
		n1 = n1->val;
		n2 = (cw_stiloe_name_t *)k2->o.stiloe;
		n2 = n2->val;

		retval = (n1 == n2) ? TRUE : FALSE;
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

#if (0)
static void
stiloe_p_delete(cw_stiloe_t *a_stiloe, cw_stilt_t *a_stilt)
{
	mtx_delete(&a_stiloe->lock);
	stilt_free(a_stilt, a_stiloe);
}
#endif

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
static cw_stilte_t
stilo_p_no_print(cw_stilo_t *a_stilo, cw_stilt_t *a_stilt, cw_stilo_t *a_file,
    cw_bool_t a_syntactic, cw_bool_t a_newline)
{
	cw_stilte_t	retval;
	cw_uint8_t	newline = (a_newline) ? '\n' : '\0';

	if (a_syntactic)
		retval = stilo_file_output(a_file, a_stilt, "-notype-[c]",
		    newline);
	else {
		retval = stilo_file_output(a_file, a_stilt,
		    "--nostringval--[c]", newline);
	}

	return retval;
}

/*
 * array.
 */
void
stilo_array_new(cw_stilo_t *a_stilo, cw_stilt_t *a_stilt, cw_uint32_t a_len)
{
	cw_stiloe_array_t	*array;
	cw_uint32_t		i;

	stilo_p_new(a_stilo, STILOT_ARRAY);

	array = (cw_stiloe_array_t *)stilt_malloc(a_stilt,
	    sizeof(cw_stiloe_array_t));

	array->iterations = 0;
	array->e.a.len = a_len;
	if (array->e.a.len > 0) {
		array->e.a.arr = (cw_stilo_t *)stilt_malloc(a_stilt,
		    sizeof(cw_stilo_t) * array->e.a.len);
		for (i = 0; i < array->e.a.len; i++)
			stilo_null_new(&array->e.a.arr[i]);
	}
	a_stilo->o.stiloe = (cw_stiloe_t *)array;

	stiloe_p_new(a_stilo->o.stiloe, STILOT_ARRAY);
}

cw_stilte_t
stilo_array_subarray_new(cw_stilo_t *a_stilo, cw_stilo_t *a_array, cw_stilt_t
    *a_stilt, cw_uint32_t a_offset, cw_uint32_t a_len)
{
	cw_stilte_t		retval;
	cw_stiloe_array_t	*array, *orig;

	orig = (cw_stiloe_array_t *)a_array->o.stiloe;
	_cw_check_ptr(orig);
	_cw_assert(orig->stiloe.magic == _CW_STILOE_MAGIC);
	if (a_offset + a_len > orig->e.a.len) {
		retval = STILTE_RANGECHECK;
		goto RETURN;
	}

	stilo_p_new(a_stilo, STILOT_ARRAY);
	array = (cw_stiloe_array_t *)stilt_malloc(a_stilt,
	    sizeof(cw_stiloe_array_t));
	a_stilo->o.stiloe = (cw_stiloe_t *)array;

	array->iterations = 0;
	memcpy(&array->e.i.stilo, a_array, sizeof(cw_stilo_t));

	array->e.i.beg_offset = a_offset;
	array->e.i.len = a_len;

	memcpy(&array->stiloe, &orig->stiloe, sizeof(cw_stiloe_t));
	array->stiloe.watchpoint = FALSE;
	array->stiloe.indirect = TRUE;

	retval = STILTE_NONE;
	RETURN:
	return retval;
}

static void
stilo_p_array_delete(cw_stilo_t *a_stilo, cw_stilt_t *a_stilt)
{
	cw_stiloe_array_t	*array;

	array = (cw_stiloe_array_t *)a_stilo->o.stiloe;
	_cw_check_ptr(array);
	_cw_assert(array->stiloe.magic == _CW_STILOE_MAGIC);

	if (array->e.a.len > 0)
		stilt_free(a_stilt, array->e.a.arr);

/*  	stiloe_p_delete(&array->stiloe, a_stilt); */
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

cw_stilte_t
stilo_array_copy(cw_stilo_t *a_to, cw_stilo_t *a_from, cw_stilt_t *a_stilt)
{
	cw_stilte_t		retval;
	cw_stiloe_array_t	*array;
	cw_stilo_t		*arr_from, *arr_to;
	cw_uint32_t		i, len;

	array = (cw_stiloe_array_t *)a_from->o.stiloe;

	/*
	 * Set arr_from and len according to whether this is an indirect object.
	 */
	if (array->stiloe.indirect) {
		cw_stiloe_array_t	*array_indir;

		array_indir = (cw_stiloe_array_t *)array->e.i.stilo.o.stiloe;

		arr_from =
		    &array_indir->e.a.arr[array->e.i.beg_offset];
		len = array->e.i.len;
		_cw_assert(len + array->e.i.beg_offset <=
		    array_indir->e.a.len);
	} else {
		arr_from = array->e.a.arr;
		len = array->e.a.len;
	}

	arr_to = ((cw_stiloe_array_t *)a_to->o.stiloe)->e.a.arr;

	/* Make sure destination is large enough. */
	if (((cw_stiloe_array_t *)a_to->o.stiloe)->e.a.len < len) {
		retval = STILTE_RANGECHECK;
		goto RETURN;
	}

	/*
	 * Iteratively copy elements.  Only copy one level deep (not
	 * recursively), by using dup.
	 */
	for (i = 0; i < len; i++)
		stilo_dup(&arr_to[i], &arr_from[i]);

	retval = STILTE_NONE;
	RETURN:
	return retval;
}

static cw_stilte_t
stilo_p_array_print(cw_stilo_t *a_stilo, cw_stilt_t *a_stilt, cw_stilo_t
    *a_file, cw_bool_t a_syntactic, cw_bool_t a_newline)
{
	cw_stilte_t	retval;
	cw_uint8_t	newline = (a_newline) ? '\n' : '\0';
	cw_stilo_t	*arr;
	cw_uint32_t	nelms, i;

	arr = stilo_array_get(a_stilo);
	nelms = stilo_array_len_get(a_stilo);

	if (a_syntactic) {
		if (a_stilo->attrs == STILOA_EXECUTABLE) {
			retval = stilo_file_output(a_file, a_stilt, "{");
			if (retval)
				goto RETURN;
		} else {
			retval = stilo_file_output(a_file, a_stilt, "[[");
			if (retval)
				goto RETURN;
		}
		for (i = 0; i < nelms; i++) {
			retval = stilo_print(&arr[i], a_stilt, a_file,
			    a_syntactic, FALSE);
			if (retval)
				goto RETURN;

			if (i < nelms - 1) {
				retval = stilo_file_output(a_file, a_stilt,
				    " ");
				if (retval)
					goto RETURN;
			}
		}
		if (a_stilo->attrs == STILOA_EXECUTABLE) {
			retval = stilo_file_output(a_file, a_stilt, "}[c]",
			    newline);
			if (retval)
				goto RETURN;
		} else {
			retval = stilo_file_output(a_file, a_stilt, "][c]",
			    newline);
			if (retval)
				goto RETURN;
		}
	} else {
		retval = stilo_file_output(a_file, a_stilt,
		    "--nostringval--[c]", newline);
		if (retval)
			goto RETURN;
	}

	retval = STILTE_NONE;
	RETURN:
	return retval;
}

cw_uint32_t
stilo_array_len_get(cw_stilo_t *a_stilo)
{
	cw_uint32_t		retval;
	cw_stiloe_array_t	*array;

	_cw_check_ptr(a_stilo);
	_cw_assert(a_stilo->magic == _CW_STILO_MAGIC);

	array = (cw_stiloe_array_t *)a_stilo->o.stiloe;
	_cw_assert(array->stiloe.magic == _CW_STILOE_MAGIC);
	_cw_assert(array->stiloe.type == STILOT_ARRAY);

	if (array->stiloe.indirect == FALSE)
		retval = array->e.a.len;
	else
		retval = array->e.i.len;

	return retval;
}

cw_stilo_t *
stilo_array_el_get(cw_stilo_t *a_stilo, cw_sint64_t a_offset)
{
	cw_stilo_t		*retval;
	cw_stiloe_array_t	*array;

	_cw_check_ptr(a_stilo);
	_cw_assert(a_stilo->magic == _CW_STILO_MAGIC);
	_cw_assert(a_stilo->type == STILOT_ARRAY);

	array = (cw_stiloe_array_t *)a_stilo->o.stiloe;
	_cw_assert(array->stiloe.magic == _CW_STILOE_MAGIC);

	if (array->stiloe.indirect == FALSE) {
		if (a_offset >= array->e.a.len || a_offset < 0) {
			retval = NULL;
			goto RETURN;
		}
		retval = &array->e.a.arr[a_offset];
	} else {
		retval = stilo_array_el_get(&array->e.i.stilo, a_offset +
		    array->e.i.beg_offset);
	}

	RETURN:
	return retval;
}

cw_stilte_t
stilo_array_el_set(cw_stilo_t *a_stilo, cw_stilo_t *a_el, cw_sint64_t a_offset)
{
	cw_stilte_t		retval;
	cw_stiloe_array_t	*array;

	_cw_check_ptr(a_stilo);
	_cw_assert(a_stilo->magic == _CW_STILO_MAGIC);
	_cw_assert(a_stilo->type == STILOT_ARRAY);

	array = (cw_stiloe_array_t *)a_stilo->o.stiloe;
	_cw_assert(array->stiloe.magic == _CW_STILOE_MAGIC);

	if (array->stiloe.indirect == FALSE) {
		if (a_offset >= array->e.a.len || a_offset < 0) {
			retval = STILTE_RANGECHECK;
			goto RETURN;
		}
		stilo_dup(&array->e.a.arr[a_offset], a_el);
		retval = STILTE_NONE;
	} else {
		retval = stilo_array_el_set(&array->e.i.stilo, a_el, a_offset +
		    array->e.i.beg_offset);
	}

	RETURN:
	return retval;
}

cw_stilo_t *
stilo_array_get(cw_stilo_t *a_stilo)
{
	cw_stilo_t		*retval;
	cw_stiloe_array_t	*array;

	_cw_check_ptr(a_stilo);
	_cw_assert(a_stilo->magic == _CW_STILO_MAGIC);
	_cw_assert(a_stilo->type == STILOT_ARRAY);

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

cw_stilte_t
stilo_array_set(cw_stilo_t *a_stilo, cw_uint32_t a_offset, cw_stilo_t *a_arr,
    cw_uint32_t a_len)
{
	cw_stilte_t		retval;
	cw_stiloe_array_t	*array;

	_cw_check_ptr(a_stilo);
	_cw_assert(a_stilo->magic == _CW_STILO_MAGIC);
	_cw_assert(a_stilo->type == STILOT_ARRAY);

	array = (cw_stiloe_array_t *)a_stilo->o.stiloe;
	_cw_assert(array->stiloe.magic == _CW_STILOE_MAGIC);

	/* Get the array pointer. */
	if (array->stiloe.indirect == FALSE) {
		cw_stilo_t	*arr;
		cw_uint32_t	i;

		if (a_offset + a_len > array->e.a.len) {
			retval = STILTE_RANGECHECK;
			goto RETURN;
		}
		arr = array->e.a.arr;

		/* Set the array. */
		for (i = 0; i < a_len; i++) {
			/* XXX Check local/global allocation. */
			stilo_dup(&arr[i + a_offset], &a_arr[i]);
		}
		retval = STILTE_NONE;
	} else {
		retval = stilo_array_set(&array->e.i.stilo, a_offset, a_arr,
		    a_len);
	}

	RETURN:
	return retval;
}

/*
 * boolean.
 */
void
stilo_boolean_new(cw_stilo_t *a_stilo, cw_bool_t a_val)
{
	stilo_p_new(a_stilo, STILOT_BOOLEAN);
	a_stilo->o.boolean.val = a_val;
}


static cw_stilte_t
stilo_p_boolean_print(cw_stilo_t *a_stilo, cw_stilt_t *a_stilt, cw_stilo_t
    *a_file, cw_bool_t a_syntactic, cw_bool_t a_newline)
{
	cw_stilte_t	retval;
	cw_uint8_t	newline = (a_newline) ? '\n' : '\0';

	if (a_stilo->o.boolean.val)
		retval = stilo_file_output(a_file, a_stilt, "true[c]", newline);
	else {
		retval = stilo_file_output(a_file, a_stilt, "false[c]",
		    newline);
	}

	return retval;
}

cw_bool_t
stilo_boolean_get(cw_stilo_t *a_stilo)
{
	_cw_check_ptr(a_stilo);
	_cw_assert(a_stilo->magic == _CW_STILO_MAGIC);

	return a_stilo->o.boolean.val;
}

void
stilo_boolean_set(cw_stilo_t *a_stilo, cw_bool_t a_val)
{
	_cw_check_ptr(a_stilo);
	_cw_assert(a_stilo->magic == _CW_STILO_MAGIC);
	_cw_assert(a_stilo->type == STILOT_BOOLEAN);

	a_stilo->o.boolean.val = a_val;
}

/*
 * condition.
 */
void
stilo_condition_new(cw_stilo_t *a_stilo, cw_stilt_t *a_stilt)
{
	stilo_p_new(a_stilo, STILOT_CONDITION);
}

static void
stilo_p_condition_delete(cw_stilo_t *a_stilo, cw_stilt_t *a_stilt)
{
	_cw_error("XXX Not implemented");
}

static cw_stiloe_t *
stiloe_p_condition_ref_iterate(cw_stiloe_t *a_stilo, cw_bool_t a_reset)
{
	_cw_error("XXX Not implemented");
	return NULL;	/* XXX */
}

static cw_stilte_t
stilo_p_condition_print(cw_stilo_t *a_stilo, cw_stilt_t *a_stilt, cw_stilo_t
    *a_file, cw_bool_t a_syntactic, cw_bool_t a_newline)
{
	cw_stilte_t	retval;
	cw_uint8_t	newline = (a_newline) ? '\n' : '\0';

	if (a_syntactic) {
		retval = stilo_file_output(a_file, a_stilt, "-condition-[c]",
		    newline);
	} else {
		retval = stilo_file_output(a_file, a_stilt,
		    "--nostringval--[c]", newline);
	}

	return retval;
}

/*
 * dict.
 */

void
stilo_dict_new(cw_stilo_t *a_stilo, cw_stilt_t *a_stilt, cw_uint32_t
    a_dict_size)
{
	cw_stiloe_dict_t	*dict;

	stilo_p_new(a_stilo, STILOT_DICT);

	dict = (cw_stiloe_dict_t *)stilt_malloc(a_stilt,
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

	stiloe_p_new(a_stilo->o.stiloe, STILOT_DICT);
}

static void
stilo_p_dict_delete(cw_stilo_t *a_stilo, cw_stilt_t *a_stilt)
{
	cw_stiloe_dict_t	*dict;

	_cw_check_ptr(a_stilo);
	_cw_assert(a_stilo->magic == _CW_STILO_MAGIC);

	dict = (cw_stiloe_dict_t *)a_stilo->o.stiloe;

	_cw_check_ptr(dict);
	_cw_assert(dict->stiloe.magic == _CW_STILOE_MAGIC);

	if (dict->stiloe.indirect == FALSE) {
		cw_stiloe_dicto_t	*dicto;
		cw_chi_t		*chi;

		while (dch_remove_iterate(&dict->e.d.hash, NULL, (void
		    **)&dicto, &chi) == FALSE) {
			stilt_chi_put(a_stilt, chi);
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

cw_stilte_t
stilo_dict_copy(cw_stilo_t *a_to, cw_stilo_t *a_from, cw_stilt_t *a_stilt)
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
		dicto_to = stilt_dicto_get(a_stilt);
		stilo_no_new(&dicto_to->key);
		stilo_dup(&dicto_to->key, &dicto_from->key);
		stilo_no_new(&dicto_to->val);
		stilo_dup(&dicto_to->val, &dicto_from->val);
		chi = stilt_chi_get(a_stilt);

		/* Insert. */
		dch_insert(&to->e.d.hash, &dicto_to->key, dicto_to, chi);
	}
	stiloe_p_unlock(&from->stiloe);

	return STILTE_NONE;
}

static cw_stilte_t
stilo_p_dict_print(cw_stilo_t *a_stilo, cw_stilt_t *a_stilt, cw_stilo_t *a_file,
    cw_bool_t a_syntactic, cw_bool_t a_newline)
{
	cw_stilte_t	retval;
	cw_uint8_t	newline = (a_newline) ? '\n' : '\0';
	
	if (a_syntactic) {
		retval = stilo_file_output(a_file, a_stilt, "-dict-[c]",
		    newline);
	} else {
		retval = stilo_file_output(a_file, a_stilt,
		    "--nostringval--[c]", newline);
	}

	return retval;
}

void
stilo_dict_def(cw_stilo_t *a_stilo, cw_stilt_t *a_stilt, cw_stilo_t *a_key,
    cw_stilo_t *a_val)
{
	cw_stiloe_dict_t	*dict;

	_cw_check_ptr(a_stilo);
	_cw_assert(a_stilo->magic == _CW_STILO_MAGIC);
	_cw_assert(a_stilo->type == STILOT_DICT);
	/* XXX Make sure setglobal is correct. */
	/* XXX Make sure value isn't local if dict is global. */

	dict = (cw_stiloe_dict_t *)a_stilo->o.stiloe;

	if (dict->stiloe.indirect)
		stilo_dict_def(&dict->e.i.stilo, a_stilt, a_key, a_val);
	else {
		cw_stiloe_dicto_t	*dicto;

		stiloe_p_lock(&dict->stiloe);
		if (dch_search(&dict->e.d.hash, (void *)a_key, (void **)&dicto)
		    == FALSE) {
			/* a_key is already defined. */
			stilo_dup(&dicto->val, a_val);

			/*
			 * If (a_key == &dicto->val), things will break badly.
			 * However, I can't think of a way that this could
			 * possibly happen in real use, so just assert.
			 */
			_cw_assert(a_key != &dicto->val);
		} else {
			cw_chi_t	*chi;

			/* Allocate and initialize. */
			dicto = stilt_dicto_get(a_stilt);
			chi = stilt_chi_get(a_stilt);
			stilo_no_new(&dicto->key);
			stilo_dup(&dicto->key, a_key);
			stilo_no_new(&dicto->val);
			stilo_dup(&dicto->val, a_val);

			/* Insert. */
			dch_insert(&dict->e.d.hash, (void *)&dicto->key,
			    (void *)dicto, chi);

			if (dict->stiloe.global)
				stilo_p_name_kref_insert(a_key, a_stilt, dict);
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
	_cw_assert(a_stilo->type == STILOT_DICT);

	dict = (cw_stiloe_dict_t *)a_stilo->o.stiloe;

	if (dict->stiloe.indirect)
		stilo_dict_undef(&dict->e.i.stilo, a_stilt, a_key);
	else {
		cw_stiloe_dicto_t	*dicto;
		cw_chi_t		*chi;

		stiloe_p_lock(&dict->stiloe);
		if (dch_remove(&dict->e.d.hash, (void *)a_key, NULL, (void
		    **)&dicto, &chi) == FALSE) {
			stilt_dicto_put(a_stilt, dicto);
			stilt_chi_put(a_stilt, chi);
		}
		if (dict->stiloe.global)
			stilo_p_name_kref_remove(a_key, a_stilt, dict);

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
	_cw_assert(a_stilo->type == STILOT_DICT);

	dict = (cw_stiloe_dict_t *)a_stilo->o.stiloe;

	if (dict->stiloe.indirect) {
		retval = stilo_dict_lookup(&dict->e.i.stilo, a_stilt, a_key,
		    r_stilo);
	} else if ((dict->stiloe.global == FALSE) ||
	    (stilo_p_name_kref_search(a_key, dict) == FALSE)) {
		cw_stiloe_dicto_t	*dicto;

		stiloe_p_lock(&dict->stiloe);
		if (dch_search(&dict->e.d.hash, (void *)a_key, (void **)&dicto)
		    == FALSE) {
			if (r_stilo != NULL) {
				stilo_no_new(r_stilo);
				stilo_dup(r_stilo, &dicto->val);
			}
			retval = FALSE;
		} else
			retval = TRUE;
		stiloe_p_unlock(&dict->stiloe);
	} else
		retval = TRUE;
	
	return retval;
}

cw_uint32_t
stilo_dict_count(cw_stilo_t *a_stilo)
{
	cw_uint32_t		retval;
	cw_stiloe_dict_t	*dict;

	_cw_check_ptr(a_stilo);
	_cw_assert(a_stilo->magic == _CW_STILO_MAGIC);
	_cw_assert(a_stilo->type == STILOT_DICT);

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
	_cw_assert(a_stilo->type == STILOT_DICT);

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
	cw_stiloe_file_t	*file;

	stilo_p_new(a_stilo, STILOT_FILE);

	file = (cw_stiloe_file_t *)stilt_malloc(a_stilt,
	    sizeof(cw_stiloe_file_t));

	file->stilt = a_stilt;
	file->fd = -1;

	file->buffer = NULL;
	file->buffer_mode = BUFFER_EMPTY;
	file->buffer_size = 0;
	file->buffer_offset = 0;

	file->read_f = NULL;
	file->write_f = NULL;
	file->arg = NULL;
	file->position = 0;

	a_stilo->o.stiloe = (cw_stiloe_t *)file;

	stiloe_p_new(a_stilo->o.stiloe, STILOT_FILE);
}

static void
stilo_p_file_delete(cw_stilo_t *a_stilo, cw_stilt_t *a_stilt)
{
	cw_stiloe_file_t	*file;
	cw_bool_t		ioerror = FALSE;

	_cw_check_ptr(a_stilo);
	_cw_assert(a_stilo->magic == _CW_STILO_MAGIC);
	_cw_assert(a_stilo->type == STILOT_FILE);

	file = (cw_stiloe_file_t *)a_stilo->o.stiloe;

	_cw_check_ptr(file);
	_cw_assert(file->stiloe.magic == _CW_STILOE_MAGIC);

	if (file->buffer != NULL) {
		if (file->buffer_mode == BUFFER_WRITE) {
			switch (file->fd) {
			case -2:
				if (file->write_f(file->arg, a_stilo, a_stilt,
				    file->buffer, file->buffer_offset))
					ioerror = TRUE;
				break;
			case -1:
				break;
			default:
				if (write(file->fd, file->buffer,
				    file->buffer_offset) == -1)
					ioerror = TRUE;
				break;
			}
		}
		stilt_free(file->stilt, file->buffer);
	}
	if (file->fd >= 0) {
		if (close(file->fd) == -1)
			ioerror = TRUE;
	}

	if (ioerror)
		stilt_error(a_stilt, STILTE_IOERROR);
}

static cw_stiloe_t *
stiloe_p_file_ref_iterate(cw_stiloe_t *a_stiloe, cw_bool_t a_reset)
{
	cw_stiloe_file_t	*file;

	file = (cw_stiloe_file_t *)a_stiloe;

	_cw_check_ptr(file);
	_cw_assert(file->stiloe.magic == _CW_STILOE_MAGIC);

	return NULL;
}

static cw_stilte_t
stilo_p_file_print(cw_stilo_t *a_stilo, cw_stilt_t *a_stilt, cw_stilo_t *a_file,
    cw_bool_t a_syntactic, cw_bool_t a_newline)
{
	cw_stilte_t		retval;
	cw_stiloe_file_t	*file;
	cw_uint8_t		newline = (a_newline) ? '\n' : '\0';

	_cw_check_ptr(a_stilo);
	_cw_assert(a_stilo->magic == _CW_STILO_MAGIC);
	_cw_assert(a_stilo->type == STILOT_FILE);

	file = (cw_stiloe_file_t *)a_stilo->o.stiloe;

	_cw_check_ptr(file);
	_cw_assert(file->stiloe.magic == _CW_STILOE_MAGIC);

	if (a_syntactic) {
		retval = stilo_file_output(a_file, a_stilt, "-file-[c]",
		    newline);
	} else {
		retval = stilo_file_output(a_file, a_stilt,
		    "--nostringval--[c]", newline);
	}

	return retval;
}

void
stilo_file_fd_wrap(cw_stilo_t *a_stilo, cw_uint32_t a_fd)
{
	cw_stiloe_file_t	*file;

	_cw_check_ptr(a_stilo);
	_cw_assert(a_stilo->magic == _CW_STILO_MAGIC);
	_cw_assert(a_stilo->type == STILOT_FILE);

	file = (cw_stiloe_file_t *)a_stilo->o.stiloe;

	_cw_check_ptr(file);
	_cw_assert(file->stiloe.magic == _CW_STILOE_MAGIC);
	_cw_assert(file->fd == -1);

	file->fd = a_fd;
}

void
stilo_file_interactive(cw_stilo_t *a_stilo, cw_stil_read_t *a_read,
    cw_stil_write_t *a_write, void *a_arg)
{
	cw_stiloe_file_t	*file;

	_cw_check_ptr(a_stilo);
	_cw_assert(a_stilo->magic == _CW_STILO_MAGIC);
	_cw_assert(a_stilo->type == STILOT_FILE);

	file = (cw_stiloe_file_t *)a_stilo->o.stiloe;

	_cw_check_ptr(file);
	_cw_assert(file->stiloe.magic == _CW_STILOE_MAGIC);
	_cw_assert(file->fd == -1);

	/*
	 * -2 is a special value that signifies this is an interactive editor
	 * "file".
	 */
	file->fd = -2;
	file->read_f = a_read;
	file->write_f = a_write;
	file->arg = a_arg;
	file->position = 0;
}

/* STILTE_LIMITCHECK, STILTE_INVALIDFILEACCESS */
cw_stilte_t
stilo_file_open(cw_stilo_t *a_stilo, const cw_uint8_t *a_filename, cw_uint32_t
    a_nlen, const cw_uint8_t *a_flags, cw_uint32_t a_flen)
{
	cw_stilte_t		retval;
	cw_stiloe_file_t	*file;
	cw_uint8_t		filename[PATH_MAX], flags[3];
	int			access;

	/*
	 * Copy the arguments to local buffers in order to assure '\0'
	 * termination.
	 */
	if (a_nlen >= sizeof(filename)) {
		retval = STILTE_LIMITCHECK;
		goto RETURN;
	}
	memcpy(filename, a_filename, a_nlen);
	filename[a_nlen] = '\0';

	if (a_flen >= sizeof(flags)) {
		retval = STILTE_LIMITCHECK;
		goto RETURN;
	}
	memcpy(flags, a_flags, a_flen);
	flags[a_flen] = '\0';

	_cw_check_ptr(a_stilo);
	_cw_assert(a_stilo->magic == _CW_STILO_MAGIC);
	_cw_assert(a_stilo->type == STILOT_FILE);

	file = (cw_stiloe_file_t *)a_stilo->o.stiloe;

	_cw_check_ptr(file);
	_cw_assert(file->stiloe.magic == _CW_STILOE_MAGIC);

	if (file->fd != -1) {
		retval = STILTE_INVALIDFILEACCESS;
		goto RETURN;
	}

	/* Convert a_flags to the integer representation. */
	switch (flags[0]) {
	case 'r':
		switch (flags[1]) {
		case '\0':
			access = O_RDONLY;
			break;
		case '+':
			access = O_RDWR;
			break;
		default:
			retval = STILTE_INVALIDFILEACCESS;
			goto RETURN;
		}
		break;
	case 'w':
		switch (flags[1]) {
		case '\0':
			access = O_WRONLY | O_CREAT | O_TRUNC;
			break;
		case '+':
			access = O_RDWR | O_CREAT | O_TRUNC;
			break;
		default:
			retval = STILTE_INVALIDFILEACCESS;
			goto RETURN;
		}
		break;
	case 'a':
		switch (flags[1]) {
		case '\0':
			access = O_WRONLY | O_APPEND | O_CREAT;
			break;
		case '+':
			access = O_RDWR | O_APPEND | O_CREAT;
			break;
		default:
			retval = STILTE_INVALIDFILEACCESS;
			goto RETURN;
		}
		break;
	default:
		retval = STILTE_INVALIDFILEACCESS;
		goto RETURN;
	}

	file->fd = open(filename, access, 0x1ff);
	if (file->fd == -1) {
		switch (errno) {
		case ENOSPC:
		case EMFILE:
		case ENFILE:
			retval = STILTE_IOERROR;
			goto RETURN;
		default:
			retval = STILTE_INVALIDFILEACCESS;
			goto RETURN;
		}
	}

	retval = STILTE_NONE;
	RETURN:
	return retval;
}

/* STILTE_IOERROR */
cw_stilte_t
stilo_file_close(cw_stilo_t *a_stilo, cw_stilt_t *a_stilt)
{
	cw_stilte_t		retval;
	cw_stiloe_file_t	*file;

	_cw_check_ptr(a_stilo);
	_cw_assert(a_stilo->magic == _CW_STILO_MAGIC);
	_cw_assert(a_stilo->type == STILOT_FILE);

	file = (cw_stiloe_file_t *)a_stilo->o.stiloe;

	_cw_check_ptr(file);
	_cw_assert(file->stiloe.magic == _CW_STILOE_MAGIC);

	if (file->fd == -1) {
		retval = STILTE_IOERROR;
		goto RETURN;
	}

	/* Flush and get rid of the buffer if necessary. */
	retval = stilo_file_buffer_flush(a_stilo, a_stilt);
	if (retval)
		goto RETURN;
	if (file->buffer != NULL) {
		stilt_free(a_stilt, file->buffer);
		file->buffer = NULL;
		file->buffer_size = 0;
		file->buffer_mode = BUFFER_EMPTY;
	}

	if ((file->fd >= 0) && (close(file->fd) == -1)) {
		file->fd = -1;
		retval = STILTE_IOERROR;
		goto RETURN;
	}
	file->fd = -1;

	retval = STILTE_NONE;
	RETURN:
	return retval;
}

/* -1: STILTE_IOERROR */
cw_sint32_t
stilo_file_read(cw_stilo_t *a_stilo, cw_stilt_t *a_stilt, cw_uint32_t a_len,
    cw_uint8_t *r_str)
{
	cw_sint32_t		retval;
	cw_stiloe_file_t	*file;

	_cw_check_ptr(a_stilo);
	_cw_assert(a_stilo->magic == _CW_STILO_MAGIC);
	_cw_assert(a_stilo->type == STILOT_FILE);

	file = (cw_stiloe_file_t *)a_stilo->o.stiloe;

	_cw_check_ptr(file);
	_cw_assert(file->stiloe.magic == _CW_STILOE_MAGIC);

	if (file->fd == -1) {
		retval = -1;
		goto RETURN;
	}		

	/*
	 * Use the internal buffer if it exists.  If there aren't enough data in
	 * the buffer to fill r_str, copy what we have to r_str, then do a
	 * readv() to fill r_str and the buffer.  If the buffer is empty, also
	 * do a readv() to replenish it.
	 */
	if (file->buffer != NULL) {
		if ((file->buffer_mode == BUFFER_READ) && (a_len <=
		    file->buffer_offset)) {
			/* We had enough buffered data. */
			memcpy(r_str, file->buffer, a_len);
			memmove(file->buffer, &file->buffer[a_len],
			    file->buffer_offset - a_len);
			retval = a_len;
			file->buffer_offset -= a_len;
			if (file->buffer_offset == 0)
				file->buffer_mode = BUFFER_EMPTY;
		} else {
			ssize_t		nread;

			/*
			 * Copy any buffered before reading more data.
			 */
			if (file->buffer_mode == BUFFER_READ) {
				memcpy(r_str, file->buffer,
				    file->buffer_offset);
				retval = file->buffer_offset;
				r_str += file->buffer_offset;
				a_len -= file->buffer_offset;
			} else
				retval = 0;
			/* Clear the buffer. */
			file->buffer_offset = 0;
			file->buffer_mode = BUFFER_EMPTY;

			if (file->fd >= 0) {
				struct iovec	iov[2];

				/*
				 * Finish filling r_str and replenish the
				 * internal buffer.
				 */
				iov[0].iov_base = r_str;
				iov[0].iov_len = a_len;
				iov[1].iov_base = file->buffer;
				iov[1].iov_len = file->buffer_size;

				nread = readv(file->fd, iov, 2);
			} else {
				/* Use the read wrapper function. */
				nread = file->read_f(file->arg, a_stilo,
				    a_stilt, a_len, r_str);
			}

			/* Handle various read return values. */
			if (nread == -1) {
				if (retval == 0) {
					retval = -1;
					goto RETURN;
				}
				/*
				 * There was an error, but we already managed to
				 * provide some data, so don't report an error
				 * this time around.
				 */
			} else if (nread <= a_len) {
				/*
				 * We didn't get enough data to start filling
				 * the internal buffer.
				 */
				retval += nread;
			} else {
				retval += a_len;
				file->buffer_offset = nread - a_len;
				file->buffer_mode = BUFFER_READ;
			}
		}
	} else {
		if (file->fd >= 0)
			retval = read(file->fd, r_str, a_len);
		else
			retval = file->read_f(file->arg, a_stilo, a_stilt,
			    a_len, r_str);
	}

	if (retval == 0) {
		file->fd = -1;
		if (file->buffer != NULL) {
			file->buffer_offset = 0;
			file->buffer_mode = BUFFER_EMPTY;
		}
	}
/*  	_cw_out_put("retval [i|s:s|+:+] :", retval); */
/*  	_cw_out_put_n(retval, "[s]", r_str); */
/*  	_cw_out_put(":\n"); */
	RETURN:
	return retval;
}

/* STILTE_IOERROR */
cw_stilte_t
stilo_file_write(cw_stilo_t *a_stilo, cw_stilt_t *a_stilt, const cw_uint8_t
    *a_str, cw_uint32_t a_len)
{
	cw_stilte_t		retval;
	cw_stiloe_file_t	*file;

	_cw_check_ptr(a_stilo);
	_cw_assert(a_stilo->magic == _CW_STILO_MAGIC);
	_cw_assert(a_stilo->type == STILOT_FILE);

	file = (cw_stiloe_file_t *)a_stilo->o.stiloe;

	_cw_check_ptr(file);
	_cw_assert(file->stiloe.magic == _CW_STILOE_MAGIC);
	if (file->fd == -1) {
		retval = STILTE_IOERROR;
		goto RETURN;
	}

	if (file->buffer != NULL) {
		/* Discard cached read data if necessary. */
		if (file->buffer_mode == BUFFER_READ) {
			file->buffer_mode = BUFFER_EMPTY;
			file->buffer_offset = 0;
		}

		/*
		 * Use the internal buffer.  If a_str won't entirely fit, do a
		 * writev() for a normal file descriptor, or a straight write
		 * for a wrapped file.
		 */
		if (a_len <= file->buffer_size - file->buffer_offset) {
			/* a_str will fit. */
			memcpy(&file->buffer[file->buffer_offset],
			    a_str, a_len);
			file->buffer_mode = BUFFER_WRITE;
			file->buffer_offset += a_len;
		} else if (file->fd >= 0) {
			struct iovec	iov[2];

			/* a_str won't fit.  Do a writev(). */

			iov[0].iov_base = file->buffer;
			iov[0].iov_len = file->buffer_offset;
			iov[1].iov_base = (char *)a_str;
			iov[1].iov_len = a_len;

			if (writev(file->fd, iov, 2) == -1) {
				retval = STILTE_IOERROR;
				goto RETURN;
			}

			file->buffer_mode = BUFFER_EMPTY;
			file->buffer_offset = 0;
		} else {
			/*
			 * a_str won't fit.  Flush the buffer and call the
			 * write wrapper function.
			 */
			retval = stilo_file_buffer_flush(a_stilo, a_stilt);
			if (retval)
				goto RETURN;

			if (file->write_f(file->arg, a_stilo, a_stilt, a_str,
			    a_len)) {
				retval = STILTE_IOERROR;
				goto RETURN;
			}
			file->position += a_len;
		}
	} else {
		if (file->fd >= 0) {
			if (write(file->fd, a_str, a_len) == -1) {
				retval = STILTE_IOERROR;
				goto RETURN;
			}
		} else {
			if (file->write_f(file->arg, a_stilo, a_stilt, a_str,
			    a_len)) {
				retval = STILTE_IOERROR;
				goto RETURN;
			}
			file->position += a_len;
		}
	}

	retval = STILTE_NONE;
	RETURN:
	return retval;
}

/* STILTE_IOERROR */
cw_stilte_t
stilo_file_output(cw_stilo_t *a_stilo, cw_stilt_t *a_stilt, const char
    *a_format, ...)
{
	cw_stilte_t		retval;
	cw_stiloe_file_t	*file;
	va_list			ap;

	_cw_check_ptr(a_stilo);
	_cw_assert(a_stilo->magic == _CW_STILO_MAGIC);
	_cw_assert(a_stilo->type == STILOT_FILE);

	file = (cw_stiloe_file_t *)a_stilo->o.stiloe;

	_cw_check_ptr(file);
	_cw_assert(file->stiloe.magic == _CW_STILOE_MAGIC);

	if (file->fd == -1) {
		retval = STILTE_IOERROR;
		goto RETURN;
	}

	if (file->buffer != NULL) {
		cw_uint32_t	maxlen;
		cw_sint32_t	nwrite;

		/*
		 * There is an internal buffer; attempt to render to it.  If the
		 * resulting string doesn't fit in the buffer, flush the buffer
		 * and write the string directly to the file.  This results in
		 * rendering part of the string twice, but the alternative is to
		 * always malloc().  Chances are good that rendering to the
		 * buffer will succeed, so this seems like a reasonable trade
		 * off.
		 */

		/* Discard cached read data if necessary. */
		if (file->buffer_mode == BUFFER_READ) {
			file->buffer_mode = BUFFER_EMPTY;
			file->buffer_offset = 0;
		}

		maxlen = file->buffer_size - file->buffer_offset;
		va_start(ap, a_format);
		if ((nwrite =
		    out_put_svn(cw_g_out,
		    &file->buffer[file->buffer_offset],
		    maxlen, a_format, ap)) == maxlen) {
			/*
			 * It probably didn't fit (there's definitely no space
			 * left over).
			 */
			va_end(ap);

			/* Flush the internal buffer. */
			retval = stilo_file_buffer_flush(a_stilo, a_stilt);
			if (retval)
				goto RETURN;

			if (file->fd >= 0) {
				/* Write directly to the file. */
				va_start(ap, a_format);
				if (out_put_fv(cw_g_out, file->fd, a_format, ap)
				    == -1) {
					retval = STILTE_IOERROR;
					goto RETURN;
				}
			} else {
				/*
				 * Try to render to the buffer again.  If it
				 * still doesn't fit, use out_put_sva to
				 * allocate a string, write the string, then
				 * deallocate it.
				 */
				va_end(ap);
				va_start(ap, a_format);

				if ((nwrite = out_put_svn(cw_g_out,
				    &file->buffer[file->buffer_offset], maxlen,
				    a_format, ap)) == maxlen) {
					char	*str;

					va_end(ap);
					va_start(ap, a_format);

					nwrite = out_put_sva(cw_g_out, &str,
					    a_format, ap);
					if (file->write_f(file->arg, a_stilo,
					    a_stilt, str, nwrite)) {
						_cw_free(str);
						retval = STILTE_IOERROR;
						goto RETURN;
					}
					_cw_free(str);
				} else {
					/* It fit on the second try. */
					file->buffer_mode = BUFFER_WRITE;
					file->buffer_offset += nwrite;
				}
			}
		} else {
			/* It fit. */
			file->buffer_mode = BUFFER_WRITE;
			file->buffer_offset += nwrite;
		}
		va_end(ap);
	} else {
		va_start(ap, a_format);
		if (out_put_fv(cw_g_out, file->fd, a_format, ap) == -1) {
			retval = STILTE_IOERROR;
			goto RETURN;
		}
		va_end(ap);
	}

	retval = STILTE_NONE;
	RETURN:
	return retval;
}

/* STILTE_IOERROR */
cw_stilte_t
stilo_file_output_n(cw_stilo_t *a_stilo, cw_stilt_t *a_stilt, cw_uint32_t
    a_size, const char *a_format, ...)
{
	cw_stilte_t		retval;
	cw_stiloe_file_t	*file;
	va_list			ap;

	_cw_check_ptr(a_stilo);
	_cw_assert(a_stilo->magic == _CW_STILO_MAGIC);
	_cw_assert(a_stilo->type == STILOT_FILE);

	file = (cw_stiloe_file_t *)a_stilo->o.stiloe;

	_cw_check_ptr(file);
	_cw_assert(file->stiloe.magic == _CW_STILOE_MAGIC);

	if (file->fd == -1) {
		retval = STILTE_IOERROR;
		goto RETURN;
	}

	va_start(ap, a_format);
	if (file->buffer != NULL) {
		cw_uint32_t	maxlen;

		/*
		 * There is an internal buffer; if a_size is less than the
		 * amount of free buffer space, render to it.  Otherwise, flush
		 * the buffer and write the string directly to the file.
		 */

		/* Discard cached read data if necessary. */
		if (file->buffer_mode == BUFFER_READ) {
			file->buffer_mode = BUFFER_EMPTY;
			file->buffer_offset = 0;
		}

		maxlen = file->buffer_size - file->buffer_offset;
		if (a_size < maxlen) {
			cw_sint32_t	nwrite;

			/* It will fit. */

			nwrite = out_put_svn(cw_g_out,
			    &file->buffer[file->buffer_offset],
			    maxlen, a_format, ap);
			file->buffer_mode = BUFFER_WRITE;
			file->buffer_offset += nwrite;
		} else {
			/* It won't fit. */

			/* Flush the internal buffer, if necessary. */
			retval = stilo_file_buffer_flush(a_stilo, a_stilt);
			if (retval)
				goto RETURN;

			if (file->fd >= 0) {
				/* Write directly to the file. */
				if (out_put_fvn(cw_g_out, file->fd, a_size,
				    a_format, ap) == -1) {
					retval = STILTE_IOERROR;
					goto RETURN;
				}
			} else {
				cw_sint32_t	nwrite;
				char		*str;

				nwrite = out_put_sva(cw_g_out, &str, a_format,
				    ap);
				if (file->write_f(file->arg, a_stilo, a_stilt,
				    str, (nwrite < a_size) ? nwrite : a_size)) {
					_cw_free(str);
					retval = STILTE_IOERROR;
					goto RETURN;
				}
				_cw_free(str);
			}
		}
	} else {
		if (file->fd >= 0) {
			if (out_put_fvn(cw_g_out, file->fd, a_size, a_format,
			    ap) == -1) {
				retval = STILTE_IOERROR;
				goto RETURN;
			}
		} else {
			cw_sint32_t	nwrite;
			char		*str;

			nwrite = out_put_sva(cw_g_out, &str, a_format, ap);
			if (file->write_f(file->arg, a_stilo, a_stilt, str,
			    (nwrite < a_size) ? nwrite : a_size)) {
				_cw_free(str);
				retval = STILTE_IOERROR;
				goto RETURN;
			}
			_cw_free(str);
		}
	}
	va_end(ap);

	retval = STILTE_NONE;
	RETURN:
	return retval;
}

/* STILTE_IOERROR */
cw_stilte_t
stilo_file_truncate(cw_stilo_t *a_stilo, cw_uint32_t a_length)
{
	cw_stilte_t		retval;
	cw_stiloe_file_t	*file;

	_cw_check_ptr(a_stilo);
	_cw_assert(a_stilo->magic == _CW_STILO_MAGIC);
	_cw_assert(a_stilo->type == STILOT_FILE);

	file = (cw_stiloe_file_t *)a_stilo->o.stiloe;

	_cw_check_ptr(file);
	_cw_assert(file->stiloe.magic == _CW_STILOE_MAGIC);

	if (file->fd == -2) {
		retval = STILTE_IOERROR;
		goto RETURN;
	}

	/* XXX Reset buffer. */

	if (ftruncate(file->fd, a_length)) {
		retval = STILTE_IOERROR;
		goto RETURN;
	}

	retval = STILTE_NONE;
	RETURN:
	return retval;
}

/* -1: STILTE_IOERROR */
cw_sint64_t
stilo_file_position_get(cw_stilo_t *a_stilo)
{
	cw_sint64_t		retval;
	cw_stiloe_file_t	*file;

	_cw_check_ptr(a_stilo);
	_cw_assert(a_stilo->magic == _CW_STILO_MAGIC);
	_cw_assert(a_stilo->type == STILOT_FILE);

	file = (cw_stiloe_file_t *)a_stilo->o.stiloe;

	_cw_check_ptr(file);
	_cw_assert(file->stiloe.magic == _CW_STILOE_MAGIC);

	if (file->fd == -1) {
		retval = -1;
		goto RETURN;
	}

	if (file->fd == -2)
		retval = file->position;
	else
		retval = lseek(file->fd, 0, SEEK_CUR);

	RETURN:
	return retval;
}

/* STILTE_IOERROR */
cw_stilte_t
stilo_file_position_set(cw_stilo_t *a_stilo, cw_stilt_t *a_stilt, cw_sint64_t
    a_position)
{
	cw_stilte_t		retval;
	cw_stiloe_file_t	*file;

	_cw_check_ptr(a_stilo);
	_cw_assert(a_stilo->magic == _CW_STILO_MAGIC);
	_cw_assert(a_stilo->type == STILOT_FILE);

	file = (cw_stiloe_file_t *)a_stilo->o.stiloe;

	_cw_check_ptr(file);
	_cw_assert(file->stiloe.magic == _CW_STILOE_MAGIC);

	if (file->fd < 0) {
		retval = STILTE_IOERROR;
		goto RETURN;
	}

	retval = stilo_file_buffer_flush(a_stilo, a_stilt);
	if (retval)
		goto RETURN;

	if (lseek(file->fd, a_position, SEEK_SET) == -1) {
		retval = STILTE_IOERROR;
		goto RETURN;
	}

	retval = STILTE_NONE;
	RETURN:
	return retval;
}

cw_uint32_t
stilo_file_buffer_size_get(cw_stilo_t *a_stilo)
{
	cw_stiloe_file_t	*file;

	_cw_check_ptr(a_stilo);
	_cw_assert(a_stilo->magic == _CW_STILO_MAGIC);
	_cw_assert(a_stilo->type == STILOT_FILE);

	file = (cw_stiloe_file_t *)a_stilo->o.stiloe;

	_cw_check_ptr(file);
	_cw_assert(file->stiloe.magic == _CW_STILOE_MAGIC);

	return file->buffer_size;
}

void
stilo_file_buffer_size_set(cw_stilo_t *a_stilo, cw_uint32_t a_size)
{
	cw_stiloe_file_t	*file;

	_cw_check_ptr(a_stilo);
	_cw_assert(a_stilo->magic == _CW_STILO_MAGIC);
	_cw_assert(a_stilo->type == STILOT_FILE);

	file = (cw_stiloe_file_t *)a_stilo->o.stiloe;

	_cw_check_ptr(file);
	_cw_assert(file->stiloe.magic == _CW_STILOE_MAGIC);

	if (a_size == 0) {
		if (file->buffer != NULL) {
			stilt_free(file->stilt, file->buffer);
			file->buffer = NULL;
			file->buffer_size = 0;
		}
	} else {
		if (file->buffer != NULL)
			stilt_free(file->stilt, file->buffer);
		file->buffer = (cw_uint8_t *)stilt_malloc(file->stilt,
		    a_size);
		file->buffer_size = a_size;
	}
	file->buffer_mode = BUFFER_EMPTY;
	file->buffer_offset = 0;
}

cw_sint64_t
stilo_file_buffer_count(cw_stilo_t *a_stilo)
{
	cw_sint64_t		retval;
	cw_stiloe_file_t	*file;

	_cw_check_ptr(a_stilo);
	_cw_assert(a_stilo->magic == _CW_STILO_MAGIC);
	_cw_assert(a_stilo->type == STILOT_FILE);

	file = (cw_stiloe_file_t *)a_stilo->o.stiloe;

	_cw_check_ptr(file);
	_cw_assert(file->stiloe.magic == _CW_STILOE_MAGIC);

	if ((file->fd != -1 && file->buffer != NULL && file->buffer_mode
	    != BUFFER_WRITE))
		retval = file->buffer_offset;
	else
		retval = 0;

	return retval;
}

/*
 * XXX There will need to be a private interface once object locking is in
 * place.
 */
/* STILTE_IOERROR */
cw_stilte_t
stilo_file_buffer_flush(cw_stilo_t *a_stilo, cw_stilt_t *a_stilt)
{
	cw_stilte_t		retval;
	cw_stiloe_file_t	*file;

	_cw_check_ptr(a_stilo);
	_cw_assert(a_stilo->magic == _CW_STILO_MAGIC);
	_cw_assert(a_stilo->type == STILOT_FILE);

	file = (cw_stiloe_file_t *)a_stilo->o.stiloe;

	_cw_check_ptr(file);
	_cw_assert(file->stiloe.magic == _CW_STILOE_MAGIC);

	if (file->fd == -1) {
		retval = STILTE_IOERROR;
		goto RETURN;
	}
	
	if (file->buffer != NULL) {
		/* Only write if the buffered data is for writing. */
		if (file->buffer_mode == BUFFER_WRITE) {
			if (file->fd >= 0) {
				/* Normal file descriptor. */
				if (write(file->fd, file->buffer,
				    file->buffer_offset) == -1) {
					retval = STILTE_IOERROR;
					goto RETURN;
				}
			} else {
				/* Use the write wrapper function. */
				if (file->write_f(file->arg, a_stilo, a_stilt,
				    file->buffer, file->buffer_offset)) {
					retval = STILTE_IOERROR;
					goto RETURN;
				}
			}
		}
		/*
		 * Reset the buffer to being empty, regardless of the type of
		 * buffered data.
		 */
		file->buffer_mode = BUFFER_EMPTY;
		file->buffer_offset = 0;
	}

	retval = STILTE_NONE;
	RETURN:
	return retval;
}

void
stilo_file_buffer_reset(cw_stilo_t *a_stilo)
{
	cw_stiloe_file_t	*file;

	_cw_check_ptr(a_stilo);
	_cw_assert(a_stilo->magic == _CW_STILO_MAGIC);
	_cw_assert(a_stilo->type == STILOT_FILE);

	file = (cw_stiloe_file_t *)a_stilo->o.stiloe;

	_cw_check_ptr(file);
	_cw_assert(file->stiloe.magic == _CW_STILOE_MAGIC);

	file->buffer_mode = BUFFER_EMPTY;
	file->buffer_offset = 0;
}

cw_bool_t
stilo_file_status(cw_stilo_t *a_stilo)
{
	cw_bool_t		retval;
	cw_stiloe_file_t	*file;

	_cw_check_ptr(a_stilo);
	_cw_assert(a_stilo->magic == _CW_STILO_MAGIC);
	_cw_assert(a_stilo->type == STILOT_FILE);

	file = (cw_stiloe_file_t *)a_stilo->o.stiloe;

	_cw_check_ptr(file);
	_cw_assert(file->stiloe.magic == _CW_STILOE_MAGIC);

	if (file->fd != -1)
		retval = FALSE;
	else
		retval = TRUE;

	return retval;
}

/* -1: STILTE_IOERROR */
cw_sint64_t
stilo_file_mtime(cw_stilo_t *a_stilo)
{
	cw_sint64_t		retval;
	struct stat		sb;
	cw_stiloe_file_t	*file;

	_cw_check_ptr(a_stilo);
	_cw_assert(a_stilo->magic == _CW_STILO_MAGIC);
	_cw_assert(a_stilo->type == STILOT_FILE);

	file = (cw_stiloe_file_t *)a_stilo->o.stiloe;

	_cw_check_ptr(file);
	_cw_assert(file->stiloe.magic == _CW_STILOE_MAGIC);

	if (file->fd < 0) {
		retval = -1;
		goto RETURN;
	}

	if (fstat(file->fd, &sb) == -1) {
		retval = -1;
		goto RETURN;
	}

	/* Keep 63 bits of accuracy. */
	retval = sb.st_mtimespec.tv_sec;
	retval <<= 31;
	retval |= (sb.st_mtimespec.tv_nsec >> 1);

	RETURN:
	return retval;
}

/*
 * hook.
 */
void
stilo_hook_new(cw_stilo_t *a_stilo, cw_stilt_t *a_stilt)
{
	stilo_p_new(a_stilo, STILOT_HOOK);
}

static void
stilo_p_hook_delete(cw_stilo_t *a_stilo, cw_stilt_t *a_stilt)
{
	_cw_error("XXX Not implemented");
}

static cw_stiloe_t *
stiloe_p_hook_ref_iterate(cw_stiloe_t *a_stilo, cw_bool_t a_reset)
{
	_cw_error("XXX Not implemented");
	return NULL;	/* XXX */
}

cw_stilte_t
stilo_hook_copy(cw_stilo_t *a_to, cw_stilo_t *a_from, cw_stilt_t *a_stilt)
{
	cw_stilte_t	retval;

	_cw_error("XXX Not implemented");

	retval = STILTE_UNREGISTERED;	/* XXX */

	return retval;
}

static cw_stilte_t
stilo_p_hook_print(cw_stilo_t *a_stilo, cw_stilt_t *a_stilt, cw_stilo_t *a_file,
    cw_bool_t a_syntactic, cw_bool_t a_newline)
{
	cw_stilte_t	retval;
	cw_uint8_t	newline = (a_newline) ? '\n' : '\0';
	
	if (a_syntactic) {
		retval = stilo_file_output(a_file, a_stilt, "-hook-[c]",
		    newline);
	} else {
		retval = stilo_file_output(a_file, a_stilt,
		    "--nostringval--[c]", newline);
	}

	return retval;
}

/*
 * integer.
 */
static cw_stilte_t
stilo_p_integer_print(cw_stilo_t *a_stilo, cw_stilt_t *a_stilt, cw_stilo_t
    *a_file, cw_bool_t a_syntactic, cw_bool_t a_newline)
{
	cw_stilte_t	retval;
	cw_uint8_t	newline = (a_newline) ? '\n' : '\0';
	
	retval = stilo_file_output(a_file, a_stilt, "[q|s:s][c]",
	    a_stilo->o.integer.i, newline);

	return retval;
}

/*
 * lock.
 */
void
stilo_lock_new(cw_stilo_t *a_stilo, cw_stilt_t *a_stilt)
{
	stilo_p_new(a_stilo, STILOT_LOCK);
}

static void
stilo_p_lock_delete(cw_stilo_t *a_stilo, cw_stilt_t *a_stilt)
{
	_cw_error("XXX Not implemented");
}

static cw_stiloe_t *
stiloe_p_lock_ref_iterate(cw_stiloe_t *a_stilo, cw_bool_t a_reset)
{
	_cw_error("XXX Not implemented");
	return NULL;	/* XXX */
}

static cw_stilte_t
stilo_p_lock_print(cw_stilo_t *a_stilo, cw_stilt_t *a_stilt, cw_stilo_t *a_file,
    cw_bool_t a_syntactic, cw_bool_t a_newline)
{
	cw_stilte_t	retval;
	cw_uint8_t	newline = (a_newline) ? '\n' : '\0';
	
	if (a_syntactic) {
		retval = stilo_file_output(a_file, a_stilt, "-lock-[c]",
		    newline);
	} else {
		retval = stilo_file_output(a_file, a_stilt,
		    "--nostringval--[c]", newline);
	}

	return retval;
}

/*
 * mark.
 */
void
stilo_mark_new(cw_stilo_t *a_stilo)
{
	stilo_p_new(a_stilo, STILOT_MARK);
}

static cw_stilte_t
stilo_p_mark_print(cw_stilo_t *a_stilo, cw_stilt_t *a_stilt, cw_stilo_t *a_file,
    cw_bool_t a_syntactic, cw_bool_t a_newline)
{
	cw_stilte_t	retval;
	cw_uint8_t	newline = (a_newline) ? '\n' : '\0';
	
	if (a_syntactic) {
		retval = stilo_file_output(a_file, a_stilt, "-mark-[c]",
		    newline);
	} else {
		retval = stilo_file_output(a_file, a_stilt,
		    "--nostringval--[c]", newline);
	}

	return retval;
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
 * Names are kept in a global hash table of stiloe_name's, and there is only one
 * stiloe_name per unique character string.  This allows the address of each
 * stiloe_name in the global hash table to be used as a unique key, so
 * regardless of the string length of a name, once it has been converted to a
 * stiloe_name pointer, name comparisons are a constant time operation.
 *
 * The following diagram shows the various name-related object relationships
 * that are possible.  Locally allocated name objects use a thread-specific
 * cache.  Globally allocated name objects refer directly to the global table.
 *
 * The reason for the thread-specific cache is that all operations on the global
 * table require locking.  By caching references to the global table on a
 * per-thread basis, we amortize the cost of locking the global table.
 *
 * The garbage collector periodically cleans out unused cached references from
 * the thread-specific hashes.  This means that any cached references not
 * referred to at GC time will be collected.  The frequency of local GC should
 * be low enough that repopulating the name cache after each GC doesn't add
 * significant overhead.
 *
 * /-----------------\                     /-----------------\
 * | stil.name_hash  |                     | stilt.name_hash |
 * |                 |                     |                 |
 * | /-------------\ |                     | /-------------\ |
 * | | stiloe_name | |                     | | stiloe_name | |
 * | |             | |                     | |             | |
 * | | /------\    | |                     | | /-------\   | |
 * | | | name |    | |            /------------| stilo |   | |
 * | | \------/    | |           /         | | \-------/   | |
 * | |             | |          /          | |             | |
 * | \-------------/ |          |          | \-------------/ |
 * |                 |          |          |                 |
 * | ............... |          |          | ............... |
 * | ............... |          |          | ............... |
 * | ............... |          |          | ............... |
 * |                 |          |          |                 |
 * | /-------------\ |          |          | /-------------\ |
 * | | stiloe_name | |          |          | | stiloe_name | |
 * | |             | |          /          | |             | |
 * | | /------\    | |         /           | | /-------\   | |
 * | | | name |    |<---------/   /------------| stilo |   | |
 * | | \------/    | |           /         | | \-------/   | |
 * | |             | |          /          | |             | |
 * | \-------------/ |          |          | \-------------/ |
 * |                 |          |          |                 |
 * | ............... |          |          \-----------------/
 * | ............... |          |
 * | ............... |          |
 * |                 |          |
 * | /-------------\ |          |          /-----------------\
 * | | stiloe_name | |          |          | stilt.name_hash |
 * | |             | |          |          |                 |
 * | | /------\    | |          |          | /-------------\ |
 * | | | name |    |<-----------+--\       | | stiloe_name | |
 * | | \------/    | |          |   \      | |             | |
 * | |             | |          |    \     | | /-------\   | |
 * | \-------------/ |          |     \--------| stilo |   | |
 * |                 |          |          | | \-------/   | |
 * | ............... |          |          | |             | |
 * | ............... |          |          | \-------------/ |
 * | ............... |          |          |                 |
 * |                 |          |          | ............... |
 * | /-------------\ |          /          | ............... |
 * | | stiloe_name | |         /           | ............... |
 * | |             |<---------/            |                 |
 * | | /------\    | |                     | /-------------\ |
 * | | | name |    | |                     | | stiloe_name | |
 * | | \------/    |<-----------\          | |             | |
 * | |             | |           \         | | /-------\   | |
 * | \-------------/ |            \------------| stilo |   | |
 * |       ^         |                     | | \-------/   | |
 * \-------|---------/                     | |             | |
 *         |                               | \-------------/ |
 *         |                               |        ^        |
 *         |                               \--------|--------/
 *         |                                        |
 * /-------------------\                            |
 * | stilo (global VM) |                            |
 * | (possibly keyed)  |                            |
 * \-------------------/                   /------------------\
 *                                         | stilo (local VM) |
 *                                         \------------------/
 */

/*
 * a_stilt's allocation mode (local/global) is used to determine how the object
 * is allocated.
 */
void
stilo_name_new(cw_stilo_t *a_stilo, cw_stilt_t *a_stilt, const cw_uint8_t
    *a_name, cw_uint32_t a_len, cw_bool_t a_is_static)
{
	cw_stiloe_name_t	*name, key;

	stilo_p_new(a_stilo, STILOT_NAME);

	/* Fake up a key so that we can search the hash tables. */
	key.e.n.name = a_name;
	key.e.n.len = a_len;

	if (stilt_currentglobal(a_stilt) == FALSE) {
		cw_dch_t		*name_hash;
		cw_stiloe_name_t	*gname;

		/*
		 * Look in the per-thread name cache for a cached reference to
		 * the name.  If there is no cached reference, check the global
		 * hash for the name.  Create the name in the global hash if
		 * necessary, then create a cached reference if necessary.
		 */
		name_hash = stilt_name_hash_get(a_stilt);
		if (dch_search(name_hash, (void *)&key, (void **)&name)) {
			/* Not found in the per-thread name cache. */
			gname = stilo_p_name_gref(a_stilt, a_name, a_len,
			    a_is_static);

			name = (cw_stiloe_name_t *)stilt_malloc(a_stilt,
			    sizeof(cw_stiloe_name_t));

			name->val = gname;

			/* stilo-internal initialization. */
			stilo_p_new(&name->e.i.stilo, STILOT_NAME);
			name->e.i.stilo.o.stiloe = (cw_stiloe_t *)gname;

			stiloe_p_new(&name->stiloe, STILOT_NAME);
			name->stiloe.indirect = TRUE;

			/*
			 * Insert a cached entry for this thread.
			 */
			dch_insert(name_hash, (void *)gname, (void **)name,
			    stilt_chi_get(a_stilt));
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

static cw_stilte_t
stilo_p_name_print(cw_stilo_t *a_stilo, cw_stilt_t *a_stilt, cw_stilo_t *a_file,
    cw_bool_t a_syntactic, cw_bool_t a_newline)
{
	cw_stilte_t		retval;
	cw_stiloe_name_t	*name;

	/* Chase down the name. */
	name = (cw_stiloe_name_t *)a_stilo->o.stiloe;
	name = name->val;
	_cw_assert(name == name->val);
	
	if ((a_syntactic) && (a_stilo->attrs == STILOA_LITERAL)) {
		retval = stilo_file_output(a_file, a_stilt, "/");
		if (retval)
			goto RETURN;
	}

	retval = stilo_file_output_n(a_file, a_stilt, name->e.n.len, "[s]",
	    name->e.n.name);
	if (retval)
		goto RETURN;

	if (a_newline) {
		retval = stilo_file_output(a_file, a_stilt, "\n");
		if (retval)
			goto RETURN;
	}

	retval = STILTE_NONE;
	RETURN:
	return retval;
}

const cw_uint8_t *
stilo_name_str_get(cw_stilo_t *a_stilo)
{
	const cw_uint8_t	*retval;
	cw_stiloe_name_t	*name;

	/* Chase down the name. */
	name = (cw_stiloe_name_t *)a_stilo->o.stiloe;
	name = name->val;
	_cw_assert(name == name->val);

	retval = name->e.n.name;

	return retval;
}

cw_uint32_t
stilo_name_len_get(cw_stilo_t *a_stilo)
{
	cw_uint32_t		retval;
	cw_stiloe_name_t	*name;

	/* Chase down the name. */
	name = (cw_stiloe_name_t *)a_stilo->o.stiloe;
	name = name->val;
	_cw_assert(name == name->val);

	retval = name->e.n.len;

	return retval;
}

static cw_stiloe_name_t *
stilo_p_name_gref(cw_stilt_t *a_stilt, const char *a_str, cw_uint32_t a_len,
    cw_bool_t a_is_static)
{
	cw_stiloe_name_t	*retval, key;
	cw_mtx_t		*name_lock;
	cw_dch_t		*name_hash;

/*  	_cw_out_put_e("Reference ([s] VM) \"", stilt_currentglobal(a_stilt) ? */
/*  	    "global" : "local"); */
/*  	_cw_out_put_n(a_len, "[s]", a_str); */
/*  	_cw_out_put("\" (len [i])\n", a_len); */

	/* Fake up a key so that we can search the hash tables. */
	key.e.n.name = a_str;
	key.e.n.len = a_len;

	name_lock = stil_name_lock_get(stilt_stil_get(a_stilt));
	name_hash = stil_name_hash_get(stilt_stil_get(a_stilt));

	/*
	 * Look in the global hash for the name.  If the name doesn't exist,
	 * create it.
	 */
	mtx_lock(name_lock);
	if (dch_search(name_hash, (void *)&key, (void **)&retval)) {
		cw_stilag_t	*stilag;

		/*
		 * Not found in the global hash.  Create, initialize, and insert
		 * a new entry.
		 */
		stilag = stil_stilag_get(stilt_stil_get(a_stilt));
		retval = (cw_stiloe_name_t *)stilag_malloc(stilag,
		    sizeof(cw_stiloe_name_t));
		memset(retval, 0, sizeof(cw_stiloe_name_t));

		retval->val = retval;

		retval->e.n.is_static_name = a_is_static;
		retval->e.n.len = a_len;
	
		if (a_is_static == FALSE) {
			/* This should be allocated from global space. */
			retval->e.n.name =
			    stilag_malloc(stil_stilag_get(stilt_stil_get(a_stilt)),
			    a_len);
			/*
			 * Cast away the const here; it's the only place that
			 * the string is allowed to be modified, and this cast
			 * is better than dropping the const altogether.
			 */
			memcpy((cw_uint8_t *)retval->e.n.name, a_str, a_len);
		} else
			retval->e.n.name = a_str;

		stiloe_p_new(&retval->stiloe, STILOT_NAME);

		dch_insert(name_hash, (void *)retval, (void
		    **)retval, stilt_chi_get(a_stilt));
	}
	mtx_unlock(name_lock);

	return retval;
}

/* Insert a keyed reference. */
static void
stilo_p_name_kref_insert(cw_stilo_t *a_stilo, cw_stilt_t *a_stilt, const
    cw_stiloe_dict_t *a_dict)
{
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

	dch_insert(name->e.n.keyed_refs, (void *)a_dict, NULL,
	    stilt_chi_get(a_stilt));

	stiloe_p_unlock(&name->stiloe);
}

/* Search for a keyed reference matching a_dict. */
static cw_bool_t
stilo_p_name_kref_search(const cw_stilo_t *a_stilo, const cw_stiloe_dict_t
    *a_dict)
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
stilo_p_name_kref_remove(const cw_stilo_t *a_stilo, cw_stilt_t *a_stilt, const
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
		stilt_chi_put(a_stilt, chi);

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
static cw_stilte_t
stilo_p_null_print(cw_stilo_t *a_stilo, cw_stilt_t *a_stilt, cw_stilo_t *a_file,
    cw_bool_t a_syntactic, cw_bool_t a_newline)
{
	cw_stilte_t	retval;
	cw_uint8_t	newline = (a_newline) ? '\n' : '\0';
	
	if (a_syntactic)
		retval = stilo_file_output(a_file, a_stilt, "null[c]", newline);
	else {
		retval = stilo_file_output(a_file, a_stilt,
		    "--nostringval--[c]", newline);
	}

	return retval;
}

/*
 * operator.
 */
void
stilo_operator_new(cw_stilo_t *a_stilo, cw_op_t *a_op)
{
	stilo_p_new(a_stilo, STILOT_OPERATOR);
	a_stilo->o.operator.f = a_op;
}

static cw_stilte_t
stilo_p_operator_print(cw_stilo_t *a_stilo, cw_stilt_t *a_stilt, cw_stilo_t
    *a_file, cw_bool_t a_syntactic, cw_bool_t a_newline)
{
	cw_stilte_t	retval;
	cw_uint8_t	newline = (a_newline) ? '\n' : '\0';
	
	if (a_syntactic) {
		retval = stilo_file_output(a_file, a_stilt, "-operator-[c]",
		    newline);
	} else {
		retval = stilo_file_output(a_file, a_stilt,
		    "--nostringval--[c]", newline);
	}

	return retval;
}

/*
 * string.
 */
void
stilo_string_new(cw_stilo_t *a_stilo, cw_stilt_t *a_stilt, cw_uint32_t a_len)
{
	cw_stiloe_string_t	*string;

	stilo_p_new(a_stilo, STILOT_STRING);

	string = (cw_stiloe_string_t *)stilt_malloc(a_stilt,
	    sizeof(cw_stiloe_string_t));

	string->iterations = 0;
	string->e.s.len = a_len;

	if (string->e.s.len > 0) {
		string->e.s.str = (cw_uint8_t
		    *)stilt_malloc(a_stilt, string->e.s.len);
		memset(string->e.s.str, 0, string->e.s.len);
	} else
		string->e.s.str = NULL;

	a_stilo->o.stiloe = (cw_stiloe_t *)string;

	stiloe_p_new(a_stilo->o.stiloe, STILOT_STRING);
}

cw_stilte_t
stilo_string_substring_new(cw_stilo_t *a_stilo, cw_stilo_t *a_string, cw_stilt_t
    *a_stilt, cw_uint32_t a_offset, cw_uint32_t a_len)
{
	cw_stilte_t		retval;
	cw_stiloe_string_t	*string, *orig;

	orig = (cw_stiloe_string_t *)a_string->o.stiloe;
	_cw_check_ptr(orig);
	_cw_assert(orig->stiloe.magic == _CW_STILOE_MAGIC);
	if (a_offset + a_len > orig->e.s.len) {
		retval = STILTE_RANGECHECK;
		goto RETURN;
	}

	stilo_p_new(a_stilo, STILOT_STRING);
	string = (cw_stiloe_string_t *)stilt_malloc(a_stilt,
	    sizeof(cw_stiloe_string_t));
	a_stilo->o.stiloe = (cw_stiloe_t *)string;

	string->iterations = 0;
	memcpy(&string->e.i.stilo, a_string, sizeof(cw_stilo_t));

	string->e.i.beg_offset = a_offset;
	string->e.i.len = a_len;

	memcpy(&string->stiloe, &orig->stiloe, sizeof(cw_stiloe_t));
	string->stiloe.watchpoint = FALSE;
	string->stiloe.indirect = TRUE;

	retval = STILTE_NONE;
	RETURN:
	return retval;
}

static void
stilo_p_string_delete(cw_stilo_t *a_stilo, cw_stilt_t *a_stilt)
{
	cw_stiloe_string_t	*string;

	string = (cw_stiloe_string_t *)a_stilo->o.stiloe;
	_cw_check_ptr(string);
	_cw_assert(string->stiloe.magic == _CW_STILOE_MAGIC);

/*  	stiloe_p_delete(&string->stiloe, a_stilt); */
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

cw_stilte_t
stilo_string_copy(cw_stilo_t *a_to, cw_stilo_t *a_from, cw_stilt_t *a_stilt)
{
	cw_stilte_t		retval;
	cw_stiloe_string_t	*string;
	cw_uint8_t		*str_from, *str_to;
	cw_uint32_t		len;

	string = (cw_stiloe_string_t *)a_from->o.stiloe;

	/*
	 * Set str_from and len according to whether this is an indirect object.
	 */
	if (string->stiloe.indirect) {
		cw_stiloe_string_t	*string_indir;

		string_indir = (cw_stiloe_string_t *)string->e.i.stilo.o.stiloe;

		str_from =
		    &string_indir->e.s.str[string->e.i.beg_offset];
		len = string->e.i.len;
		_cw_assert(len + string->e.i.beg_offset <=
		    string_indir->e.s.len);
	} else {
		str_from = string->e.s.str;
		len = string->e.s.len;
	}

	str_to = ((cw_stiloe_string_t *)a_to->o.stiloe)->e.s.str;

	/* Make sure destination is large enough. */
	if (((cw_stiloe_string_t *)a_to->o.stiloe)->e.s.len < len) {
		retval = STILTE_RANGECHECK;
		goto RETURN;
	}

	/* Copy the appropriate range. */
	memcpy(str_to, str_from, len);

	retval = STILTE_NONE;
	RETURN:
	return retval;
}

static cw_stilte_t
stilo_p_string_print(cw_stilo_t *a_stilo, cw_stilt_t *a_stilt, cw_stilo_t
    *a_file, cw_bool_t a_syntactic, cw_bool_t a_newline)
{
	cw_stilte_t	retval;
	cw_uint8_t	newline = (a_newline) ? '\n' : '\0';
	cw_uint8_t	*str;
	cw_sint32_t	len;
	cw_uint32_t	i;

	str = stilo_string_get(a_stilo);
	len = stilo_string_len_get(a_stilo);

	if (a_syntactic) {
		stilo_file_output(a_file, a_stilt, "(");
		for (i = 0; i < len; i++) {
			switch (str[i]) {
			case '\n':
				retval = stilo_file_output(a_file, a_stilt,
				    "\\n");
				break;
			case '\r':
				retval = stilo_file_output(a_file, a_stilt,
				    "\\r");
				break;
			case '\t':
				retval = stilo_file_output(a_file, a_stilt,
				    "\\t");
				break;
			case '\b':
				retval = stilo_file_output(a_file, a_stilt,
				    "\\b");
				break;
			case '\f':
				retval = stilo_file_output(a_file, a_stilt,
				    "\\f");
				break;
			case '\\':
				retval = stilo_file_output(a_file, a_stilt,
				    "\\\\");
				break;
			case '(':
				retval = stilo_file_output(a_file, a_stilt,
				    "\\(");
				break;
			case ')':
				retval = stilo_file_output(a_file, a_stilt,
				    "\\)");
				break;
			default:
				if (isprint(str[i]))
					retval = stilo_file_output(a_file,
					    a_stilt, "[c]", str[i]);
				else {
					retval = stilo_file_output(a_file,
					    a_stilt, "\\x[i|b:16|w:2|p:0]",
					    str[i]);
				}
				break;
			}
			if (retval)
				goto RETURN;
		}
		retval = stilo_file_output(a_file, a_stilt, ")[c]", newline);
		if (retval)
			goto RETURN;
	} else {
		if (len > 0) {
			retval = stilo_file_output_n(a_file, a_stilt, len,
			    "[s]", str);
			if (retval)
				goto RETURN;
		}
		retval = stilo_file_output(a_file, a_stilt, "[c]", newline);
		if (retval)
			goto RETURN;
	}

	retval = STILTE_NONE;
	RETURN:
	return retval;
}

cw_uint32_t
stilo_string_len_get(cw_stilo_t *a_stilo)
{
	cw_uint32_t		retval;
	cw_stiloe_string_t	*string;

	_cw_check_ptr(a_stilo);
	_cw_assert(a_stilo->magic == _CW_STILO_MAGIC);
	_cw_assert(a_stilo->type == STILOT_STRING);

	string = (cw_stiloe_string_t *)a_stilo->o.stiloe;
	_cw_assert(string->stiloe.magic == _CW_STILOE_MAGIC);

	if (string->stiloe.indirect == FALSE)
		retval = string->e.s.len;
	else
		retval = string->e.i.len;

	return retval;
}

cw_uint8_t *
stilo_string_el_get(cw_stilo_t *a_stilo, cw_sint64_t a_offset)
{
	cw_uint8_t		*retval;
	cw_stiloe_string_t	*string;

	_cw_check_ptr(a_stilo);
	_cw_assert(a_stilo->magic == _CW_STILO_MAGIC);
	_cw_assert(a_stilo->type == STILOT_STRING);

	string = (cw_stiloe_string_t *)a_stilo->o.stiloe;
	_cw_assert(string->stiloe.magic == _CW_STILOE_MAGIC);

	if (string->stiloe.indirect == FALSE) {
		if (a_offset >= string->e.s.len || a_offset < 0) {
			retval = NULL;
			goto RETURN;
		}
		retval = &string->e.s.str[a_offset];
	} else {
		retval = stilo_string_el_get(&string->e.i.stilo, a_offset +
		    string->e.i.beg_offset);
	}

	RETURN:
	return retval;
}

cw_stilte_t
stilo_string_el_set(cw_stilo_t *a_stilo, cw_uint8_t a_el, cw_sint64_t a_offset)
{
	cw_stilte_t		retval;
	cw_stiloe_string_t	*string;

	_cw_check_ptr(a_stilo);
	_cw_assert(a_stilo->magic == _CW_STILO_MAGIC);
	_cw_assert(a_stilo->type == STILOT_STRING);

	string = (cw_stiloe_string_t *)a_stilo->o.stiloe;
	_cw_assert(string->stiloe.magic == _CW_STILOE_MAGIC);

	if (string->stiloe.indirect == FALSE) {
		if (a_offset >= string->e.s.len || a_offset < 0) {
			retval = STILTE_RANGECHECK;
			goto RETURN;
		}
		string->e.s.str[a_offset] = a_el;
		retval = STILTE_NONE;
	} else {
		retval = stilo_string_el_set(&string->e.i.stilo, a_el, a_offset
		    + string->e.i.beg_offset);
	}

	RETURN:
	return retval;
}

cw_uint8_t *
stilo_string_get(cw_stilo_t *a_stilo)
{
	cw_uint8_t		*retval;
	cw_stiloe_string_t	*string;

	_cw_check_ptr(a_stilo);
	_cw_assert(a_stilo->magic == _CW_STILO_MAGIC);
	_cw_assert(a_stilo->type == STILOT_STRING);

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

cw_stilte_t
stilo_string_set(cw_stilo_t *a_stilo, cw_uint32_t a_offset, const cw_uint8_t
    *a_str, cw_uint32_t a_len)
{
	cw_stilte_t		retval;
	cw_stiloe_string_t	*string;
	cw_uint8_t		*str;

	_cw_check_ptr(a_stilo);
	_cw_assert(a_stilo->magic == _CW_STILO_MAGIC);
	_cw_assert(a_stilo->type == STILOT_STRING);

	string = (cw_stiloe_string_t *)a_stilo->o.stiloe;
	_cw_assert(string->stiloe.magic == _CW_STILOE_MAGIC);

	/* Get the string pointer. */
	if (string->stiloe.indirect == FALSE) {
		if (a_offset + a_len > string->e.s.len) {
			retval = STILTE_RANGECHECK;
			goto RETURN;
		}
		str = string->e.s.str;

		memcpy(&str[a_offset], a_str, a_len);
		retval = STILTE_NONE;
	} else {
		retval = stilo_string_set(&string->e.i.stilo, a_offset, a_str,
		    a_len);
	}

	RETURN:
	return retval;
}
