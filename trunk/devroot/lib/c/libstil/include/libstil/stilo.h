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

#ifdef _LIBSTIL_DBG
#define _CW_STILO_MAGIC		0x398754ba
#endif

typedef struct cw_stilo_s cw_stilo_t;
typedef struct cw_stiloe_s cw_stiloe_t;
typedef struct cw_stiloe_dicto_s cw_stiloe_dicto_t;

/* Declared here to avoid a circular header dependency. */
typedef struct cw_stil_s cw_stil_t;
typedef struct cw_stila_s cw_stila_t;
typedef struct cw_stilt_s cw_stilt_t;
typedef struct cw_stilsc_s cw_stilsc_t;
typedef struct cw_stiloe_dict_s cw_stiloe_dict_t;

/* Interpreter errors. */
typedef enum {
	STILTE_NONE,			/* No error. */
	STILTE_DSTACKUNDERFLOW,		/* No poppable dictionary on dstack. */
	STILTE_ESTACKOVERFLOW,		/* estack too deep. */
	STILTE_INTERRUPT,		/* Interrupt. */
	STILTE_INVALIDACCESS,		/* Permission error. */
	STILTE_INVALIDCONTEXT,		/* Bad thread context. */
	STILTE_INVALIDEXIT,		/* exit operator called outside loop. */
	STILTE_INVALIDFILEACCESS,	/* Insufficient file permissions. */
	STILTE_IOERROR,			/* read()/write()/etc. error. */
	STILTE_LIMITCHECK,		/* Value outside legal range. */
	STILTE_RANGECHECK,		/* Out of bounds string or array use. */
	STILTE_STACKUNDERFLOW,		/* Not enough objects on ostack. */
	STILTE_SYNTAXERROR,		/* Scanner syntax error. */
	STILTE_TIMEOUT,			/* Timeout. */
	STILTE_TYPECHECK,		/* Incorrect argument type. */
	STILTE_UNDEFINED,		/* Object not found in dstack. */
	STILTE_UNDEFINEDFILENAME,	/* Bad filename. */
	STILTE_UNDEFINEDRESULT,
	STILTE_UNMATCHEDMARK,		/* No mark on ostack. */
	STILTE_UNREGISTERED		/* Other non-enumerated error. */
#define	STILTE_LAST	STILTE_UNREGISTERED
} cw_stilte_t;

/* Defined here to resolve circular dependencies. */
typedef void cw_op_t(cw_stilt_t *);

typedef enum {
	STILOT_NO,
	STILOT_ARRAY,
	STILOT_BOOLEAN,
	STILOT_CONDITION,
	STILOT_DICT,
	STILOT_FILE,
	STILOT_HOOK,
	STILOT_INTEGER,
	STILOT_MARK,
	STILOT_MUTEX,
	STILOT_NAME,
	STILOT_NULL,
	STILOT_OPERATOR,
	STILOT_STRING
#define	STILOT_LAST	STILOT_STRING
}	cw_stilot_t;

/* Attributes. */
typedef enum {
	STILOA_LITERAL,
	STILOA_EXECUTABLE
}	cw_stiloa_t;

/*
 * Main object structure.
 */
struct cw_stilo_s {
#ifdef _LIBSTIL_DBG
	cw_uint32_t	magic;
#endif

	/*
	 * This should be before the bit fields so that in stilo_dup(), where
	 * memcpy() is used, the stiloe pointer will be valid before the type is
	 * set.  That way, the GC won't go chasing bad pointers.
	 */
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

	/*
	 * Type.
	 */
	cw_stilot_t	type:4;
	/*
	 * If type is STILOT_OPERARTOR and fast_op is TRUE, this operator can be
	 * handled specially in stilt_loop().
	 */
	cw_bool_t	fast_op:1;
	/*
	 * If this is an operator, and op_code <= STILN_LAST, op_code
	 * corresponds to the name of this operator.  This can be used to print
	 * the operator name or inline fast_op's in the interpreter loop.
	 */
	cw_stiln_t	op_code:10;
	/*
	 * Attributes.  A stilo is either LITERAL or EXECUTABLE.
	 */
	cw_stiloa_t	attrs:1;
	/*
	 * TRUE if the bind operator has processed this array.  This is used to
	 * avoid infinite recursion in the bind operator when binding recursive
	 * procedures.
	 */
	cw_bool_t	array_bound:1;
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
};

