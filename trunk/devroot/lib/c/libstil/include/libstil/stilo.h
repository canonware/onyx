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

/* Declared here to avoid circular header dependencies. */
typedef struct cw_stil_s cw_stil_t;
typedef struct cw_stila_s cw_stila_t;
typedef struct cw_stilt_s cw_stilt_t;
typedef struct cw_stilsc_s cw_stilsc_t;
typedef void cw_op_t(cw_stilt_t *);

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
	STILTE_UNDEFINEDRESULT,		/* Divide by 0. */
	STILTE_UNMATCHEDMARK,		/* No mark on ostack. */
	STILTE_UNREGISTERED		/* Other non-enumerated error. */
#define	STILTE_LAST	STILTE_UNREGISTERED
} cw_stilte_t;

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
 * The stil language was designed to use 64 bit signed integers.  However, 64
 * bit processors still aren't the defacto standard, and stil performance can
 * suffer considerably due to the compiler emulating 64 bit integers.  It is
 * possible to use stil with signed 32 bit integers, but this introduces a
 * number of unpleasant limitations.  For example, files are limited to 2 GB in
 * size.
 */
#define	_CW_STILOI_SIZEOF	8
#if (_CW_STILOI_SIZEOF == 8)
typedef cw_sint64_t cw_stiloi_t;
#elif (_CW_STILOI_SIZEOF == 4)
typedef cw_sint32_t cw_stiloi_t;
#else
#error "Unsupported stiloi size"
#endif

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
			cw_stiloi_t	i;
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
} while (0)
	
#define		stilo_type_get(a_stilo)	(a_stilo)->type
cw_stiloe_t	*stilo_stiloe_get(cw_stilo_t *a_stilo);
cw_bool_t	stilo_lcheck(cw_stilo_t *a_stilo);

#define		stilo_attrs_get(a_stilo) (a_stilo)->attrs
#define		stilo_attrs_set(a_stilo, a_attrs) (a_stilo)->attrs = (a_attrs)

cw_stilte_t	stilo_print(cw_stilo_t *a_stilo, cw_stilo_t *a_file, cw_uint32_t
    a_depth, cw_bool_t a_newline);
