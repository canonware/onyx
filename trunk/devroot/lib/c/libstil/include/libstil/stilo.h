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

#ifdef _LIBSTIL_DBG
#define _CW_STILO_MAGIC		0x398754ba
#endif

typedef struct cw_stilo_s cw_stilo_t;
typedef struct cw_stiloe_s cw_stiloe_t;
typedef struct cw_stiloe_dicto_s cw_stiloe_dicto_t;

/* Declared here to avoid a circular header dependency. */
typedef struct cw_stila_s cw_stila_t;
typedef struct cw_stilag_s cw_stilag_t;
typedef struct cw_stilat_s cw_stilat_t;
typedef struct cw_stilt_s cw_stilt_t;
typedef struct cw_stilsc_s cw_stilsc_t;
typedef struct cw_stiloe_dict_s cw_stiloe_dict_t;

/* Interpreter errors. */
typedef enum {
	STILTE_NONE,			/* No error. */
	STILTE_DICTSTACKOVERFLOW,	/* dstack too deep. */
	STILTE_DICTSTACKUNDERFLOW,	/* No poppable dictionary on dstack. */
	STILTE_EXECSTACKOVERFLOW,	/* estack too deep. */
	STILTE_INTERRUPT,		/* Interrupt. */
	STILTE_INVALIDACCESS,		/* Permission error. */
	STILTE_INVALIDCONTEXT,		/* Bad thread context. */
	STILTE_INVALIDEXIT,		/* exit operator called outside loop. */
	STILTE_INVALIDFILEACCESS,	/* Insufficient file permissions. */
	STILTE_IOERROR,			/* read()/write()/etc. error. */
	STILTE_LIMITCHECK,		/* Value outside legal range. */
	STILTE_RANGECHECK,		/* Out of bounds string or array use. */
	STILTE_STACKOVERFLOW,		/* ostack too full. */
	STILTE_STACKUNDERFLOW,		/* Not enough objects on ostack. */
	STILTE_SYNTAXERROR,		/* Scanner syntax error. */
	STILTE_TIMEOUT,			/* Timeout. */
	STILTE_TYPECHECK,		/* Incorrect argument type. */
	STILTE_UNDEFINED,		/* Object not found in dstack. */
	STILTE_UNDEFINEDFILENAME,	/* Bad filename. */
	STILTE_UNDEFINEDRESOURCE,	/* Resource not found. */
	STILTE_UNDEFINEDRESULT,
	STILTE_UNMATCHEDMARK,		/* No mark on ostack. */
	STILTE_UNREGISTERED,		/* Other non-enumerated error. */
	STILTE_VMERROR			/* Out of memory. */
#define	STILTE_LAST	STILTE_VMERROR
} cw_stilte_t;

typedef cw_sint32_t cw_stil_read_t (void *a_arg, cw_stilo_t *a_file, cw_stilt_t
    *a_stilt, cw_uint32_t a_len, cw_uint8_t *r_str);
typedef cw_bool_t cw_stil_write_t (void *a_arg, cw_stilo_t *a_file, cw_stilt_t
    *a_stilt, const cw_uint8_t *a_str, cw_uint32_t a_len);

/* Defined here to resolve circular dependencies. */
typedef void cw_op_t(cw_stilt_t *);

typedef enum {
	STILOT_NO		=  0,
	STILOT_ARRAY		=  1,
	STILOT_BOOLEAN		=  2,
	STILOT_CONDITION	=  3,
	STILOT_DICT		=  4,
	STILOT_FILE		=  5,
	STILOT_HOOK		=  6,
	STILOT_INTEGER		=  7,
	STILOT_LOCK		=  8,
	STILOT_MARK		=  9,
	STILOT_NAME		= 10,
	STILOT_NULL		= 11,
	STILOT_OPERATOR		= 12,
	STILOT_STRING		= 13
}	cw_stilot_t;

/* Attributes. */
typedef enum {
	STILOA_LITERAL		=  0,
	STILOA_EXECUTABLE	=  1
}	cw_stiloa_t;

/* Permissions. */
typedef enum {
	STILOP_UNLIMITED	=  0,
	STILOP_READONLY		=  1,
	STILOP_EXECUTEONLY	=  2,
	STILOP_NONE		=  3
}	cw_stilop_t;

/*
 * Main object structure.
 */