cw_sint32_t	stilo_compare(cw_stilo_t *a_a, cw_stilo_t *a_b);

/*
 * This code is only GC-safe as long as o.stiloe is memcpy()ed at or before the
 * time that the type is set.  This is probably okay with any compiler/OS
 * combination available, but the memcpy() interface does not explicitly
 * guarantee that bytes are copied in ascending order.
 *
 * To make this code safe even if the memcpy() does strange things, the memcpy()
 * call needs to be surrounded by thd_crit_{enter,leave}().
 */
#define		stilo_dup(a_to, a_from) do {				\
	/* Copy. */							\
	memcpy((a_to), (a_from), sizeof(cw_stilo_t));			\
									\
	/* Reset debug flags on new copy. */				\
	(a_to)->breakpoint = FALSE;					\
	(a_to)->watchpoint = FALSE;					\
} while (0)
	
#define		stilo_type_get(a_stilo)	(a_stilo)->type
cw_stiloe_t	*stilo_stiloe_get(cw_stilo_t *a_stilo);
cw_bool_t	stilo_lcheck(cw_stilo_t *a_stilo);

#define		stilo_attrs_get(a_stilo) (a_stilo)->attrs
#define		stilo_attrs_set(a_stilo, a_attrs) (a_stilo)->attrs = (a_attrs)

cw_stilte_t	stilo_print(cw_stilo_t *a_stilo, cw_stilo_t *a_file, cw_uint32_t
    a_depth, cw_bool_t a_newline);

/*
 * no.
 */
#ifdef _LIBSTIL_DBG
#define	stilo_no_new(a_stilo) do {					\
	memset((a_stilo), 0, sizeof(cw_stilo_t));			\
	(a_stilo)->magic = _CW_STILO_MAGIC;				\
	(a_stilo)->type = STILOT_NO;					\
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
void		stilo_array_new(cw_stilo_t *a_stilo, cw_stil_t *a_stil,
    cw_bool_t a_locking, cw_uint32_t a_len);
void		stilo_array_subarray_new(cw_stilo_t *a_stilo, cw_stilo_t
    *a_array, cw_stil_t *a_stil, cw_uint32_t a_offset, cw_uint32_t a_len);
void		stilo_array_copy(cw_stilo_t *a_to, cw_stilo_t *a_from);
cw_uint32_t	stilo_array_len_get(cw_stilo_t *a_stilo);
void		stilo_array_el_get(cw_stilo_t *a_stilo, cw_sint64_t a_offset,
    cw_stilo_t *r_el);
void		stilo_array_el_set(cw_stilo_t *a_stilo, cw_stilo_t *a_el,
    cw_sint64_t a_offset);

/*
 * boolean.
 */
void		stilo_boolean_new(cw_stilo_t *a_stilo, cw_bool_t a_val);
cw_bool_t	stilo_boolean_get(cw_stilo_t *a_stilo);
void		stilo_boolean_set(cw_stilo_t *a_stilo, cw_bool_t a_val);

/*
 * condition.
 */
void		stilo_condition_new(cw_stilo_t *a_stilo, cw_stil_t *a_stil);
void		stilo_condition_signal(cw_stilo_t *a_stilo);
void		stilo_condition_broadcast(cw_stilo_t *a_stilo);
void		stilo_condition_wait(cw_stilo_t *a_stilo, cw_stilo_t *a_mutex);
cw_bool_t	stilo_condition_timedwait(cw_stilo_t *a_stilo, cw_stilo_t
    *a_mutex, const struct timespec *a_timeout);

/*
 * dict.
 */
void		stilo_dict_new(cw_stilo_t *a_stilo, cw_stil_t *a_stil, cw_bool_t
    a_locking, cw_uint32_t a_dict_size);
