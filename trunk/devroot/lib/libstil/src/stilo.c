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
#include "../include/libstil/stilt_l.h"

#include <stdarg.h>
#include <ctype.h>
#include <errno.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <poll.h>

#ifdef _LIBSTIL_DBG
#define _CW_STILOE_MAGIC	0x0fa6e798
#endif

/*
 * Don't actually free stiloe's if we're running GC diagnostics.  Instead, just
 * reset the stiloe magic.  This way, we should still core dump when we hit
 * collected stiloe's, but can actually see the old contents of the stiloe we
 * tried to use.
 */
#if (0 && defined(_LIBSTIL_CONFESS) && defined(_LIBSTIL_DBG))
#define	_CW_FREE(a_stilo)
#define	_CW_STILOE_FREE(a_stiloe) (a_stiloe)->stiloe.magic = 0
#else
#define	_CW_FREE(a_stilo) _cw_free(a_stilo)
#define	_CW_STILOE_FREE(a_stiloe) _cw_free(a_stiloe)
#endif

/*
 * Size of stack-allocated buffer to use for stilo_file_readline().  If this
 * overflows, heap allocation is used.
 */
#ifdef _LIBSTIL_DBG
#define	_CW_STILO_FILE_READLINE_BUFSIZE	 25
#else
#define	_CW_STILO_FILE_READLINE_BUFSIZE	100
#endif

typedef struct cw_stiloe_array_s cw_stiloe_array_t;
typedef struct cw_stiloe_condition_s cw_stiloe_condition_t;
typedef struct cw_stiloe_file_s cw_stiloe_file_t;
typedef struct cw_stiloe_hook_s cw_stiloe_hook_t;
typedef struct cw_stiloe_mutex_s cw_stiloe_mutex_t;
typedef struct cw_stiloe_name_s cw_stiloe_name_t;
typedef struct cw_stiloe_string_s cw_stiloe_string_t;

struct cw_stiloe_array_s {
	cw_stiloe_t	stiloe;
	/*
	 * Access is locked if this object has the locking bit set.  Indirect
	 * arrays aren't locked, but their parents are.
	 */
	cw_mtx_t	lock;
	/*
	 * Used for remembering the current state of reference iteration.
	 */
	cw_uint32_t	ref_iter;
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
	/*
	 * Access is locked if this object has the locking bit set.
	 */
	cw_mtx_t	lock;
	/*
	 * Used for remembering the current state of reference iteration.
	 */
	cw_uint32_t	ref_iter;
	/*
	 * If non-NULL, the previous reference iteration returned the key of
	 * this dicto, so the value of this dicto is the next reference to
	 * check.
	 */
	cw_stiloe_dicto_t *dicto;
	/*
	 * Name/value pairs.  The keys are (cw_stilo_t *), and the values are
	 * (cw_stiloe_dicto_t *).  The stilo that the key points to resides in
	 * the stiloe_dicto (value) structure.
	 */
	cw_dch_t	hash;
};

struct cw_stiloe_file_s {
	cw_stiloe_t	stiloe;
	/*
	 * Access is locked if this object has the locking bit set.
	 */
	cw_mtx_t	lock;
	/*
	 * File descriptor number.  -1: Invalid, -2: Wrapped.
	 */
	cw_sint32_t	fd;
	/*
	 * Buffering.
	 */
	cw_uint8_t	*buffer;
	cw_uint32_t	buffer_size;
	enum {
		BUFFER_EMPTY,
		BUFFER_READ,
		BUFFER_WRITE
	}		buffer_mode;
	cw_uint32_t	buffer_offset;
	/*
	 * Used for wrapped files.
	 */
	cw_stilo_file_read_t	*read_f;
	cw_stilo_file_write_t	*write_f;
	void		*arg;
	cw_sint64_t	position;
};

struct cw_stiloe_hook_s {
	cw_stiloe_t	stiloe;
	void		*data;
	cw_stilo_hook_exec_t *exec_f;
	cw_stilo_hook_ref_iter_t *ref_iter_f;
	cw_stilo_hook_delete_t *delete_f;
};

struct cw_stiloe_mutex_s {
	cw_stiloe_t	stiloe;
	cw_mtx_t	lock;
};

struct cw_stiloe_name_s {
	cw_stiloe_t	stiloe;
	/*
	 * name is not required to be NULL-terminated, so we keep track of the
	 * length.
	 */
	const cw_uint8_t *str;
	cw_uint32_t	len;
};

struct cw_stiloe_string_s {
	cw_stiloe_t	stiloe;
	/*
	 * Access is locked if this object has the locking bit set.  Indirect
	 * strings aren't locked, but their parents are.
	 */
	cw_mtx_t	lock;
	/*
	 * Used for remembering the current state of reference iteration.
	 */
	cw_uint32_t	ref_iter;
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
static void	stiloe_p_new(cw_stiloe_t *a_stiloe, cw_stilot_t a_type,
    cw_bool_t a_locking);

/* no. */
static cw_stilte_t stilo_p_no_print(cw_stilo_t *a_stilo, cw_stilo_t *a_file,
    cw_uint32_t a_depth);

/* array. */
static void	stiloe_p_array_delete(cw_stiloe_t *a_stiloe, cw_stil_t *a_stil);
static cw_stiloe_t *stiloe_p_array_ref_iter(cw_stiloe_t *a_stiloe, cw_bool_t
    a_reset);
static cw_stilte_t stilo_p_array_print(cw_stilo_t *a_stilo, cw_stilo_t *a_file,
    cw_uint32_t a_depth);

#define		stiloe_p_array_lock(a_stiloe) do {			\
	if ((a_stiloe)->stiloe.locking && !(a_stiloe)->stiloe.indirect)	\
		mtx_lock(&(a_stiloe)->lock);				\
} while (0)
#define		stiloe_p_array_unlock(a_stiloe) do {			\
	if ((a_stiloe)->stiloe.locking && !(a_stiloe)->stiloe.indirect)	\
		mtx_unlock(&(a_stiloe)->lock);				\
} while (0)

/* boolean. */
static cw_stilte_t stilo_p_boolean_print(cw_stilo_t *a_stilo, cw_stilo_t
    *a_file, cw_uint32_t a_depth);

/* condition. */
static void	stiloe_p_condition_delete(cw_stiloe_t *a_stiloe, cw_stil_t
    *a_stil);
static cw_stiloe_t *stiloe_p_condition_ref_iter(cw_stiloe_t *a_stiloe,
    cw_bool_t a_reset);
static cw_stilte_t stilo_p_condition_print(cw_stilo_t *a_stilo, cw_stilo_t
    *a_file, cw_uint32_t a_depth);

/* dict. */
static void	stiloe_p_dict_delete(cw_stiloe_t *a_stiloe, cw_stil_t *a_stil);
static cw_stiloe_t *stiloe_p_dict_ref_iter(cw_stiloe_t *a_stiloe, cw_bool_t
    a_reset);
static cw_stilte_t stilo_p_dict_print(cw_stilo_t *a_stilo, cw_stilo_t *a_file,
    cw_uint32_t a_depth);

#define		stiloe_p_dict_lock(a_stiloe) do {			\
	if ((a_stiloe)->stiloe.locking)					\
		mtx_lock(&(a_stiloe)->lock);				\
} while (0)
#define		stiloe_p_dict_unlock(a_stiloe) do {			\
	if ((a_stiloe)->stiloe.locking)					\
		mtx_unlock(&(a_stiloe)->lock);				\
} while (0)

/* file. */
static void	stiloe_p_file_delete(cw_stiloe_t *a_stiloe, cw_stil_t *a_stil);
static cw_stiloe_t *stiloe_p_file_ref_iter(cw_stiloe_t *a_stiloe, cw_bool_t
    a_reset);
static cw_stilte_t stilo_p_file_print(cw_stilo_t *a_stilo, cw_stilo_t *a_file,
    cw_uint32_t a_depth);
static cw_stilte_t stilo_p_file_buffer_flush(cw_stilo_t *a_stilo);

#define		stiloe_p_file_lock(a_stiloe) do {			\
	if ((a_stiloe)->stiloe.locking)					\
		mtx_lock(&(a_stiloe)->lock);				\
} while (0)
#define		stiloe_p_file_unlock(a_stiloe) do {			\
	if ((a_stiloe)->stiloe.locking)					\
		mtx_unlock(&(a_stiloe)->lock);				\
} while (0)

/* hook. */
static void	stiloe_p_hook_delete(cw_stiloe_t *a_stiloe, cw_stil_t *a_stil);
static cw_stiloe_t *stiloe_p_hook_ref_iter(cw_stiloe_t *a_stiloe, cw_bool_t
    a_reset);
static cw_stilte_t stilo_p_hook_print(cw_stilo_t *a_stilo, cw_stilo_t *a_file,
    cw_uint32_t a_depth);

/* integer. */
static cw_stilte_t stilo_p_integer_print(cw_stilo_t *a_stilo, cw_stilo_t
    *a_file, cw_uint32_t a_depth);

/* mark. */
static cw_stilte_t stilo_p_mark_print(cw_stilo_t *a_stilo, cw_stilo_t *a_file,
    cw_uint32_t a_depth);

/* mutex. */
static void	stiloe_p_mutex_delete(cw_stiloe_t *a_stiloe, cw_stil_t *a_stil);
static cw_stiloe_t *stiloe_p_mutex_ref_iter(cw_stiloe_t *a_stiloe, cw_bool_t
    a_reset);
static cw_stilte_t stilo_p_mutex_print(cw_stilo_t *a_stilo, cw_stilo_t *a_file,
    cw_uint32_t a_depth);

/* name. */
static void	stiloe_p_name_delete(cw_stiloe_t *a_stiloe, cw_stil_t *a_stil);
static cw_stiloe_t *stiloe_p_name_ref_iter(cw_stiloe_t *a_stiloe, cw_bool_t
    a_reset);
static cw_stilte_t stilo_p_name_print(cw_stilo_t *a_stilo, cw_stilo_t *a_file,
    cw_uint32_t a_depth);

/* null. */
static cw_stilte_t stilo_p_null_print(cw_stilo_t *a_stilo, cw_stilo_t *a_file,
    cw_uint32_t a_depth);

/* operator. */
static cw_stilte_t stilo_p_operator_print(cw_stilo_t *a_stilo, cw_stilo_t
    *a_file, cw_uint32_t a_depth);

/* string. */
static void	stiloe_p_string_delete(cw_stiloe_t *a_stiloe, cw_stil_t
    *a_stil);
static cw_stiloe_t *stiloe_p_string_ref_iter(cw_stiloe_t *a_stiloe, cw_bool_t
    a_reset);
static cw_stilte_t stilo_p_string_print(cw_stilo_t *a_stilo, cw_stilo_t *a_file,
    cw_uint32_t a_depth);

#define		stiloe_p_string_lock(a_stiloe) do {			\
	if ((a_stiloe)->stiloe.locking && !(a_stiloe)->stiloe.indirect)	\
		mtx_lock(&(a_stiloe)->lock);				\
} while (0)
#define		stiloe_p_string_unlock(a_stiloe) do {			\
	if ((a_stiloe)->stiloe.locking && !(a_stiloe)->stiloe.indirect)	\
		mtx_unlock(&(a_stiloe)->lock);				\
} while (0)

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
	 stilo_p_no_print},	/* Debugging only. */

	/* STILOT_ARRAY */
	{stiloe_p_array_delete,
	 stiloe_p_array_ref_iter,
	 stilo_p_array_print},

	/* STILOT_BOOLEAN */
	{NULL,
	 NULL,
	 stilo_p_boolean_print},

	/* STILOT_CONDITION */
	{stiloe_p_condition_delete,
	 stiloe_p_condition_ref_iter,
	 stilo_p_condition_print},
	
	/* STILOT_DICT */
	{stiloe_p_dict_delete,
	 stiloe_p_dict_ref_iter,
	 stilo_p_dict_print},

	/* STILOT_FILE */
	{stiloe_p_file_delete,
	 stiloe_p_file_ref_iter,
	 stilo_p_file_print},

	/* STILOT_HOOK */
	{stiloe_p_hook_delete,
	 stiloe_p_hook_ref_iter,
	 stilo_p_hook_print},

	/* STILOT_INTEGER */
	{NULL,
	 NULL,
	 stilo_p_integer_print},

	/* STILOT_MARK */
	{NULL,
	 NULL,
	 stilo_p_mark_print},

	/* STILOT_MUTEX */
	{stiloe_p_mutex_delete,
	 stiloe_p_mutex_ref_iter,
	 stilo_p_mutex_print},

	/* STILOT_NAME */
	{stiloe_p_name_delete,
	 stiloe_p_name_ref_iter,
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
	{stiloe_p_string_delete,
	 stiloe_p_string_ref_iter,
	 stilo_p_string_print}
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
		retval = ch_direct_hash((void *)name);
	} else {
		_cw_error("XXX Not implemented");
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
		
		n1 = (cw_stiloe_name_t *)k1->o.stiloe;
		n2 = (cw_stiloe_name_t *)k2->o.stiloe;

		retval = (n1 == n2) ? TRUE : FALSE;
	} else {
		_cw_error("XXX Not implemented");
	}

	return retval;
}