struct cw_stilo_s {
#if (defined(_LIBSTIL_DBG) || defined(_LIBSTIL_DEBUG))
	cw_uint32_t	magic;
#endif

	/*
	 * Type.
	 */
	cw_stilot_t	type:4;
	/*
	 * Attributes.  A stilo is either LITERAL or EXECUTABLE.
	 */
	cw_stiloa_t	attrs:1;
	/*
	 * Permissions.  A stiloe has read and execute permissions.
	 */
	cw_stilop_t	perms:2;
	/*
	 * If TRUE, there is a breakpoint set on this object.  In general, this
	 * field is not looked at unless the interpreter has been put into
	 * debugging mode.
	 */
	cw_bool_t	breakpoint:1;
	/*
	 * If TRUE, there is a watchpoint set on this object.  In general, this
	 * field is not looked at unless the interpreter has been put into
	 * debugging mode. Note that setting a watchpoint on a reference to an
	 * extended type only detects changes that are made via that particular
	 * reference to the extension.
	 */
	cw_bool_t	watchpoint:1;

	union {
		struct {
			cw_bool_t	val;
		}	boolean;
		struct {
			cw_sint64_t	i;
		}	integer;
		struct {
			cw_op_t		*f;
		}	operator;
		cw_stiloe_t	*stiloe;
	}	o;
};

/* This is private, but stila needs to know its size. */
struct cw_stiloe_dicto_s {
	cw_stilo_t	key;
	cw_stilo_t	val;
};

void		stilo_delete(cw_stilo_t *a_stilo, cw_stilt_t *a_stilt);

cw_sint32_t	stilo_compare(cw_stilo_t *a_a, cw_stilo_t *a_b, cw_stilt_t
    *a_stilt);
#define		stilo_dup(a_to, a_from) do {				\
	/* Copy. */							\
	memcpy((a_to), (a_from), sizeof(cw_stilo_t));			\
									\
	/* Reset debug flags on new copy. */				\
	(a_to)->breakpoint = FALSE;					\
	(a_to)->watchpoint = FALSE;					\
} while (0)
	
void		stilo_move(cw_stilo_t *a_to, cw_stilo_t *a_from);

#define		stilo_type_get(a_stilo)	(a_stilo)->type
cw_bool_t	stilo_global_get(cw_stilo_t *a_stilo);
cw_bool_t	stilo_local_get(cw_stilo_t *a_stilo);

#define		stilo_attrs_get(a_stilo) (a_stilo)->attrs
#define		stilo_attrs_set(a_stilo, a_attrs) (a_stilo)->attrs = (a_attrs)

#define		stilo_perms_get(a_stilo) (a_stilo)->perms
#define		stilo_perms_set(a_stilo, a_perms) (a_stilo)->perms = (a_perms)

void		stilo_print(cw_stilo_t *a_stilo, cw_stilt_t *a_stilt, cw_stilo_t
    *a_file, cw_bool_t a_syntactic, cw_bool_t a_newline);

/* XXX For the GC only. */
cw_stiloe_t	*stilo_l_stiloe_get(cw_stilo_t *a_stilo);
cw_stiloe_t	*stiloe_l_ref_iterate(cw_stiloe_t *a_stiloe, cw_bool_t a_reset);

/*
 * no.
 */
#ifdef _LIBSTIL_DBG
#define	stilo_no_new(a_stilo) do {					\
	memset((a_stilo), 0, sizeof(cw_stilo_t));			\
	(a_stilo)->type = STILOT_NO;					\
	(a_stilo)->magic = _CW_STILO_MAGIC;				\
} while (0)
#else
#define	stilo_no_new(a_stilo) do {					\
	memset((a_stilo), 0, sizeof(cw_stilo_t));			\
	(a_stilo)->type = STILOT_NO;					\
} while (0)
#endif

/*
 * array.
 */
void		stilo_array_new(cw_stilo_t *a_stilo, cw_stilt_t *a_stilt,
    cw_uint32_t a_len);
cw_stilte_t	stilo_array_subarray_new(cw_stilo_t *a_stilo, cw_stilo_t
    *a_array, cw_stilt_t *a_stilt, cw_uint32_t a_offset, cw_uint32_t a_len);
cw_stilte_t	stilo_array_copy(cw_stilo_t *a_to, cw_stilo_t *a_from,
    cw_stilt_t *a_stilt);