void		stilo_dict_copy(cw_stilo_t *a_to, cw_stilo_t *a_from,
    cw_stil_t *a_stil, cw_bool_t a_locking);
void		stilo_dict_def(cw_stilo_t *a_stilo, cw_stil_t *a_stil,
    cw_stilo_t *a_key, cw_stilo_t *a_val);
void		stilo_dict_undef(cw_stilo_t *a_stilo, cw_stil_t *a_stil, const
    cw_stilo_t *a_key);
cw_bool_t	stilo_dict_lookup(cw_stilo_t *a_stilo, const cw_stilo_t *a_key,
    cw_stilo_t *r_stilo);
cw_uint32_t	stilo_dict_count(cw_stilo_t *a_stilo);
void		stilo_dict_iterate(cw_stilo_t *a_stilo, cw_stilo_t *r_stilo);

/*
 * file.
 */
typedef cw_sint32_t cw_stilo_file_read_t (void *a_arg, cw_stilo_t *a_file,
    cw_uint32_t a_len, cw_uint8_t *r_str);
typedef cw_bool_t cw_stilo_file_write_t (void *a_arg, cw_stilo_t *a_file, const
    cw_uint8_t *a_str, cw_uint32_t a_len);

void		stilo_file_new(cw_stilo_t *a_stilo, cw_stil_t *a_stil, cw_bool_t
    a_locking);
void		stilo_file_fd_wrap(cw_stilo_t *a_stilo, cw_uint32_t a_fd);
void		stilo_file_interactive(cw_stilo_t *a_stilo, cw_stilo_file_read_t
    *a_read, cw_stilo_file_write_t *a_write, void *a_arg);
cw_stilte_t	stilo_file_open(cw_stilo_t *a_stilo, const cw_uint8_t
    *a_filename, cw_uint32_t a_nlen, const cw_uint8_t *a_flags, cw_uint32_t
    a_flen);
cw_stilte_t	stilo_file_close(cw_stilo_t *a_stilo);
cw_sint32_t	stilo_file_read(cw_stilo_t *a_stilo, cw_uint32_t a_len,
    cw_uint8_t *r_str);
cw_stilte_t	stilo_file_readline(cw_stilo_t *a_stilo, cw_stil_t *a_stil,
    cw_bool_t a_locking, cw_stilo_t *r_string, cw_bool_t *r_eof);
cw_stilte_t	stilo_file_write(cw_stilo_t *a_stilo, const cw_uint8_t *a_str,
    cw_uint32_t a_len);
cw_stilte_t	stilo_file_output(cw_stilo_t *a_stilo, const char *a_format,
    ...);
cw_stilte_t	stilo_file_output_n(cw_stilo_t *a_stilo, cw_uint32_t a_size,
    const char *a_format, ...);
cw_stilte_t	stilo_file_truncate(cw_stilo_t *a_stilo, cw_uint32_t a_length);
cw_sint64_t	stilo_file_position_get(cw_stilo_t *a_stilo);
cw_stilte_t	stilo_file_position_set(cw_stilo_t *a_stilo, cw_sint64_t
    a_position);
cw_uint32_t	stilo_file_buffer_size_get(cw_stilo_t *a_stilo);
void		stilo_file_buffer_size_set(cw_stilo_t *a_stilo, cw_uint32_t
    a_size);
cw_sint64_t	stilo_file_buffer_count(cw_stilo_t *a_stilo);
cw_stilte_t	stilo_file_buffer_flush(cw_stilo_t *a_stilo);
void		stilo_file_buffer_reset(cw_stilo_t *a_stilo);
cw_bool_t	stilo_file_status(cw_stilo_t *a_stilo);
cw_sint64_t	stilo_file_mtime(cw_stilo_t *a_stilo);

/*
 * hook.
 */
typedef cw_stilte_t cw_stilo_hook_exec_t (void *a_data, cw_stilt_t *a_stilt);
typedef cw_stiloe_t *cw_stilo_hook_ref_iter_t (void *a_data, cw_bool_t
    a_reset);
typedef void cw_stilo_hook_delete_t (void *a_data, cw_stil_t *a_stil);