/*
 * stiloe.
 */
/* Can be called at any time during stiloe_* initialization. */
static void
stiloe_p_new(cw_stiloe_t *a_stiloe, cw_stilot_t a_type, cw_bool_t a_locking)
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

/*
 * Only to be called from the GC.
 */
cw_stilte_t
stiloe_l_print(cw_stiloe_t *a_stiloe, cw_stilo_t *a_file, cw_uint32_t a_depth,
    cw_bool_t a_newline)
{
	cw_stilte_t	retval;
	cw_stilo_t	stilo;

	stilo_p_new(&stilo, a_stiloe->type);
	stilo.o.stiloe = a_stiloe;

	retval = stilo_print(&stilo, a_file, a_depth, a_newline);

	return retval;
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

/*
 * no.
 */
static cw_stilte_t
stilo_p_no_print(cw_stilo_t *a_stilo, cw_stilo_t *a_file, cw_uint32_t a_depth)
{
	cw_stilte_t	retval;

	retval = stilo_file_output(a_file, "-notype-");

	return retval;
}

/*
 * array.
 */
void
stilo_array_new(cw_stilo_t *a_stilo, cw_stil_t *a_stil, cw_bool_t a_locking,
    cw_uint32_t a_len)
{
	cw_stiloe_array_t	*array;
	cw_uint32_t		i;

	array = (cw_stiloe_array_t *)_cw_malloc(sizeof(cw_stiloe_array_t));

	stiloe_p_new(&array->stiloe, STILOT_ARRAY, a_locking);
	if (a_locking)
		mtx_new(&array->lock);
	array->e.a.len = a_len;
	if (array->e.a.len > 0) {
		array->e.a.arr = (cw_stilo_t *)_cw_malloc(sizeof(cw_stilo_t) *
		    array->e.a.len);
		for (i = 0; i < array->e.a.len; i++)
			stilo_null_new(&array->e.a.arr[i]);
	}

	memset(a_stilo, 0, sizeof(cw_stilo_t));
	a_stilo->o.stiloe = (cw_stiloe_t *)array;
#ifdef _LIBSTIL_DBG
	a_stilo->magic = _CW_STILO_MAGIC;
#endif
	a_stilo->type = STILOT_ARRAY;

	stila_l_gc_register(stil_stila_get(a_stil), (cw_stiloe_t *)array);
}

void
stilo_array_subarray_new(cw_stilo_t *a_stilo, cw_stilo_t *a_array, cw_stil_t
    *a_stil, cw_uint32_t a_offset, cw_uint32_t a_len)
{
	cw_stiloe_array_t	*array, *orig;

	_cw_check_ptr(a_stilo);

	orig = (cw_stiloe_array_t *)a_array->o.stiloe;
	_cw_check_ptr(orig);
	_cw_assert(orig->stiloe.magic == _CW_STILOE_MAGIC);

	if (orig->stiloe.indirect) {
		stilo_array_subarray_new(a_stilo, &orig->e.i.stilo, a_stil,
		    a_offset + orig->e.i.beg_offset, a_len);
	} else {
		_cw_assert(a_offset + a_len <= orig->e.a.len);

		array = (cw_stiloe_array_t
		    *)_cw_malloc(sizeof(cw_stiloe_array_t));

		stiloe_p_new(&array->stiloe, STILOT_ARRAY, FALSE);
		array->stiloe.indirect = TRUE;
		memcpy(&array->e.i.stilo, a_array, sizeof(cw_stilo_t));
		array->e.i.beg_offset = a_offset;
		array->e.i.len = a_len;

		memset(a_stilo, 0, sizeof(cw_stilo_t));
		a_stilo->o.stiloe = (cw_stiloe_t *)array;
#ifdef _LIBSTIL_DBG
		a_stilo->magic = _CW_STILO_MAGIC;
#endif
		a_stilo->type = STILOT_ARRAY;

		stila_l_gc_register(stil_stila_get(a_stil), (cw_stiloe_t
		    *)array);
	}
}

static void
stiloe_p_array_delete(cw_stiloe_t *a_stiloe, cw_stil_t *a_stil)
{
	cw_stiloe_array_t	*array;

	array = (cw_stiloe_array_t *)a_stiloe;

	_cw_check_ptr(array);
	_cw_assert(array->stiloe.magic == _CW_STILOE_MAGIC);
	_cw_assert(array->stiloe.type == STILOT_ARRAY);

	if (array->stiloe.indirect == FALSE && array->e.a.len > 0)
		_CW_FREE(array->e.a.arr);

	if (array->stiloe.locking && array->stiloe.indirect == FALSE)
		mtx_delete(&array->lock);

	_CW_STILOE_FREE(array);
}

static cw_stiloe_t *
stiloe_p_array_ref_iter(cw_stiloe_t *a_stiloe, cw_bool_t a_reset)
{
	cw_stiloe_t		*retval;
	cw_stiloe_array_t	*array;

	array = (cw_stiloe_array_t *)a_stiloe;

	if (a_reset)
		array->ref_iter = 0;

	if (a_stiloe->indirect) {
		if (array->ref_iter == 0) {
			retval = array->e.i.stilo.o.stiloe;
			array->ref_iter++;
		} else
			retval = NULL;
	} else {
		retval = NULL;
		while (retval == NULL && array->ref_iter < array->e.a.len) {
			retval =
			    stilo_stiloe_get(&array->e.a.arr[array->ref_iter]);
			array->ref_iter++;
		}
	}

	return retval;
}

static cw_stilte_t
stilo_p_array_print(cw_stilo_t *a_stilo, cw_stilo_t *a_file, cw_uint32_t
    a_depth)
{
	cw_stilte_t	retval;

	if (a_depth > 0) {
		cw_stilo_t	el;
		cw_uint32_t	nelms, i;

		if (a_stilo->attrs == STILOA_EXECUTABLE) {
			retval = stilo_file_output(a_file, "{");
			if (retval)
				goto RETURN;
		} else {
			retval = stilo_file_output(a_file, "[[");
			if (retval)
				goto RETURN;
		}

		nelms = stilo_array_len_get(a_stilo);
		for (i = 0; i < nelms; i++) {
			stilo_array_el_get(a_stilo, i, &el);
			retval = stilo_print(&el, a_file, a_depth - 1, FALSE);
			if (retval)
				goto RETURN;

			if (i < nelms - 1) {
				retval = stilo_file_output(a_file, " ");
				if (retval)
					goto RETURN;
			}
		}
		if (a_stilo->attrs == STILOA_EXECUTABLE) {
			retval = stilo_file_output(a_file, "}");
			if (retval)
				goto RETURN;
		} else {
			retval = stilo_file_output(a_file, "]");
			if (retval)
				goto RETURN;
		}
	} else {
		retval = stilo_file_output(a_file, "-array-");
		if (retval)
			goto RETURN;
	}

	retval = STILTE_NONE;
	RETURN:
	return retval;
}

void
stilo_array_copy(cw_stilo_t *a_to, cw_stilo_t *a_from)
{
	cw_stiloe_array_t	*array_fr, *array_fr_i = NULL, *array_fr_l;
	cw_stiloe_array_t	*array_to, *array_to_i = NULL, *array_to_l;
	cw_stilo_t		*arr_fr, *arr_to;
	cw_uint32_t		i, len_fr, len_to;

	/*
	 * Set array pointers.
	 */
	array_fr = (cw_stiloe_array_t *)a_from->o.stiloe;
	if (array_fr->stiloe.indirect)
		array_fr_i = (cw_stiloe_array_t *)array_fr->e.i.stilo.o.stiloe;
	array_to = (cw_stiloe_array_t *)a_to->o.stiloe;
	if (array_to->stiloe.indirect)
		array_to_i = (cw_stiloe_array_t *)array_to->e.i.stilo.o.stiloe;

	/*
	 * Set arr_fr and len_fr according to whether array_fr is an indirect
	 * object.
	 */
	if (array_fr_i != NULL) {
		array_fr_l = array_fr_i;
		arr_fr = &array_fr_i->e.a.arr[array_fr->e.i.beg_offset];
		len_fr = array_fr->e.i.len;
		_cw_assert(len_fr + array_fr->e.i.beg_offset <=
		    array_fr_i->e.a.len);
	} else {
		array_fr_l = array_fr;
		arr_fr = array_fr->e.a.arr;
		len_fr = array_fr->e.a.len;
	}

	/*
	 * Set arr_to and len_to according to whether array_to is an indirect
	 * object.
	 */
	if (array_to_i != NULL) {
		array_to_l = array_to_i;
		arr_to = &array_to_i->e.a.arr[array_to->e.i.beg_offset];
		len_to = array_to->e.i.len;
	} else {
		array_to_l = array_to;
		arr_to = array_to->e.a.arr;
		len_to = array_to->e.a.len;
	}

	/* Make sure destination is large enough. */
	_cw_assert(len_fr <= len_to);

	/*
	 * Iteratively copy elements.  Only copy one level deep (not
	 * recursively), by using dup.
	 */
	stiloe_p_array_lock(array_fr_l);
	stiloe_p_array_lock(array_to_l);
	for (i = 0; i < len_fr; i++)
		stilo_dup(&arr_to[i], &arr_fr[i]);
	stiloe_p_array_unlock(array_fr_l);

	/*
	 * Truncate the destination array if it is shorter than the source
	 * array.
	 */
	if (len_to > len_fr) {
		if (array_to_i != NULL)
			array_to->e.i.len = len_fr;
		else
			array_to->e.a.len = len_fr;
	}
	stiloe_p_array_unlock(array_to_l);
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

void
stilo_array_el_get(cw_stilo_t *a_stilo, cw_sint64_t a_offset, cw_stilo_t *r_el)
{
	cw_stiloe_array_t	*array;

	_cw_check_ptr(a_stilo);
	_cw_assert(a_stilo->magic == _CW_STILO_MAGIC);
	_cw_assert(a_stilo->type == STILOT_ARRAY);
	_cw_check_ptr(r_el);

	array = (cw_stiloe_array_t *)a_stilo->o.stiloe;

	_cw_check_ptr(array);
	_cw_assert(array->stiloe.magic == _CW_STILOE_MAGIC);
	_cw_assert(array->stiloe.type == STILOT_ARRAY);

	stiloe_p_array_lock(array);
	if (array->stiloe.indirect == FALSE) {
		_cw_assert(a_offset < array->e.a.len && a_offset >= 0);
		stilo_dup(r_el, &array->e.a.arr[a_offset]);
	} else {
		stilo_array_el_get(&array->e.i.stilo, a_offset +
		    array->e.i.beg_offset, r_el);
	}
	stiloe_p_array_unlock(array);
}

void
stilo_array_el_set(cw_stilo_t *a_stilo, cw_stilo_t *a_el, cw_sint64_t a_offset)
{
	cw_stiloe_array_t	*array;

	_cw_check_ptr(a_stilo);
	_cw_assert(a_stilo->magic == _CW_STILO_MAGIC);
	_cw_assert(a_stilo->type == STILOT_ARRAY);

	array = (cw_stiloe_array_t *)a_stilo->o.stiloe;

	_cw_check_ptr(array);
	_cw_assert(array->stiloe.magic == _CW_STILOE_MAGIC);
	_cw_assert(array->stiloe.type == STILOT_ARRAY);

	stiloe_p_array_lock(array);
	if (array->stiloe.indirect == FALSE) {
		_cw_assert(a_offset < array->e.a.len && a_offset >= 0);
		stilo_dup(&array->e.a.arr[a_offset], a_el);
	} else {
		stilo_array_el_set(&array->e.i.stilo, a_el, a_offset +
		    array->e.i.beg_offset);
	}
	stiloe_p_array_unlock(array);
}

/*
 * This function is unsafe to use for arrays with locking, so the public API
 * forces the use of stilo_array_el_get() instead.
 * However, the GC needs to cache pointers to the arrays in gcdict for
 * performance reasons, so it uses this function.
 */
cw_stilo_t *
stilo_l_array_get(cw_stilo_t *a_stilo)
{
	cw_stilo_t		*retval;
	cw_stiloe_array_t	*array;

	_cw_check_ptr(a_stilo);
	_cw_assert(a_stilo->magic == _CW_STILO_MAGIC);
	_cw_assert(a_stilo->type == STILOT_ARRAY);

	array = (cw_stiloe_array_t *)a_stilo->o.stiloe;

	_cw_check_ptr(array);
	_cw_assert(array->stiloe.magic == _CW_STILOE_MAGIC);
	_cw_assert(array->stiloe.type == STILOT_ARRAY);

	if (array->stiloe.indirect == FALSE)
		retval = array->e.a.arr;
	else {
		retval = &stilo_l_array_get(&array->e.i.stilo)
		    [array->e.i.beg_offset];
	}

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
stilo_p_boolean_print(cw_stilo_t *a_stilo, cw_stilo_t *a_file, cw_uint32_t
    a_depth)
{
	cw_stilte_t	retval;

	if (a_stilo->o.boolean.val)
		retval = stilo_file_output(a_file, "true");
	else
		retval = stilo_file_output(a_file, "false");

	return retval;
}

cw_bool_t
stilo_boolean_get(cw_stilo_t *a_stilo)
{
	_cw_check_ptr(a_stilo);
	_cw_assert(a_stilo->magic == _CW_STILO_MAGIC);
	_cw_assert(a_stilo->type == STILOT_BOOLEAN);

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
stilo_condition_new(cw_stilo_t *a_stilo, cw_stil_t *a_stil)
{
	cw_stiloe_condition_t	*condition;

	condition = (cw_stiloe_condition_t
	    *)_cw_malloc(sizeof(cw_stiloe_condition_t));

	stiloe_p_new(&condition->stiloe, STILOT_CONDITION, FALSE);
	cnd_new(&condition->condition);

	memset(a_stilo, 0, sizeof(cw_stilo_t));
	a_stilo->o.stiloe = (cw_stiloe_t *)condition;
#ifdef _LIBSTIL_DBG
	a_stilo->magic = _CW_STILO_MAGIC;
#endif
	a_stilo->type = STILOT_CONDITION;

	stila_l_gc_register(stil_stila_get(a_stil), (cw_stiloe_t *)condition);
}

static void
stiloe_p_condition_delete(cw_stiloe_t *a_stiloe, cw_stil_t *a_stil)
{
	cw_stiloe_condition_t	*condition;

	condition = (cw_stiloe_condition_t *)a_stiloe;

	_cw_check_ptr(condition);
	_cw_assert(condition->stiloe.magic == _CW_STILOE_MAGIC);
	_cw_assert(condition->stiloe.type == STILOT_CONDITION);

	cnd_delete(&condition->condition);

	_CW_STILOE_FREE(condition);
}

static cw_stiloe_t *
stiloe_p_condition_ref_iter(cw_stiloe_t *a_stiloe, cw_bool_t a_reset)
{
	return NULL;
}

static cw_stilte_t
stilo_p_condition_print(cw_stilo_t *a_stilo, cw_stilo_t *a_file, cw_uint32_t
    a_depth)
{
	cw_stilte_t	retval;

	retval = stilo_file_output(a_file, "-condition-");

	return retval;
}

void
stilo_condition_signal(cw_stilo_t *a_stilo)
{
	cw_stiloe_condition_t	*condition;

	_cw_check_ptr(a_stilo);
	_cw_assert(a_stilo->magic == _CW_STILO_MAGIC);
	_cw_assert(a_stilo->type == STILOT_CONDITION);

	condition = (cw_stiloe_condition_t *)a_stilo->o.stiloe;

	_cw_check_ptr(condition);
	_cw_assert(condition->stiloe.magic == _CW_STILOE_MAGIC);
	_cw_assert(condition->stiloe.type == STILOT_CONDITION);

	cnd_signal(&condition->condition);
}

void
stilo_condition_broadcast(cw_stilo_t *a_stilo)
{
	cw_stiloe_condition_t	*condition;

	_cw_check_ptr(a_stilo);
	_cw_assert(a_stilo->magic == _CW_STILO_MAGIC);
	_cw_assert(a_stilo->type == STILOT_CONDITION);

	condition = (cw_stiloe_condition_t *)a_stilo->o.stiloe;

	_cw_check_ptr(condition);
	_cw_assert(condition->stiloe.magic == _CW_STILOE_MAGIC);
	_cw_assert(condition->stiloe.type == STILOT_CONDITION);

	cnd_broadcast(&condition->condition);
}

void
stilo_condition_wait(cw_stilo_t *a_stilo, cw_stilo_t *a_mutex)
{
	cw_stiloe_condition_t	*condition;
	cw_stiloe_mutex_t	*mutex;

	_cw_check_ptr(a_stilo);
	_cw_assert(a_stilo->magic == _CW_STILO_MAGIC);
	_cw_assert(a_stilo->type == STILOT_CONDITION);

	condition = (cw_stiloe_condition_t *)a_stilo->o.stiloe;

	_cw_check_ptr(condition);
	_cw_assert(condition->stiloe.magic == _CW_STILOE_MAGIC);
	_cw_assert(condition->stiloe.type == STILOT_CONDITION);

	_cw_check_ptr(a_mutex);
	_cw_assert(a_mutex->magic == _CW_STILO_MAGIC);
	_cw_assert(a_mutex->type == STILOT_MUTEX);

	mutex = (cw_stiloe_mutex_t *)a_mutex->o.stiloe;

	_cw_check_ptr(mutex);
	_cw_assert(mutex->stiloe.magic == _CW_STILOE_MAGIC);
	_cw_assert(mutex->stiloe.type == STILOT_MUTEX);

	cnd_wait(&condition->condition, &mutex->lock);
}

cw_bool_t
stilo_condition_timedwait(cw_stilo_t *a_stilo, cw_stilo_t *a_mutex, const struct
    timespec *a_timeout)
{
	cw_bool_t		retval;
	cw_stiloe_condition_t	*condition;
	cw_stiloe_mutex_t	*mutex;

	_cw_check_ptr(a_stilo);
	_cw_assert(a_stilo->magic == _CW_STILO_MAGIC);
	_cw_assert(a_stilo->type == STILOT_CONDITION);

	condition = (cw_stiloe_condition_t *)a_stilo->o.stiloe;

	_cw_check_ptr(condition);
	_cw_assert(condition->stiloe.magic == _CW_STILOE_MAGIC);
	_cw_assert(condition->stiloe.type == STILOT_CONDITION);

	_cw_check_ptr(a_mutex);
	_cw_assert(a_mutex->magic == _CW_STILO_MAGIC);
	_cw_assert(a_mutex->type == STILOT_MUTEX);

	mutex = (cw_stiloe_mutex_t *)a_mutex->o.stiloe;

	_cw_check_ptr(mutex);
	_cw_assert(mutex->stiloe.magic == _CW_STILOE_MAGIC);
	_cw_assert(mutex->stiloe.type == STILOT_MUTEX);

	retval = cnd_timedwait(&condition->condition, &mutex->lock, a_timeout);
	
	return retval;
}

/*
 * dict.
 */
void
stilo_dict_new(cw_stilo_t *a_stilo, cw_stil_t *a_stil, cw_bool_t a_locking,
    cw_uint32_t a_dict_size)
{
	cw_stiloe_dict_t	*dict;

	dict = (cw_stiloe_dict_t *)_cw_malloc(sizeof(cw_stiloe_dict_t));

	stiloe_p_new(&dict->stiloe, STILOT_DICT, a_locking);
	if (a_locking)
		mtx_new(&dict->lock);
	dict->dicto = NULL;

	/*
	 * Don't create a dict smaller than _LIBSTIL_DICT_SIZE, since rounding
	 * errors for calculating the grow/shrink points can cause severe
	 * performance problems if the dict grows significantly.
	 *
	 * Don't let the table get more than 80% full, or less than 25% full,
	 * when shrinking.
	 */
	if (a_dict_size < _LIBSTIL_DICT_SIZE)
		a_dict_size = _LIBSTIL_DICT_SIZE;

	dch_new(&dict->hash, NULL, a_dict_size * 1.25, a_dict_size, a_dict_size
	    / 4, stilo_p_hash, stilo_p_key_comp);

	memset(a_stilo, 0, sizeof(cw_stilo_t));
	a_stilo->o.stiloe = (cw_stiloe_t *)dict;
#ifdef _LIBSTIL_DBG
	a_stilo->magic = _CW_STILO_MAGIC;
#endif
	a_stilo->type = STILOT_DICT;

	stila_l_gc_register(stil_stila_get(a_stil), (cw_stiloe_t *)dict);
}

static void
stiloe_p_dict_delete(cw_stiloe_t *a_stiloe, cw_stil_t *a_stil)
{
	cw_stiloe_dict_t	*dict;
	cw_stiloe_dicto_t	*dicto;
	cw_chi_t		*chi;

	dict = (cw_stiloe_dict_t *)a_stiloe;

	_cw_check_ptr(dict);
	_cw_assert(dict->stiloe.magic == _CW_STILOE_MAGIC);
	_cw_assert(dict->stiloe.type == STILOT_DICT);

	if (dict->stiloe.locking)
		mtx_delete(&dict->lock);
	while (dch_remove_iterate(&dict->hash, NULL, (void **)&dicto, &chi) ==
	    FALSE) {
		stila_dicto_put(stil_stila_get(a_stil), dicto);
		stila_chi_put(stil_stila_get(a_stil), chi);
	}

	_CW_STILOE_FREE(dict);
}

static cw_stiloe_t *
stiloe_p_dict_ref_iter(cw_stiloe_t *a_stiloe, cw_bool_t a_reset)
{
	cw_stiloe_t		*retval;
	cw_stiloe_dict_t	*dict;

	dict = (cw_stiloe_dict_t *)a_stiloe;

	if (a_reset) {
		dict->ref_iter = 0;
		dict->dicto = NULL;
	}

	retval = NULL;
	while (retval == NULL && dict->ref_iter < dch_count(&dict->hash)) {
		if (dict->dicto == NULL) {
			/* Key. */
			dch_get_iterate(&dict->hash, NULL, (void
			    **)&dict->dicto);
			retval = stilo_stiloe_get(&dict->dicto->key);
		} else {
			/* Value. */
			retval = stilo_stiloe_get(&dict->dicto->val);
			dict->ref_iter++;
			dict->dicto = NULL;
		}
	}

	return retval;
}

static cw_stilte_t
stilo_p_dict_print(cw_stilo_t *a_stilo, cw_stilo_t *a_file, cw_uint32_t
    a_depth)
{
	cw_stilte_t	retval;

	if (a_depth > 0) {
		cw_stilo_t	key, val;
		cw_uint32_t	count, i;

		retval = stilo_file_output(a_file, "<<");
		if (retval)
			goto RETURN;

		for (i = 0, count = stilo_dict_count(a_stilo); i < count; i++) {
			/* Get key and val. */
			stilo_dict_iterate(a_stilo, &key);
			stilo_dict_lookup(a_stilo, &key, &val);

			/* Print key. */
			retval = stilo_print(&key, a_file, a_depth - 1, FALSE);
			if (retval)
				goto RETURN;
			retval = stilo_file_output(a_file, " ");
			if (retval)
				goto RETURN;

			/* Print val. */
			retval = stilo_print(&val, a_file, a_depth - 1, FALSE);
			if (retval)
				goto RETURN;
			if (i < count - 1) {
				retval = stilo_file_output(a_file, " ");
				if (retval)
					goto RETURN;
			}
		}
		retval = stilo_file_output(a_file, ">>");
	} else {
		retval = stilo_file_output(a_file, "-dict-");
		if (retval)
			goto RETURN;
	}

	retval = STILTE_NONE;
	RETURN:
	return retval;
}

void
stilo_dict_copy(cw_stilo_t *a_to, cw_stilo_t *a_from, cw_stil_t *a_stil,
    cw_bool_t a_locking)
{
	cw_stiloe_dict_t	*to, *from;
	cw_uint32_t		i, count;
	cw_stiloe_dicto_t	*dicto_to, *dicto_from;
	cw_chi_t		*chi;

	_cw_check_ptr(a_to);
	_cw_assert(a_to->magic == _CW_STILO_MAGIC);
	_cw_assert(a_to->type == STILOT_DICT);
	_cw_assert(stilo_dict_count(a_to) == 0);

	from = (cw_stiloe_dict_t *)a_from->o.stiloe;

	_cw_check_ptr(from);
	_cw_assert(from->stiloe.magic == _CW_STILOE_MAGIC);
	_cw_assert(from->stiloe.type == STILOT_DICT);

	/* Deep (but not recursive) copy. */
	count = dch_count(&from->hash);
	stilo_dict_new(a_to, a_stil, a_locking, count);
	to = (cw_stiloe_dict_t *)a_to->o.stiloe;

	stiloe_p_dict_lock(from);
	for (i = 0, count = dch_count(&from->hash); i < count; i++) {
		/* Get a dicto. */
		dch_get_iterate(&from->hash, NULL, (void **)&dicto_from);

		/* Allocate and copy. */
		dicto_to = stila_dicto_get(stil_stila_get(a_stil));
		stilo_no_new(&dicto_to->key);
		stilo_dup(&dicto_to->key, &dicto_from->key);
		stilo_no_new(&dicto_to->val);
		stilo_dup(&dicto_to->val, &dicto_from->val);
		chi = stila_chi_get(stil_stila_get(a_stil));

		/* Insert. */
		dch_insert(&to->hash, &dicto_to->key, dicto_to, chi);
	}
	stiloe_p_dict_unlock(from);
}

void
stilo_dict_def(cw_stilo_t *a_stilo, cw_stil_t *a_stil, cw_stilo_t *a_key,
    cw_stilo_t *a_val)
{
	cw_stiloe_dict_t	*dict;
	cw_stiloe_dicto_t	*dicto;

	_cw_check_ptr(a_stilo);
	_cw_assert(a_stilo->magic == _CW_STILO_MAGIC);
	_cw_assert(a_stilo->type == STILOT_DICT);

	dict = (cw_stiloe_dict_t *)a_stilo->o.stiloe;

	_cw_check_ptr(dict);
	_cw_assert(dict->stiloe.magic == _CW_STILOE_MAGIC);
	_cw_assert(dict->stiloe.type == STILOT_DICT);

	stiloe_p_dict_lock(dict);
	if (dch_search(&dict->hash, (void *)a_key, (void **)&dicto) == FALSE) {
		/* a_key is already defined. */
		stilo_dup(&dicto->val, a_val);

		/*
		 * If (a_key == &dicto->val), things will break badly.  However,
		 * I can't think of a way that this could possibly happen in
		 * real use, so just assert.
		 */
		_cw_assert(a_key != &dicto->val);
	} else {
		cw_chi_t	*chi;

		/* Allocate and initialize. */
		dicto = stila_dicto_get(stil_stila_get(a_stil));
		chi = stila_chi_get(stil_stila_get(a_stil));
		stilo_no_new(&dicto->key);
		stilo_dup(&dicto->key, a_key);
		stilo_no_new(&dicto->val);
		stilo_dup(&dicto->val, a_val);

		/* Insert. */
		thd_crit_enter();
		dch_insert(&dict->hash, (void *)&dicto->key, (void *)dicto,
		    chi);
		thd_crit_leave();
	}
	stiloe_p_dict_unlock(dict);
}

void
stilo_dict_undef(cw_stilo_t *a_stilo, cw_stil_t *a_stil, const cw_stilo_t
    *a_key)
{
	cw_stiloe_dict_t	*dict;
	cw_stiloe_dicto_t	*dicto;
	cw_chi_t		*chi;
	cw_bool_t		error;

	_cw_check_ptr(a_stilo);
	_cw_assert(a_stilo->magic == _CW_STILO_MAGIC);
	_cw_assert(a_stilo->type == STILOT_DICT);

	dict = (cw_stiloe_dict_t *)a_stilo->o.stiloe;

	_cw_check_ptr(dict);
	_cw_assert(dict->stiloe.magic == _CW_STILOE_MAGIC);
	_cw_assert(dict->stiloe.type == STILOT_DICT);

	stiloe_p_dict_lock(dict);
	thd_crit_enter();
	error = dch_remove(&dict->hash, (void *)a_key, NULL, (void **)&dicto,
	    &chi);
	thd_crit_leave();
	stiloe_p_dict_unlock(dict);

	if (error == FALSE) {
		stila_dicto_put(stil_stila_get(a_stil), dicto);
		stila_chi_put(stil_stila_get(a_stil), chi);
	}
}

cw_bool_t
stilo_dict_lookup(cw_stilo_t *a_stilo, const cw_stilo_t *a_key, cw_stilo_t
    *r_stilo)
{
	cw_bool_t		retval;
	cw_stiloe_dict_t	*dict;
	cw_stiloe_dicto_t	*dicto;

	_cw_check_ptr(a_stilo);
	_cw_assert(a_stilo->magic == _CW_STILO_MAGIC);
	_cw_assert(a_stilo->type == STILOT_DICT);

	dict = (cw_stiloe_dict_t *)a_stilo->o.stiloe;

	_cw_check_ptr(dict);
	_cw_assert(dict->stiloe.magic == _CW_STILOE_MAGIC);
	_cw_assert(dict->stiloe.type == STILOT_DICT);

	stiloe_p_dict_lock(dict);
	if (dch_search(&dict->hash, (void *)a_key, (void **)&dicto) == FALSE) {
		if (r_stilo != NULL)
			stilo_dup(r_stilo, &dicto->val);
		retval = FALSE;
	} else
		retval = TRUE;
	stiloe_p_dict_unlock(dict);

	return retval;
}

/*
 * This function is generally unsafe to use, since the return value can
 * disappear due to GC before the pointer is turned into a legitimate reference.
 * However, the GC itself needs to cache pointers to the actual values inside
 * the dict for performance reasons, so it uses this function.
 */
cw_stilo_t *
stilo_l_dict_lookup(cw_stilo_t *a_stilo, const cw_stilo_t *a_key)
{
	cw_stilo_t		*retval;
	cw_stiloe_dict_t	*dict;
	cw_stiloe_dicto_t	*dicto;

	_cw_check_ptr(a_stilo);
	_cw_assert(a_stilo->magic == _CW_STILO_MAGIC);
	_cw_assert(a_stilo->type == STILOT_DICT);

	dict = (cw_stiloe_dict_t *)a_stilo->o.stiloe;

	_cw_check_ptr(dict);
	_cw_assert(dict->stiloe.magic == _CW_STILOE_MAGIC);
	_cw_assert(dict->stiloe.type == STILOT_DICT);

	stiloe_p_dict_lock(dict);
	if (dch_search(&dict->hash, (void *)a_key, (void **)&dicto) == FALSE) {
		retval = &dicto->val;
	} else
		retval = NULL;
	stiloe_p_dict_unlock(dict);

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

	_cw_check_ptr(dict);
	_cw_assert(dict->stiloe.magic == _CW_STILOE_MAGIC);
	_cw_assert(dict->stiloe.type == STILOT_DICT);

	stiloe_p_dict_lock(dict);
	retval = dch_count(&dict->hash);
	stiloe_p_dict_unlock(dict);

	return retval;
}

void
stilo_dict_iterate(cw_stilo_t *a_stilo, cw_stilo_t *r_stilo)
{
	cw_stiloe_dict_t	*dict;
	cw_stilo_t		*stilo;
	
	_cw_check_ptr(a_stilo);
	_cw_assert(a_stilo->magic == _CW_STILO_MAGIC);
	_cw_assert(a_stilo->type == STILOT_DICT);

	dict = (cw_stiloe_dict_t *)a_stilo->o.stiloe;

	_cw_check_ptr(dict);
	_cw_assert(dict->stiloe.magic == _CW_STILOE_MAGIC);
	_cw_assert(dict->stiloe.type == STILOT_DICT);

	stiloe_p_dict_lock(dict);
	dch_get_iterate(&dict->hash, (void **)&stilo, NULL);
	stilo_dup(r_stilo, stilo);
	stiloe_p_dict_unlock(dict);
}

/*
 * file.
 */
void
stilo_file_new(cw_stilo_t *a_stilo, cw_stil_t *a_stil, cw_bool_t a_locking)
{
	cw_stiloe_file_t	*file;

	file = (cw_stiloe_file_t *)_cw_malloc(sizeof(cw_stiloe_file_t));

	stiloe_p_new(&file->stiloe, STILOT_FILE, a_locking);
	if (a_locking)
		mtx_new(&file->lock);
	file->fd = -1;

	file->buffer = NULL;
	file->buffer_mode = BUFFER_EMPTY;
	file->buffer_size = 0;
	file->buffer_offset = 0;

	file->read_f = NULL;
	file->write_f = NULL;
	file->arg = NULL;
	file->position = 0;

	memset(a_stilo, 0, sizeof(cw_stilo_t));
	a_stilo->o.stiloe = (cw_stiloe_t *)file;
#ifdef _LIBSTIL_DBG
	a_stilo->magic = _CW_STILO_MAGIC;
#endif
	a_stilo->type = STILOT_FILE;

	stila_l_gc_register(stil_stila_get(a_stil), (cw_stiloe_t *)file);
}

static void
stiloe_p_file_delete(cw_stiloe_t *a_stiloe, cw_stil_t *a_stil)
{
	cw_stiloe_file_t	*file;
	cw_bool_t		ioerror = FALSE;

	file = (cw_stiloe_file_t *)a_stiloe;

	_cw_check_ptr(file);
	_cw_assert(file->stiloe.magic == _CW_STILOE_MAGIC);
	_cw_assert(file->stiloe.type == STILOT_FILE);

	if (file->buffer != NULL) {
		if (file->buffer_mode == BUFFER_WRITE) {
			switch (file->fd) {
			case -2: {
				cw_stilo_t	tstilo;

				stilo_p_new(&tstilo, STILOT_FILE);
				tstilo.o.stiloe = (cw_stiloe_t *)file;

				if (file->write_f(file->arg, &tstilo,
				    file->buffer, file->buffer_offset))
					ioerror = TRUE;
				break;
			}
			case -1:
				break;
			default:
				if (write(file->fd, file->buffer,
				    file->buffer_offset) == -1)
					ioerror = TRUE;
				break;
			}
		}
		_cw_free(file->buffer);
	}
	if (file->stiloe.locking)
		mtx_delete(&file->lock);
	/*
	 * Don't automatically close() predefined or wrapped descriptors.
	 */
	if (file->fd >= 3) {
		if (close(file->fd) == -1)
			ioerror = TRUE;
	}

	/*
	 * GC-induced deletion can get a write error, but there's no thread
	 * to report it to.  Use the initial thread for lack of a better place.
	 */
	if (ioerror)
		stilt_error(stil_stilt_get(a_stil), STILTE_IOERROR);

	_CW_STILOE_FREE(file);
}

static cw_stiloe_t *
stiloe_p_file_ref_iter(cw_stiloe_t *a_stiloe, cw_bool_t a_reset)
{
	return NULL;
}

static cw_stilte_t
stilo_p_file_print(cw_stilo_t *a_stilo, cw_stilo_t *a_file, cw_uint32_t
    a_depth)
{
	cw_stilte_t		retval;

	retval = stilo_file_output(a_file, "-file-");

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
	_cw_assert(file->stiloe.type == STILOT_FILE);
	_cw_assert(file->fd == -1);

	file->fd = a_fd;
}

void
stilo_file_interactive(cw_stilo_t *a_stilo, cw_stilo_file_read_t *a_read,
    cw_stilo_file_write_t *a_write, void *a_arg)
{
	cw_stiloe_file_t	*file;

	_cw_check_ptr(a_stilo);
	_cw_assert(a_stilo->magic == _CW_STILO_MAGIC);
	_cw_assert(a_stilo->type == STILOT_FILE);

	file = (cw_stiloe_file_t *)a_stilo->o.stiloe;

	_cw_check_ptr(file);
	_cw_assert(file->stiloe.magic == _CW_STILOE_MAGIC);
	_cw_assert(file->stiloe.type == STILOT_FILE);
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
	_cw_assert(file->stiloe.type == STILOT_FILE);

	stiloe_p_file_lock(file);
	if (file->fd != -1) {
		retval = STILTE_INVALIDFILEACCESS;
		goto URETURN;
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
			goto URETURN;
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
			goto URETURN;
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
			goto URETURN;
		}
		break;
	default:
		retval = STILTE_INVALIDFILEACCESS;
		goto URETURN;
	}

	file->fd = open(filename, access, 0x1ff);
	if (file->fd == -1) {
		switch (errno) {
		case ENOSPC:
		case EMFILE:
		case ENFILE:
			retval = STILTE_IOERROR;
			goto URETURN;
		default:
			retval = STILTE_INVALIDFILEACCESS;
			goto URETURN;
		}
	}

	retval = STILTE_NONE;
	URETURN:
	stiloe_p_file_unlock(file);
	RETURN:
	return retval;
}

/* STILTE_IOERROR */
cw_stilte_t
stilo_file_close(cw_stilo_t *a_stilo)
{
	cw_stilte_t		retval;
	cw_stiloe_file_t	*file;

	_cw_check_ptr(a_stilo);
	_cw_assert(a_stilo->magic == _CW_STILO_MAGIC);
	_cw_assert(a_stilo->type == STILOT_FILE);

	file = (cw_stiloe_file_t *)a_stilo->o.stiloe;

	_cw_check_ptr(file);
	_cw_assert(file->stiloe.magic == _CW_STILOE_MAGIC);
	_cw_assert(file->stiloe.type == STILOT_FILE);

	stiloe_p_file_lock(file);
	if (file->fd == -1) {
		retval = STILTE_IOERROR;
		goto RETURN;
	}

	/* Flush and get rid of the buffer if necessary. */
	retval = stilo_p_file_buffer_flush(a_stilo);
	if (retval)
		goto RETURN;
	if (file->buffer != NULL) {
		_cw_free(file->buffer);
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
	stiloe_p_file_unlock(file);
	return retval;
}

/* -1: STILTE_IOERROR */
cw_sint32_t
stilo_file_read(cw_stilo_t *a_stilo, cw_uint32_t a_len, cw_uint8_t *r_str)
{
	cw_sint32_t		retval;
	cw_stiloe_file_t	*file;

	_cw_check_ptr(a_stilo);
	_cw_assert(a_stilo->magic == _CW_STILO_MAGIC);
	_cw_assert(a_stilo->type == STILOT_FILE);

	file = (cw_stiloe_file_t *)a_stilo->o.stiloe;

	_cw_check_ptr(file);
	_cw_assert(file->stiloe.magic == _CW_STILOE_MAGIC);
	_cw_assert(file->stiloe.type == STILOT_FILE);

	stiloe_p_file_lock(file);
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
				struct pollfd	events;
				struct iovec	iov[2];

				if (retval == 0) {
					/*
					 * No data read yet.  Sleep until some
					 * data are available.
					 */
					events.fd = file->fd;
					events.events = POLLRDNORM;
					while ((poll(&events, 1, INFTIM)) == -1)
						; /* EINTR; try again. */

					/*
					 * Finish filling r_str and replenish
					 * the internal buffer.
					 */
					iov[0].iov_base = r_str;
					iov[0].iov_len = a_len;
					iov[1].iov_base = file->buffer;
					iov[1].iov_len = file->buffer_size;

					nread = readv(file->fd, iov, 2);
				} else {
					int	nready;

					/*
					 * Only read if data are available.
					 */
					events.fd = file->fd;
					events.events = POLLRDNORM;
					while ((nready = poll(&events, 1, 0)) ==
					    -1)
						; /* EINTR; try again. */

					if (nready == 1) {
						/*
						 * Finish filling r_str and
						 * replenish the internal
						 * buffer.
						 */
						iov[0].iov_base = r_str;
						iov[0].iov_len = a_len;
						iov[1].iov_base = file->buffer;
						iov[1].iov_len =
						    file->buffer_size;

						nread = readv(file->fd, iov, 2);
					} else
						nread = 0;
				}
			} else {
				/* Use the read wrapper function. */
				nread = file->read_f(file->arg, a_stilo, a_len,
				    r_str);
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
			retval = file->read_f(file->arg, a_stilo, a_len, r_str);
	}

	if (retval == 0) {
		file->fd = -1;
		if (file->buffer != NULL) {
			file->buffer_offset = 0;
			file->buffer_mode = BUFFER_EMPTY;
		}
	}
	RETURN:
	if (retval != -1)
		file->position += retval;
	stiloe_p_file_unlock(file);
	return retval;
}

/* STILTE_IOERROR */
cw_stilte_t
stilo_file_readline(cw_stilo_t *a_stilo, cw_stil_t *a_stil, cw_bool_t a_locking,
    cw_stilo_t *r_string, cw_bool_t *r_eof)
{
	cw_stilte_t		retval;
	cw_stiloe_file_t	*file;
	cw_uint8_t		*line, s_line[_CW_STILO_FILE_READLINE_BUFSIZE];
	cw_uint32_t		i, maxlen;
	cw_sint32_t		nread;
	enum {NORMAL, CR}	state;

	_cw_check_ptr(a_stilo);
	_cw_assert(a_stilo->magic == _CW_STILO_MAGIC);
	_cw_assert(a_stilo->type == STILOT_FILE);

	file = (cw_stiloe_file_t *)a_stilo->o.stiloe;

	_cw_check_ptr(file);
	_cw_assert(file->stiloe.magic == _CW_STILOE_MAGIC);
	_cw_assert(file->stiloe.type == STILOT_FILE);

	line = s_line;

	stiloe_p_file_lock(file);
	if (file->fd == -1) {
		retval = STILTE_IOERROR;
		goto RETURN;
	}

	/*
	 * Copy bytes until we see a \n or EOF.  Note that \r\n is converted to
	 * \n.
	 */
	if (file->buffer != NULL) {
		cw_uint32_t	offset;

		/* Flush the internal buffer if it has write data. */
		if (file->buffer_mode == BUFFER_WRITE) {
			retval = stilo_p_file_buffer_flush(a_stilo);
			if (retval)
				goto RETURN;
		}

		/*
		 * Copy bytes from the internal buffer, one at a time,
		 * replenishing the internal buffer as necessary.
		 */
		for (i = 0, offset = 0, maxlen =
		    _CW_STILO_FILE_READLINE_BUFSIZE, state = NORMAL;; i++) {
			/*
			 * If we're about to overflow the line buffer, expand
			 * it.
			 */
			if (i == maxlen) {
				if (line == s_line) {
					/* First expansion. */
					line = (cw_uint8_t *)_cw_malloc(maxlen
					    << 1);
					memcpy(line, s_line, maxlen);
				} else {
					cw_uint8_t	*oldline;

					/*
					 * We've already expanded at least
					 * once.
					 */
					oldline = line;
					line = (cw_uint8_t *)_cw_malloc(maxlen
					    << 1);
					memcpy(line, oldline, maxlen);
					_cw_free(oldline);
				}
				maxlen <<= 1;
			}

			if ((offset >= file->buffer_offset) ||
			    (file->buffer_mode == BUFFER_EMPTY)) {
				/* Replenish the internal buffer. */
				if (file->fd >= 0) {
					nread = read(file->fd, file->buffer,
					    file->buffer_size);
				} else {
					/* Use the read wrapper function. */
					nread = file->read_f(file->arg, a_stilo,
					    file->buffer_size,
					    file->buffer);
				}
				if (nread <= 0) {
					/* EOF. */
					file->fd = -1;
					file->buffer_offset = 0;
					file->buffer_mode = BUFFER_EMPTY;

					stilo_string_new(r_string, a_stil,
					    a_locking, i);
					stilo_string_lock(r_string);
					stilo_string_set(r_string, 0, line, i);
					stilo_string_unlock(r_string);
					file->position += i;

					retval = STILTE_NONE;
					*r_eof = TRUE;
					goto RETURN;
				}

				file->buffer_mode = BUFFER_READ;
				file->buffer_offset = nread;
				offset = 0;
			}

			line[i] = file->buffer[offset];
			offset++;

			switch (line[i]) {
			case '\r':
				state = CR;
				break;
			case '\n':
				if (state == CR) {
					/* Throw away the preceding \r. */
					i--;
				}
				stilo_string_new(r_string, a_stil, a_locking,
				    i);
				stilo_string_lock(r_string);
				stilo_string_set(r_string, 0, line, i);
				stilo_string_unlock(r_string);
				file->position += i;

				/*
				 * Shift the remaining buffered data to the
				 * beginning of the buffer.
				 */
				if (file->buffer_offset - offset > 0) {
					memmove(file->buffer,
					    &file->buffer[offset],
					    file->buffer_offset - offset);
					file->buffer_offset -= offset;
				} else {
					file->buffer_offset = 0;
					file->buffer_mode = BUFFER_EMPTY;
				}

				retval = STILTE_NONE;
				*r_eof = FALSE;
				goto RETURN;
			default:
				state = NORMAL;
				break;
			}
		}
	} else {
		/*
		 * There is no internal buffer, so we must read one byte at a
		 * time from the file.  This is horribly inneficient, but we
		 * don't have anywhere to unput characters past a newline.
		 */
		for (i = 0, maxlen = _CW_STILO_FILE_READLINE_BUFSIZE, state =
			 NORMAL;; i++) {
			/*
			 * If we're about to overflow the line buffer, expand
			 * it.
			 */
			if (i == maxlen) {
				if (line == s_line) {
					/* First expansion. */
					line = (cw_uint8_t *)_cw_malloc(maxlen
					    << 1);
					memcpy(line, s_line, maxlen);
				} else {
					cw_uint8_t	*oldline;

					/*
					 * We've already expanded at least
					 * once.
					 */
					oldline = line;
					line = (cw_uint8_t *)_cw_malloc(maxlen
					    << 1);
					memcpy(line, oldline, maxlen);
					_cw_free(oldline);
				}
				maxlen <<= 1;
			}

			if (file->fd >= 0)
				nread = read(file->fd, &line[i], 1);
			else {
				/* Use the read wrapper function. */
				nread = file->read_f(file->arg, a_stilo, 1,
				    &line[i]);
			}
			if (nread <= 0) {
				/* EOF. */
				stilo_string_new(r_string, a_stil, a_locking,
				    i);
				stilo_string_lock(r_string);
				stilo_string_set(r_string, 0, line, i);
				stilo_string_unlock(r_string);
				file->position += i;

				retval = STILTE_NONE;
				*r_eof = TRUE;
				goto RETURN;
			}

			switch (line[i]) {
			case '\r':
				state = CR;
				break;
			case '\n':
				if (state == CR) {
					/* Throw away the preceding \r. */
					i--;
				}
				stilo_string_new(r_string, a_stil, a_locking,
				    i);
				stilo_string_lock(r_string);
				stilo_string_set(r_string, 0, line, i);
				stilo_string_unlock(r_string);
				file->position += i;

				retval = STILTE_NONE;
				*r_eof = FALSE;
				goto RETURN;
			default:
				state = NORMAL;
				break;
			}
		}
	}

	RETURN:
	stiloe_p_file_unlock(file);
	if (line != s_line)
		_cw_free(line);
	return retval;
}

/* STILTE_IOERROR */
cw_stilte_t
stilo_file_write(cw_stilo_t *a_stilo, const cw_uint8_t *a_str, cw_uint32_t
    a_len)
{
	cw_stilte_t		retval;
	cw_stiloe_file_t	*file;

	_cw_check_ptr(a_stilo);
	_cw_assert(a_stilo->magic == _CW_STILO_MAGIC);
	_cw_assert(a_stilo->type == STILOT_FILE);

	file = (cw_stiloe_file_t *)a_stilo->o.stiloe;

	_cw_check_ptr(file);
	_cw_assert(file->stiloe.magic == _CW_STILOE_MAGIC);
	_cw_assert(file->stiloe.type == STILOT_FILE);

	stiloe_p_file_lock(file);
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
			retval = stilo_p_file_buffer_flush(a_stilo);
			if (retval)
				goto RETURN;

			if (file->write_f(file->arg, a_stilo, a_str, a_len)) {
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
			if (file->write_f(file->arg, a_stilo, a_str, a_len)) {
				retval = STILTE_IOERROR;
				goto RETURN;
			}
			file->position += a_len;
		}
	}

	retval = STILTE_NONE;
	RETURN:
	stiloe_p_file_unlock(file);
	return retval;
}

/* STILTE_IOERROR */
cw_stilte_t
stilo_file_output(cw_stilo_t *a_stilo, const char *a_format, ...)
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
	_cw_assert(file->stiloe.type == STILOT_FILE);

	stiloe_p_file_lock(file);
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
		if ((nwrite = out_put_svn(cw_g_out,
		    &file->buffer[file->buffer_offset], maxlen, a_format, ap))
		    == maxlen) {
			/*
			 * It probably didn't fit (there's definitely no space
			 * left over).
			 */
			va_end(ap);

			/* Flush the internal buffer. */
			retval = stilo_p_file_buffer_flush(a_stilo);
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
					    str, nwrite)) {
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
	stiloe_p_file_unlock(file);
	return retval;
}

/* STILTE_IOERROR */
cw_stilte_t
stilo_file_output_n(cw_stilo_t *a_stilo, cw_uint32_t a_size, const char
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
	_cw_assert(file->stiloe.type == STILOT_FILE);

	stiloe_p_file_lock(file);
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
			    a_size, a_format, ap);
			_cw_assert(nwrite == a_size);
			file->buffer_mode = BUFFER_WRITE;
			file->buffer_offset += nwrite;
		} else {
			/* It won't fit. */

			/* Flush the internal buffer, if necessary. */
			retval = stilo_p_file_buffer_flush(a_stilo);
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
				if (file->write_f(file->arg, a_stilo, str,
				    (nwrite < a_size) ? nwrite : a_size)) {
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
			if (file->write_f(file->arg, a_stilo, str, (nwrite <
			    a_size) ? nwrite : a_size)) {
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
	stiloe_p_file_unlock(file);
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
	_cw_assert(file->stiloe.type == STILOT_FILE);

	stiloe_p_file_lock(file);
	if (file->fd == -2) {
		retval = STILTE_IOERROR;
		goto RETURN;
	}

	stilo_p_file_buffer_flush(a_stilo);

	if (ftruncate(file->fd, a_length)) {
		retval = STILTE_IOERROR;
		goto RETURN;
	}

	retval = STILTE_NONE;
	RETURN:
	stiloe_p_file_unlock(file);
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
	_cw_assert(file->stiloe.type == STILOT_FILE);

	stiloe_p_file_lock(file);
	if (file->fd == -1) {
		retval = -1;
		goto RETURN;
	}

	if (file->fd == -2)
		retval = file->position;
	else
		retval = lseek(file->fd, 0, SEEK_CUR);

	RETURN:
	stiloe_p_file_unlock(file);
	return retval;
}

/* STILTE_IOERROR */
cw_stilte_t
stilo_file_position_set(cw_stilo_t *a_stilo, cw_sint64_t a_position)
{
	cw_stilte_t		retval;
	cw_stiloe_file_t	*file;

	_cw_check_ptr(a_stilo);
	_cw_assert(a_stilo->magic == _CW_STILO_MAGIC);
	_cw_assert(a_stilo->type == STILOT_FILE);

	file = (cw_stiloe_file_t *)a_stilo->o.stiloe;

	_cw_check_ptr(file);
	_cw_assert(file->stiloe.magic == _CW_STILOE_MAGIC);
	_cw_assert(file->stiloe.type == STILOT_FILE);

	stiloe_p_file_lock(file);
	if (file->fd < 0) {
		retval = STILTE_IOERROR;
		goto RETURN;
	}

	retval = stilo_p_file_buffer_flush(a_stilo);
	if (retval)
		goto RETURN;

	if (lseek(file->fd, a_position, SEEK_SET) == -1) {
		retval = STILTE_IOERROR;
		goto RETURN;
	}

	retval = STILTE_NONE;
	RETURN:
	stiloe_p_file_unlock(file);
	return retval;
}

cw_uint32_t
stilo_file_buffer_size_get(cw_stilo_t *a_stilo)
{
	cw_uint32_t		retval;
	cw_stiloe_file_t	*file;

	_cw_check_ptr(a_stilo);
	_cw_assert(a_stilo->magic == _CW_STILO_MAGIC);
	_cw_assert(a_stilo->type == STILOT_FILE);

	file = (cw_stiloe_file_t *)a_stilo->o.stiloe;

	_cw_check_ptr(file);
	_cw_assert(file->stiloe.magic == _CW_STILOE_MAGIC);
	_cw_assert(file->stiloe.type == STILOT_FILE);

	stiloe_p_file_lock(file);
	retval = file->buffer_size;
	stiloe_p_file_unlock(file);

	return retval;
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
	_cw_assert(file->stiloe.type == STILOT_FILE);

	stiloe_p_file_lock(file);
	if (a_size == 0) {
		if (file->buffer != NULL) {
			_cw_free(file->buffer);
			file->buffer = NULL;
			file->buffer_size = 0;
		}
	} else {
		if (file->buffer != NULL)
			_cw_free(file->buffer);
		file->buffer = (cw_uint8_t *)_cw_malloc(a_size);
		file->buffer_size = a_size;
	}
	file->buffer_mode = BUFFER_EMPTY;
	file->buffer_offset = 0;
	stiloe_p_file_unlock(file);
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
	_cw_assert(file->stiloe.type == STILOT_FILE);

	stiloe_p_file_lock(file);
	if ((file->fd != -1 && file->buffer != NULL && file->buffer_mode
	    != BUFFER_WRITE))
		retval = file->buffer_offset;
	else
		retval = 0;
	stiloe_p_file_unlock(file);

	return retval;
}

/* STILTE_IOERROR */
cw_stilte_t
stilo_file_buffer_flush(cw_stilo_t *a_stilo)
{
	cw_stilte_t		retval;
	cw_stiloe_file_t	*file;

	_cw_check_ptr(a_stilo);
	_cw_assert(a_stilo->magic == _CW_STILO_MAGIC);
	_cw_assert(a_stilo->type == STILOT_FILE);

	file = (cw_stiloe_file_t *)a_stilo->o.stiloe;

	_cw_check_ptr(file);
	_cw_assert(file->stiloe.magic == _CW_STILOE_MAGIC);
	_cw_assert(file->stiloe.type == STILOT_FILE);

	stiloe_p_file_lock(file);
	retval = stilo_p_file_buffer_flush(a_stilo);
	stiloe_p_file_unlock(file);

	return retval;
}

static cw_stilte_t
stilo_p_file_buffer_flush(cw_stilo_t *a_stilo)
{
	cw_stilte_t		retval;
	cw_stiloe_file_t	*file;

	file = (cw_stiloe_file_t *)a_stilo->o.stiloe;

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
				if (file->write_f(file->arg, a_stilo,
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
	_cw_assert(file->stiloe.type == STILOT_FILE);

	stiloe_p_file_lock(file);
	file->buffer_mode = BUFFER_EMPTY;
	file->buffer_offset = 0;
	stiloe_p_file_unlock(file);
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
	_cw_assert(file->stiloe.type == STILOT_FILE);

	stiloe_p_file_lock(file);
	if (file->fd != -1)
		retval = FALSE;
	else
		retval = TRUE;
	stiloe_p_file_unlock(file);

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
	_cw_assert(file->stiloe.type == STILOT_FILE);

	stiloe_p_file_lock(file);
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
	stiloe_p_file_unlock(file);
	return retval;
}

/*
 * hook.
 */
void
stilo_hook_new(cw_stilo_t *a_stilo, cw_stil_t *a_stil, void *a_data,
    cw_stilo_hook_exec_t *a_exec_f, cw_stilo_hook_ref_iter_t
    *a_ref_iter_f, cw_stilo_hook_delete_t *a_delete_f)
{
	cw_stiloe_hook_t	*hook;

	hook = (cw_stiloe_hook_t *)_cw_malloc(sizeof(cw_stiloe_hook_t));

	stiloe_p_new(&hook->stiloe, STILOT_HOOK, FALSE);
	hook->data = a_data;
	hook->exec_f = a_exec_f;
	hook->ref_iter_f = a_ref_iter_f;
	hook->delete_f = a_delete_f;

	memset(a_stilo, 0, sizeof(cw_stilo_t));
	a_stilo->o.stiloe = (cw_stiloe_t *)hook;
#ifdef _LIBSTIL_DBG
	a_stilo->magic = _CW_STILO_MAGIC;
#endif
	a_stilo->type = STILOT_HOOK;

	stila_l_gc_register(stil_stila_get(a_stil), (cw_stiloe_t *)hook);
}

static void
stiloe_p_hook_delete(cw_stiloe_t *a_stiloe, cw_stil_t *a_stil)
{
	cw_stiloe_hook_t	*hook;

	hook = (cw_stiloe_hook_t *)a_stiloe;

	_cw_check_ptr(hook);
	_cw_assert(hook->stiloe.magic == _CW_STILOE_MAGIC);
	_cw_assert(hook->stiloe.type == STILOT_FILE);

	if (hook->delete_f != NULL)
		hook->delete_f(hook->data, a_stil);

	_CW_STILOE_FREE(hook);
}

static cw_stiloe_t *
stiloe_p_hook_ref_iter(cw_stiloe_t *a_stiloe, cw_bool_t a_reset)
{
	cw_stiloe_t		*retval;
	cw_stiloe_hook_t	*hook;

	hook = (cw_stiloe_hook_t *)a_stiloe;

	if (hook->ref_iter_f != NULL)
		retval = hook->ref_iter_f(hook->data, a_reset);
	else
		retval = NULL;

	return retval;
}

static cw_stilte_t
stilo_p_hook_print(cw_stilo_t *a_stilo, cw_stilo_t *a_file, cw_uint32_t
    a_depth)
{
	cw_stilte_t	retval;

	retval = stilo_file_output(a_file, "-hook-");

	return retval;
}

void *
stilo_hook_get(cw_stilo_t *a_stilo)
{
	void			*retval;
	cw_stiloe_hook_t	*hook;

	_cw_check_ptr(a_stilo);
	_cw_assert(a_stilo->magic == _CW_STILO_MAGIC);
	_cw_assert(a_stilo->type == STILOT_HOOK);

	hook = (cw_stiloe_hook_t *)a_stilo->o.stiloe;

	_cw_check_ptr(hook);
	_cw_assert(hook->stiloe.magic == _CW_STILOE_MAGIC);
	_cw_assert(hook->stiloe.type == STILOT_FILE);

	retval = hook->data;

	return retval;
}

cw_stilte_t
stilo_hook_exec(cw_stilo_t *a_stilo, cw_stilt_t *a_stilt)
{
	cw_stilte_t	retval;
	cw_stiloe_hook_t	*hook;

	_cw_check_ptr(a_stilo);
	_cw_assert(a_stilo->magic == _CW_STILO_MAGIC);
	_cw_assert(a_stilo->type == STILOT_HOOK);

	hook = (cw_stiloe_hook_t *)a_stilo->o.stiloe;

	_cw_check_ptr(hook);
	_cw_assert(hook->stiloe.magic == _CW_STILOE_MAGIC);
	_cw_assert(hook->stiloe.type == STILOT_FILE);

	if (hook->exec_f == NULL) {
		retval = STILTE_INVALIDACCESS;
		goto RETURN;
	}

	retval = hook->exec_f(hook->data, a_stilt);
	RETURN:
	return retval;
}

/*
 * integer.
 */
static cw_stilte_t
stilo_p_integer_print(cw_stilo_t *a_stilo, cw_stilo_t *a_file, cw_uint32_t
    a_depth)
{
	cw_stilte_t	retval;
	
	retval = stilo_file_output(a_file, "[q|s:s]", a_stilo->o.integer.i);

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
stilo_p_mark_print(cw_stilo_t *a_stilo, cw_stilo_t *a_file, cw_uint32_t
    a_depth)
{
	cw_stilte_t	retval;
	
	retval = stilo_file_output(a_file, "-mark-");

	return retval;
}

/*
 * mutex.
 */
void
stilo_mutex_new(cw_stilo_t *a_stilo, cw_stil_t *a_stil)
{
	cw_stiloe_mutex_t	*mutex;

	mutex = (cw_stiloe_mutex_t *)_cw_malloc(sizeof(cw_stiloe_mutex_t));

	stiloe_p_new(&mutex->stiloe, STILOT_MUTEX, FALSE);
	mtx_new(&mutex->lock);

	memset(a_stilo, 0, sizeof(cw_stilo_t));
	a_stilo->o.stiloe = (cw_stiloe_t *)mutex;
#ifdef _LIBSTIL_DBG
	a_stilo->magic = _CW_STILO_MAGIC;
#endif
	a_stilo->type = STILOT_MUTEX;

	stila_l_gc_register(stil_stila_get(a_stil), (cw_stiloe_t *)mutex);
}

static void
stiloe_p_mutex_delete(cw_stiloe_t *a_stiloe, cw_stil_t *a_stil)
{
	cw_stiloe_mutex_t	*mutex;

	mutex = (cw_stiloe_mutex_t *)a_stiloe;

	_cw_check_ptr(mutex);
	_cw_assert(mutex->stiloe.magic == _CW_STILOE_MAGIC);
	_cw_assert(mutex->stiloe.type == STILOT_MUTEX);

	mtx_delete(&mutex->lock);

	_CW_STILOE_FREE(mutex);
}

static cw_stiloe_t *
stiloe_p_mutex_ref_iter(cw_stiloe_t *a_stilo, cw_bool_t a_reset)
{
	return NULL;
}

static cw_stilte_t
stilo_p_mutex_print(cw_stilo_t *a_stilo, cw_stilo_t *a_file, cw_uint32_t
    a_depth)
{
	cw_stilte_t	retval;

	retval = stilo_file_output(a_file, "-mutex-");

	return retval;
}

void
stilo_mutex_lock(cw_stilo_t *a_stilo)
{
	cw_stiloe_mutex_t	*mutex;

	_cw_check_ptr(a_stilo);
	_cw_assert(a_stilo->magic == _CW_STILO_MAGIC);
	_cw_assert(a_stilo->type == STILOT_MUTEX);

	mutex = (cw_stiloe_mutex_t *)a_stilo->o.stiloe;

	_cw_check_ptr(mutex);
	_cw_assert(mutex->stiloe.magic == _CW_STILOE_MAGIC);
	_cw_assert(mutex->stiloe.type == STILOT_MUTEX);

	mtx_lock(&mutex->lock);
}

cw_bool_t
stilo_mutex_trylock(cw_stilo_t *a_stilo)
{
	cw_bool_t		retval;
	cw_stiloe_mutex_t	*mutex;

	_cw_check_ptr(a_stilo);
	_cw_assert(a_stilo->magic == _CW_STILO_MAGIC);
	_cw_assert(a_stilo->type == STILOT_MUTEX);

	mutex = (cw_stiloe_mutex_t *)a_stilo->o.stiloe;

	_cw_check_ptr(mutex);
	_cw_assert(mutex->stiloe.magic == _CW_STILOE_MAGIC);
	_cw_assert(mutex->stiloe.type == STILOT_MUTEX);

	retval = mtx_trylock(&mutex->lock);

	return retval;
}

void
stilo_mutex_unlock(cw_stilo_t *a_stilo)
{
	cw_stiloe_mutex_t	*mutex;

	_cw_check_ptr(a_stilo);
	_cw_assert(a_stilo->magic == _CW_STILO_MAGIC);
	_cw_assert(a_stilo->type == STILOT_MUTEX);

	mutex = (cw_stiloe_mutex_t *)a_stilo->o.stiloe;

	_cw_check_ptr(mutex);
	_cw_assert(mutex->stiloe.magic == _CW_STILOE_MAGIC);
	_cw_assert(mutex->stiloe.type == STILOT_MUTEX);

	mtx_unlock(&mutex->lock);
}

/*
 * name.
 */
void
stilo_name_new(cw_stilo_t *a_stilo, cw_stil_t *a_stil, const cw_uint8_t
    *a_str, cw_uint32_t a_len, cw_bool_t a_is_static)
{
	cw_stiloe_name_t	*name, key;
	cw_mtx_t		*name_lock;
	cw_dch_t		*name_hash;

	/* Fake up a key so that we can search the hash tables. */
	key.str = a_str;
	key.len = a_len;

	name_lock = stil_l_name_lock_get(a_stil);
	name_hash = stil_l_name_hash_get(a_stil);

	/*
	 * Look in the global hash for the name.  If the name doesn't exist,
	 * create it.
	 */
	mtx_lock(name_lock);
	thd_crit_enter();
	if (dch_search(name_hash, (void *)&key, (void **)&name)) {
		/*
		 * Not found in the name hash.  Create, initialize, and insert
		 * a new entry.
		 */
		name = (cw_stiloe_name_t *)_cw_malloc(sizeof(cw_stiloe_name_t));

		stiloe_p_new(&name->stiloe, STILOT_NAME, FALSE);
		name->stiloe.name_static = a_is_static;
		name->len = a_len;

		if (a_is_static == FALSE) {
			name->str = _cw_malloc(a_len);
			/*
			 * Cast away the const here; it's one of two places that
			 * the string is allowed to be modified, and this cast
			 * is better than dropping the const altogether.
			 */
			memcpy((cw_uint8_t *)name->str, a_str, a_len);
		} else
			name->str = a_str;

		dch_insert(name_hash, (void *)name, (void **)name,
		    stila_chi_get(stil_stila_get(a_stil)));

		memset(a_stilo, 0, sizeof(cw_stilo_t));
		a_stilo->o.stiloe = (cw_stiloe_t *)name;
#ifdef _LIBSTIL_DBG
		a_stilo->magic = _CW_STILO_MAGIC;
#endif
		a_stilo->type = STILOT_NAME;

		stila_l_gc_register(stil_stila_get(a_stil), (cw_stiloe_t *)name);
	} else {
		memset(a_stilo, 0, sizeof(cw_stilo_t));
		a_stilo->o.stiloe = (cw_stiloe_t *)name;
#ifdef _LIBSTIL_DBG
		a_stilo->magic = _CW_STILO_MAGIC;
#endif
		a_stilo->type = STILOT_NAME;
	}
	thd_crit_leave();
	mtx_unlock(name_lock);
}

static void
stiloe_p_name_delete(cw_stiloe_t *a_stiloe, cw_stil_t *a_stil)
{
	cw_stiloe_name_t	*name;
	cw_mtx_t		*name_lock;
	cw_dch_t		*name_hash;
	cw_chi_t		*chi;

	name = (cw_stiloe_name_t *)a_stiloe;

	name_lock = stil_l_name_lock_get(a_stil);
	name_hash = stil_l_name_hash_get(a_stil);

	mtx_lock(name_lock);
	/*
	 * Only delete the hash entry if this object hasn't been put back into
	 * use.
	 */
	if (name->stiloe.color != stila_l_white_get(stil_stila_get(a_stil))) {
		/*
		 * Remove from hash table.
		 */
		dch_remove(name_hash, (void *)name, NULL, NULL, &chi);
		stila_chi_put(stil_stila_get(a_stil), chi);

		if (name->stiloe.name_static == FALSE) {
			/*
			 * Cast away the const here; it's one of two places that
			 * the string is allowed to be modified, and this cast
			 * is better than dropping the const altogether.
			 */
			_CW_FREE((cw_uint8_t *)name->str);
		}

		_CW_STILOE_FREE(name);
	} else {
		/* Re-register. */
		a_stiloe->registered = FALSE;
		stila_l_gc_register(stil_stila_get(a_stil), a_stiloe);
	}
	mtx_unlock(name_lock);
}

static cw_stiloe_t *
stiloe_p_name_ref_iter(cw_stiloe_t *a_stiloe, cw_bool_t a_reset)
{
	cw_stiloe_name_t	*name;

	name = (cw_stiloe_name_t *)a_stiloe;

	return NULL;
}

static cw_stilte_t
stilo_p_name_print(cw_stilo_t *a_stilo, cw_stilo_t *a_file, cw_uint32_t
    a_depth)
{
	cw_stilte_t		retval;
	cw_stiloe_name_t	*name;

	_cw_check_ptr(a_stilo);
	_cw_assert(a_stilo->magic == _CW_STILO_MAGIC);
	_cw_assert(a_stilo->type == STILOT_NAME);

	name = (cw_stiloe_name_t *)a_stilo->o.stiloe;

	_cw_check_ptr(name);
	_cw_assert(name->stiloe.magic == _CW_STILOE_MAGIC);
	_cw_assert(name->stiloe.type == STILOT_NAME);
	
	if (a_stilo->attrs == STILOA_LITERAL) {
		retval = stilo_file_output(a_file, "/");
		if (retval)
			goto RETURN;
	}

	retval = stilo_file_output_n(a_file, name->len, "[s]",
	    name->str);
	if (retval)
		goto RETURN;

	retval = STILTE_NONE;
	RETURN:
	return retval;
}

/* Hash {name string, length}. */
cw_uint32_t
stilo_l_name_hash(const void *a_key)
{
	cw_uint32_t		retval, i;
	cw_stiloe_name_t	*key = (cw_stiloe_name_t *)a_key;
	const char		*str;

	_cw_check_ptr(a_key);

	for (i = 0, str = key->str, retval = 0; i < key->len;
	    i++, str++)
		retval = retval * 33 + *str;

	return retval;
}

/* Compare keys {name string, length}. */
cw_bool_t
stilo_l_name_key_comp(const void *a_k1, const void *a_k2)
{
	cw_stiloe_name_t	*k1 = (cw_stiloe_name_t *)a_k1;
	cw_stiloe_name_t	*k2 = (cw_stiloe_name_t *)a_k2;
	size_t			len;

	_cw_check_ptr(a_k1);
	_cw_check_ptr(a_k2);

	if (k1->len > k2->len)
		len = k1->len;
	else
		len = k2->len;

	return strncmp((char *)k1->str, (char *)k2->str, len) ? FALSE
	    : TRUE;
}

const cw_uint8_t *
stilo_name_str_get(cw_stilo_t *a_stilo)
{
	const cw_uint8_t	*retval;
	cw_stiloe_name_t	*name;

	_cw_check_ptr(a_stilo);
	_cw_assert(a_stilo->magic == _CW_STILO_MAGIC);
	_cw_assert(a_stilo->type == STILOT_NAME);

	name = (cw_stiloe_name_t *)a_stilo->o.stiloe;

	_cw_check_ptr(name);
	_cw_assert(name->stiloe.magic == _CW_STILOE_MAGIC);
	_cw_assert(name->stiloe.type == STILOT_NAME);

	retval = name->str;

	return retval;
}

cw_uint32_t
stilo_name_len_get(cw_stilo_t *a_stilo)
{
	cw_uint32_t		retval;
	cw_stiloe_name_t	*name;

	_cw_check_ptr(a_stilo);
	_cw_assert(a_stilo->magic == _CW_STILO_MAGIC);
	_cw_assert(a_stilo->type == STILOT_NAME);

	name = (cw_stiloe_name_t *)a_stilo->o.stiloe;

	_cw_check_ptr(name);
	_cw_assert(name->stiloe.magic == _CW_STILOE_MAGIC);
	_cw_assert(name->stiloe.type == STILOT_NAME);

	retval = name->len;

	return retval;
}

/*
 * null.
 */
static cw_stilte_t
stilo_p_null_print(cw_stilo_t *a_stilo, cw_stilo_t *a_file, cw_uint32_t
    a_depth)
{
	cw_stilte_t	retval;
	
	retval = stilo_file_output(a_file, "null");

	return retval;
}

/*
 * operator.
 */
void
stilo_operator_new(cw_stilo_t *a_stilo, cw_op_t *a_op, cw_stiln_t a_stiln)
{
	stilo_p_new(a_stilo, STILOT_OPERATOR);
	a_stilo->o.operator.f = a_op;
	a_stilo->op_code = a_stiln;
}

static cw_stilte_t
stilo_p_operator_print(cw_stilo_t *a_stilo, cw_stilo_t *a_file, cw_uint32_t
    a_depth)
{
	cw_stilte_t	retval;
	
	if (a_stilo->op_code != STILN_ZERO) {
		_cw_assert(a_stilo->op_code <= STILN_LAST);
		if (a_stilo->fast_op) {
			retval = stilo_file_output(a_file,
			    "---[s]---", stiln_str(a_stilo->op_code));
		} else {
			retval = stilo_file_output(a_file, "--[s]--",
			    stiln_str(a_stilo->op_code));
		}
	} else
		retval = stilo_file_output(a_file, "-operator-");

	return retval;
}

/*
 * string.
 */
void
stilo_string_new(cw_stilo_t *a_stilo, cw_stil_t *a_stil, cw_bool_t a_locking,
    cw_uint32_t a_len)
{
	cw_stiloe_string_t	*string;

	string = (cw_stiloe_string_t *)_cw_malloc(sizeof(cw_stiloe_string_t));

	stiloe_p_new(&string->stiloe, STILOT_STRING, a_locking);
	if (a_locking)
		mtx_new(&string->lock);
	string->e.s.len = a_len;
	if (string->e.s.len > 0) {
		string->e.s.str = (cw_uint8_t *)_cw_malloc(string->e.s.len);
		memset(string->e.s.str, 0, string->e.s.len);
	} else
		string->e.s.str = NULL;

	memset(a_stilo, 0, sizeof(cw_stilo_t));
	a_stilo->o.stiloe = (cw_stiloe_t *)string;
#ifdef _LIBSTIL_DBG
	a_stilo->magic = _CW_STILO_MAGIC;
#endif
	a_stilo->type = STILOT_STRING;

	stila_l_gc_register(stil_stila_get(a_stil), (cw_stiloe_t *)string);
}

void
stilo_string_substring_new(cw_stilo_t *a_stilo, cw_stilo_t *a_string, cw_stil_t
    *a_stil, cw_uint32_t a_offset, cw_uint32_t a_len)
{
	cw_stiloe_string_t	*string, *orig;

	_cw_check_ptr(a_stilo);

	orig = (cw_stiloe_string_t *)a_string->o.stiloe;
	_cw_check_ptr(orig);
	_cw_assert(orig->stiloe.magic == _CW_STILOE_MAGIC);

	if (orig->stiloe.indirect) {
		stilo_string_substring_new(a_stilo, &orig->e.i.stilo, a_stil,
		    a_offset + orig->e.i.beg_offset, a_len);
	} else {
		_cw_assert(a_offset + a_len <= orig->e.s.len);

		string = (cw_stiloe_string_t
		    *)_cw_malloc(sizeof(cw_stiloe_string_t));

		stiloe_p_new(&string->stiloe, STILOT_STRING, FALSE);
		string->stiloe.indirect = TRUE;
		memcpy(&string->e.i.stilo, a_string, sizeof(cw_stilo_t));
		string->e.i.beg_offset = a_offset;
		string->e.i.len = a_len;

		memset(a_stilo, 0, sizeof(cw_stilo_t));
		a_stilo->o.stiloe = (cw_stiloe_t *)string;
#ifdef _LIBSTIL_DBG
		a_stilo->magic = _CW_STILO_MAGIC;
#endif
		a_stilo->type = STILOT_STRING;

		stila_l_gc_register(stil_stila_get(a_stil), (cw_stiloe_t
		    *)string);
	}
}

static void
stiloe_p_string_delete(cw_stiloe_t *a_stiloe, cw_stil_t *a_stil)
{
	cw_stiloe_string_t	*string;

	string = (cw_stiloe_string_t *)a_stiloe;

	_cw_check_ptr(string);
	_cw_assert(string->stiloe.magic == _CW_STILOE_MAGIC);
	_cw_assert(string->stiloe.type == STILOT_STRING);

	if (string->stiloe.indirect == FALSE && string->e.s.len > 0)
		_CW_FREE(string->e.s.str);

	if (string->stiloe.locking && string->stiloe.indirect == FALSE)
		mtx_delete(&string->lock);

	_CW_STILOE_FREE(string);
}

static cw_stiloe_t *
stiloe_p_string_ref_iter(cw_stiloe_t *a_stiloe, cw_bool_t a_reset)
{
	cw_stiloe_t		*retval;
	cw_stiloe_string_t	*string;

	string = (cw_stiloe_string_t *)a_stiloe;

	if (a_reset)
		string->ref_iter = 0;

	if (a_stiloe->indirect == FALSE)
		retval = NULL;
	else if (string->ref_iter == 0) {
		retval = string->e.i.stilo.o.stiloe;
		string->ref_iter++;
	} else
		retval = NULL;

	return retval;
}

static cw_stilte_t
stilo_p_string_print(cw_stilo_t *a_stilo, cw_stilo_t *a_file, cw_uint32_t
    a_depth)
{
	cw_stilte_t	retval;
	cw_uint8_t	*str;
	cw_sint32_t	len;
	cw_uint32_t	i;

	str = stilo_string_get(a_stilo);
	len = stilo_string_len_get(a_stilo);

	stilo_file_output(a_file, "(");
	stilo_string_lock(a_stilo);
	for (i = 0; i < len; i++) {
		switch (str[i]) {
		case '\n':
			retval = stilo_file_output(a_file, "\\n");
			break;
		case '\r':
			retval = stilo_file_output(a_file, "\\r");
			break;
		case '\t':
			retval = stilo_file_output(a_file, "\\t");
			break;
		case '\b':
			retval = stilo_file_output(a_file, "\\b");
			break;
		case '\f':
			retval = stilo_file_output(a_file, "\\f");
			break;
		case '\\':
			retval = stilo_file_output(a_file, "\\\\");
			break;
		case '(':
			retval = stilo_file_output(a_file, "\\(");
			break;
		case ')':
			retval = stilo_file_output(a_file, "\\)");
			break;
		default:
			if (isprint(str[i]))
				retval = stilo_file_output(a_file, "[c]",
				    str[i]);
			else {
				retval = stilo_file_output(a_file,
				    "\\x[i|b:16|w:2|p:0]", str[i]);
			}
			break;
		}
		if (retval) {
			stilo_string_unlock(a_stilo);
			goto RETURN;
		}
	}
	stilo_string_unlock(a_stilo);
	retval = stilo_file_output(a_file, ")");
	if (retval)
		goto RETURN;

	retval = STILTE_NONE;
	RETURN:
	return retval;
}

void
stilo_string_copy(cw_stilo_t *a_to, cw_stilo_t *a_from)
{
	cw_stiloe_string_t	*string_fr, *string_fr_i = NULL, *string_fr_l;
	cw_stiloe_string_t	*string_to, *string_to_i = NULL, *string_to_l;
	cw_uint8_t		*str_fr, *str_to;
	cw_uint32_t		len_fr, len_to;

	/*
	 * Set string pointers.
	 */
	string_fr = (cw_stiloe_string_t *)a_from->o.stiloe;
	if (string_fr->stiloe.indirect) {
		string_fr_i = (cw_stiloe_string_t
		    *)string_fr->e.i.stilo.o.stiloe;
	}
	string_to = (cw_stiloe_string_t *)a_to->o.stiloe;
	if (string_to->stiloe.indirect) {
		string_to_i = (cw_stiloe_string_t
		    *)string_to->e.i.stilo.o.stiloe;
	}

	/*
	 * Set str_fr and len_fr according to whether string_fr is an indirect
	 * object.
	 */
	if (string_fr_i != NULL) {
		string_fr_l = string_fr_i;
		str_fr = &string_fr_i->e.s.str[string_fr->e.i.beg_offset];
		len_fr = string_fr->e.i.len;
		_cw_assert(len_fr + string_fr->e.i.beg_offset <=
		    string_fr_i->e.s.len);
	} else {
		string_fr_l = string_fr;
		str_fr = string_fr->e.s.str;
		len_fr = string_fr->e.s.len;
	}

	/*
	 * Set str_to and len_to according to whether string_to is an indirect
	 * object.
	 */
	if (string_to_i != NULL) {
		string_to_l = string_to_i;
		str_to = &string_to_i->e.s.str[string_to->e.i.beg_offset];
		len_to = string_to->e.i.len;
	} else {
		string_to_l = string_to;
		str_to = string_to->e.s.str;
		len_to = string_to->e.s.len;
	}

	/* Make sure destination is large enough. */
	_cw_assert(len_fr <= len_to);

	/*
	 * Iteratively copy elements.  Only copy one level deep (not
	 * recursively), by using dup.
	 */
	stiloe_p_string_lock(string_fr_l);
	stiloe_p_string_lock(string_to_l);
	memcpy(str_to, str_fr, len_fr);
	stiloe_p_string_unlock(string_fr_l);

	/*
	 * Truncate the destination string if it is shorter than the source
	 * string.
	 */
	if (len_to > len_fr) {
		if (string_to_i != NULL)
			string_to->e.i.len = len_fr;
		else
			string_to->e.s.len = len_fr;
	}
	stiloe_p_string_unlock(string_to_l);
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

	_cw_check_ptr(string);
	_cw_assert(string->stiloe.magic == _CW_STILOE_MAGIC);
	_cw_assert(string->stiloe.type == STILOT_STRING);

	if (string->stiloe.indirect == FALSE)
		retval = string->e.s.len;
	else
		retval = string->e.i.len;

	return retval;
}

void
stilo_string_el_get(cw_stilo_t *a_stilo, cw_sint64_t a_offset, cw_uint8_t *r_el)
{
	cw_stiloe_string_t	*string;

	_cw_check_ptr(a_stilo);
	_cw_assert(a_stilo->magic == _CW_STILO_MAGIC);
	_cw_assert(a_stilo->type == STILOT_STRING);

	string = (cw_stiloe_string_t *)a_stilo->o.stiloe;

	_cw_check_ptr(string);
	_cw_assert(string->stiloe.magic == _CW_STILOE_MAGIC);
	_cw_assert(string->stiloe.type == STILOT_STRING);

	if (string->stiloe.indirect == FALSE) {
		_cw_assert(a_offset >= 0 && a_offset < string->e.s.len);
		*r_el = string->e.s.str[a_offset];
	} else {
		stilo_string_el_get(&string->e.i.stilo, a_offset +
		    string->e.i.beg_offset, r_el);
	}
}

void
stilo_string_el_set(cw_stilo_t *a_stilo, cw_uint8_t a_el, cw_sint64_t a_offset)
{
	cw_stiloe_string_t	*string;

	_cw_check_ptr(a_stilo);
	_cw_assert(a_stilo->magic == _CW_STILO_MAGIC);
	_cw_assert(a_stilo->type == STILOT_STRING);

	string = (cw_stiloe_string_t *)a_stilo->o.stiloe;

	_cw_check_ptr(string);
	_cw_assert(string->stiloe.magic == _CW_STILOE_MAGIC);
	_cw_assert(string->stiloe.type == STILOT_STRING);

	if (string->stiloe.indirect == FALSE) {
		_cw_assert(a_offset >= 0 && a_offset < string->e.s.len);
		string->e.s.str[a_offset] = a_el;
	} else {
		stilo_string_el_set(&string->e.i.stilo, a_el, a_offset +
		    string->e.i.beg_offset);
	}
}

void
stilo_string_lock(cw_stilo_t *a_stilo)
{
	cw_stiloe_string_t	*string;

	_cw_check_ptr(a_stilo);
	_cw_assert(a_stilo->magic == _CW_STILO_MAGIC);
	_cw_assert(a_stilo->type == STILOT_STRING);

	string = (cw_stiloe_string_t *)a_stilo->o.stiloe;

	_cw_check_ptr(string);
	_cw_assert(string->stiloe.magic == _CW_STILOE_MAGIC);
	_cw_assert(string->stiloe.type == STILOT_STRING);

	if (string->stiloe.indirect == FALSE)
		stiloe_p_string_lock(string);
	else
		stilo_string_lock(&string->e.i.stilo);
}

void
stilo_string_unlock(cw_stilo_t *a_stilo)
{
	cw_stiloe_string_t	*string;

	_cw_check_ptr(a_stilo);
	_cw_assert(a_stilo->magic == _CW_STILO_MAGIC);
	_cw_assert(a_stilo->type == STILOT_STRING);

	string = (cw_stiloe_string_t *)a_stilo->o.stiloe;

	_cw_check_ptr(string);
	_cw_assert(string->stiloe.magic == _CW_STILOE_MAGIC);
	_cw_assert(string->stiloe.type == STILOT_STRING);

	if (string->stiloe.indirect == FALSE)
		stiloe_p_string_unlock(string);
	else
		stilo_string_unlock(&string->e.i.stilo);
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

	_cw_check_ptr(string);
	_cw_assert(string->stiloe.magic == _CW_STILOE_MAGIC);
	_cw_assert(string->stiloe.type == STILOT_STRING);

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
	_cw_assert(a_stilo->type == STILOT_STRING);

	string = (cw_stiloe_string_t *)a_stilo->o.stiloe;

	_cw_check_ptr(string);
	_cw_assert(string->stiloe.magic == _CW_STILOE_MAGIC);
	_cw_assert(string->stiloe.type == STILOT_STRING);

	/* Get the string pointer. */
	if (string->stiloe.indirect == FALSE) {
		_cw_assert(a_offset + a_len <= string->e.s.len);
		str = string->e.s.str;

		memcpy(&str[a_offset], a_str, a_len);
	} else
		stilo_string_set(&string->e.i.stilo, a_offset, a_str, a_len);
}