cw_uint32_t	stilo_array_len_get(cw_stilo_t *a_stilo);
cw_stilo_t	*stilo_array_el_get(cw_stilo_t *a_stilo, cw_sint64_t a_offset);
cw_stilte_t	stilo_array_el_set(cw_stilo_t *a_stilo, cw_stilo_t *a_el,
    cw_sint64_t a_offset);
cw_stilo_t	*stilo_array_get(cw_stilo_t *a_stilo);
cw_stilte_t	stilo_array_set(cw_stilo_t *a_stilo, cw_uint32_t a_offset,
    cw_stilo_t *a_arr, cw_uint32_t a_len);

/*
 * boolean.
 */
void		stilo_boolean_new(cw_stilo_t *a_stilo, cw_bool_t a_val);
cw_bool_t	stilo_boolean_get(cw_stilo_t *a_stilo);
void		stilo_boolean_set(cw_stilo_t *a_stilo, cw_bool_t a_val);

/*
 * condition.
 */
void		stilo_condition_new(cw_stilo_t *a_stilo, cw_stilt_t *a_stilt);

/*
 * dict.
 */
void		stilo_dict_new(cw_stilo_t *a_stilo, cw_stilt_t *a_stilt,
    cw_uint32_t a_dict_size);
cw_stilte_t	stilo_dict_copy(cw_stilo_t *a_to, cw_stilo_t *a_from,
    cw_stilt_t *a_stilt);
void		stilo_dict_def(cw_stilo_t *a_stilo, cw_stilt_t *a_stilt,
    cw_stilo_t *a_key, cw_stilo_t *a_val);
void		stilo_dict_undef(cw_stilo_t *a_stilo, cw_stilt_t *a_stilt, const
    cw_stilo_t *a_key);
cw_bool_t	stilo_dict_lookup(cw_stilo_t *a_stilo, cw_stilt_t *a_stilt,
    const cw_stilo_t *a_key, cw_stilo_t *r_stilo);
cw_uint32_t	stilo_dict_count(cw_stilo_t *a_stilo);
cw_bool_t	stilo_dict_iterate(cw_stilo_t *a_stilo, cw_stilt_t *a_stilt,
    cw_stilo_t *r_stilo);

/*
 * file.
 */
void		stilo_file_new(cw_stilo_t *a_stilo, cw_stilt_t *a_stilt);
void		stilo_file_fd_wrap(cw_stilo_t *a_stilo, cw_uint32_t a_fd);
void		stilo_file_interactive(cw_stilo_t *a_stilo, cw_stil_read_t
    *a_read, cw_stil_write_t *a_write, void *a_arg);
void		stilo_file_open(cw_stilo_t *a_stilo, cw_stilt_t *a_stilt, const
    cw_uint8_t *a_filename, cw_uint32_t a_nlen, const cw_uint8_t *a_flags,
    cw_uint32_t a_flen);
void		stilo_file_close(cw_stilo_t *a_stilo, cw_stilt_t *a_stilt);
cw_sint32_t	stilo_file_read(cw_stilo_t *a_stilo, cw_stilt_t *a_stilt,
    cw_uint32_t a_len, cw_uint8_t *r_str);
void		stilo_file_write(cw_stilo_t *a_stilo, cw_stilt_t *a_stilt, const
    cw_uint8_t *a_str, cw_uint32_t a_len);
void		stilo_file_output(cw_stilo_t *a_stilo, cw_stilt_t *a_stilt,
    const char *a_format, ...);
void		stilo_file_output_n(cw_stilo_t *a_stilo, cw_stilt_t *a_stilt,
    cw_uint32_t a_size, const char *a_format, ...);
void		stilo_file_truncate(cw_stilo_t *a_stilo, cw_stilt_t *a_stilt,
    cw_uint32_t a_length);
cw_sint64_t	stilo_file_position_get(cw_stilo_t *a_stilo, cw_stilt_t
    *a_stilt);
void		stilo_file_position_set(cw_stilo_t *a_stilo, cw_stilt_t
    *a_stilt, cw_sint64_t a_position);
cw_uint32_t	stilo_file_buffer_size_get(cw_stilo_t *a_stilo);
void		stilo_file_buffer_size_set(cw_stilo_t *a_stilo, cw_uint32_t
    a_size);