void		stilo_hook_new(cw_stilo_t *a_stilo, cw_stil_t *a_stil, void
    *a_data, cw_stilo_hook_exec_t *a_exec_f, cw_stilo_hook_ref_iter_t
    *a_ref_iter_f, cw_stilo_hook_delete_t *a_delete_f);
void		*stilo_hook_get(cw_stilo_t *a_stilo);
cw_stilte_t	stilo_hook_exec(cw_stilo_t *a_stilo, cw_stilt_t *a_stilt);

/*
 * integer.
 */
#ifdef _LIBSTIL_DBG
#define	stilo_integer_new(a_stilo, a_val) do {				\
	memset((a_stilo), 0, sizeof(cw_stilo_t));			\
	(a_stilo)->magic = _CW_STILO_MAGIC;				\
	(a_stilo)->o.integer.i = (a_val);				\
	(a_stilo)->type = STILOT_INTEGER;				\
} while (0)
#else
#define	stilo_integer_new(a_stilo, a_val) do {				\
	memset((a_stilo), 0, sizeof(cw_stilo_t));			\
	(a_stilo)->o.integer.i = (a_val);				\
	(a_stilo)->type = STILOT_INTEGER;				\
} while (0)
#endif

#define		stilo_integer_get(a_stilo) (a_stilo)->o.integer.i
#define		stilo_integer_set(a_stilo, a_val) do {			\
	(a_stilo)->o.integer.i = (a_val);				\
} while (0)

/*
 * mark.
 */
void		stilo_mark_new(cw_stilo_t *a_stilo);

/*
 * mutex.
 */
void		stilo_mutex_new(cw_stilo_t *a_stilo, cw_stil_t *a_stil);
void		stilo_mutex_lock(cw_stilo_t *a_stilo);
cw_bool_t	stilo_mutex_trylock(cw_stilo_t *a_stilo);
void		stilo_mutex_unlock(cw_stilo_t *a_stilo);

/*
 * name.
 */
void		stilo_name_new(cw_stilo_t *a_stilo, cw_stil_t *a_stil, const
    cw_uint8_t *a_str, cw_uint32_t a_len, cw_bool_t a_is_static);
const cw_uint8_t *stilo_name_str_get(cw_stilo_t *a_stilo);
cw_uint32_t	stilo_name_len_get(cw_stilo_t *a_stilo);

/*
 * null.
 */
#ifdef _LIBSTIL_DBG
#define	stilo_null_new(a_stilo) do {					\
	memset((a_stilo), 0, sizeof(cw_stilo_t));			\
	(a_stilo)->magic = _CW_STILO_MAGIC;				\
	(a_stilo)->type = STILOT_NULL;					\
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
void		stilo_operator_new(cw_stilo_t *a_stilo, cw_op_t *a_op,
    cw_stiln_t a_stiln);
#define		stilo_operator_f(a_stilo) (a_stilo)->o.operator.f

/*
 * string.
 */
void		stilo_string_new(cw_stilo_t *a_stilo, cw_stil_t *a_stil,
    cw_bool_t a_locking, cw_uint32_t a_len);
void		stilo_string_substring_new(cw_stilo_t *a_stilo, cw_stilo_t
    *a_string, cw_stil_t *a_stil, cw_uint32_t a_offset, cw_uint32_t a_len);
void		stilo_string_copy(cw_stilo_t *a_to, cw_stilo_t *a_from);
cw_uint32_t	stilo_string_len_get(cw_stilo_t *a_stilo);
void		stilo_string_el_get(cw_stilo_t *a_stilo, cw_sint64_t a_offset,
    cw_uint8_t *r_el);
void		stilo_string_el_set(cw_stilo_t *a_stilo, cw_uint8_t a_el,
    cw_sint64_t a_offset);
void		stilo_string_lock(cw_stilo_t *a_stilo);
void		stilo_string_unlock(cw_stilo_t *a_stilo);
cw_uint8_t	*stilo_string_get(cw_stilo_t *a_stilo);
void		stilo_string_set(cw_stilo_t *a_stilo, cw_uint32_t a_offset,
    const cw_uint8_t *a_str, cw_uint32_t a_len);