cw_sint64_t	stilo_file_buffer_count(cw_stilo_t *a_stilo);
void		stilo_file_buffer_flush(cw_stilo_t *a_stilo, cw_stilt_t
    *a_stilt);
void		stilo_file_buffer_reset(cw_stilo_t *a_stilo);
cw_bool_t	stilo_file_status(cw_stilo_t *a_stilo);
cw_sint64_t	stilo_file_mtime(cw_stilo_t *a_stilo, cw_stilt_t *a_stilt);

/*
 * hook.
 */
void		stilo_hook_new(cw_stilo_t *a_stilo, cw_stilt_t *a_stilt);
cw_stilte_t	stilo_hook_copy(cw_stilo_t *a_to, cw_stilo_t *a_from,
    cw_stilt_t *a_stilt);

/*
 * integer.
 */
#ifdef _LIBSTIL_DBG
#define	stilo_integer_new(a_stilo, a_val) do {				\
	memset((a_stilo), 0, sizeof(cw_stilo_t));			\
	(a_stilo)->type = STILOT_INTEGER;				\
	(a_stilo)->magic = _CW_STILO_MAGIC;				\
	(a_stilo)->o.integer.i = (a_val);				\
} while (0)
#else
#define	stilo_integer_new(a_stilo, a_val) do {				\
	memset((a_stilo), 0, sizeof(cw_stilo_t));			\
	(a_stilo)->type = STILOT_INTEGER;				\
	(a_stilo)->o.integer.i = (a_val);				\
} while (0)
#endif

#define		stilo_integer_get(a_stilo) (a_stilo)->o.integer.i
#define		stilo_integer_set(a_stilo, a_val) do {			\
	(a_stilo)->o.integer.i = (a_val);				\
} while (0)

/*
 * lock.
 */
void		stilo_lock_new(cw_stilo_t *a_stilo, cw_stilt_t *a_stilt);

/*
 * mark.
 */
void		stilo_mark_new(cw_stilo_t *a_stilo);

/*
 * name.
 */
void		stilo_name_new(cw_stilo_t *a_stilo, cw_stilt_t *a_stilt, const
    cw_uint8_t *a_name, cw_uint32_t a_len, cw_bool_t a_is_static);
const cw_uint8_t *stilo_name_str_get(cw_stilo_t *a_stilo);
cw_uint32_t	stilo_name_len_get(cw_stilo_t *a_stilo);
cw_uint32_t	stilo_name_hash(const void *a_key);
cw_bool_t	stilo_name_key_comp(const void *a_k1, const void *a_k2);

/*
 * null.
 */
#ifdef _LIBSTIL_DBG
#define	stilo_null_new(a_stilo) do {					\
	memset((a_stilo), 0, sizeof(cw_stilo_t));			\
	(a_stilo)->type = STILOT_NULL;					\
	(a_stilo)->magic = _CW_STILO_MAGIC;				\
} while (0)
#else
#define	stilo_null_new(a_stilo) do {					\
	memset((a_stilo), 0, sizeof(cw_stilo_t));			\
	(a_stilo)->type = STILOT_NULL;					\
} while (0)
#endif

/*
 * operator.
 */
void		stilo_operator_new(cw_stilo_t *a_stilo, cw_op_t *a_op);

/*
 * string.
 */
void		stilo_string_new(cw_stilo_t *a_stilo, cw_stilt_t *a_stilt,
    cw_uint32_t a_len);
cw_stilte_t	stilo_string_substring_new(cw_stilo_t *a_stilo, cw_stilo_t
    *a_string, cw_stilt_t *a_stilt, cw_uint32_t a_offset, cw_uint32_t a_len);
cw_stilte_t	stilo_string_copy(cw_stilo_t *a_to, cw_stilo_t *a_from,
    cw_stilt_t *a_stilt);
cw_uint32_t	stilo_string_len_get(cw_stilo_t *a_stilo);
cw_uint8_t	*stilo_string_el_get(cw_stilo_t *a_stilo, cw_sint64_t a_offset);
cw_stilte_t	stilo_string_el_set(cw_stilo_t *a_stilo, cw_uint8_t a_el,
    cw_sint64_t a_offset);
cw_uint8_t	*stilo_string_get(cw_stilo_t *a_stilo);
cw_stilte_t	stilo_string_set(cw_stilo_t *a_stilo, cw_uint32_t a_offset,
    const cw_uint8_t *a_str, cw_uint32_t a_len);
