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
#define _CW_STILOE_MAGIC	0x0fa6e798
#endif

typedef struct cw_stilo_s cw_stilo_t;
typedef struct cw_stiloe_s cw_stiloe_t;

/* Declared here to avoid circular header dependencies. */
typedef struct cw_stil_s cw_stil_t;
typedef struct cw_stila_s cw_stila_t;
typedef void cw_op_t(cw_stilo_t *);

/* Interpreter errors. */
typedef enum {
	STILO_THREADE_NONE,		/* No error. */
	STILO_THREADE_DSTACKUNDERFLOW,	/* No poppable dictionary on dstack. */
	STILO_THREADE_ESTACKOVERFLOW,	/* estack too deep. */
	STILO_THREADE_INTERRUPT,	/* Interrupt. */
	STILO_THREADE_INVALIDACCESS,	/* Permission error. */
	STILO_THREADE_INVALIDCONTEXT,	/* Bad thread context. */
	STILO_THREADE_INVALIDEXIT,	/* exit operator called outside loop. */
	STILO_THREADE_INVALIDFILEACCESS, /* Insufficient file permissions. */
	STILO_THREADE_IOERROR,		/* read()/write()/etc. error. */
	STILO_THREADE_LIMITCHECK,	/* Value outside legal range. */
	STILO_THREADE_RANGECHECK,	/* Out of bounds string or array use. */
	STILO_THREADE_STACKUNDERFLOW,	/* Not enough objects on ostack. */
	STILO_THREADE_SYNTAXERROR,	/* Scanner syntax error. */
	STILO_THREADE_TIMEOUT,		/* Timeout. */
	STILO_THREADE_TYPECHECK,	/* Incorrect argument type. */
	STILO_THREADE_UNDEFINED,	/* Object not found in dstack. */
	STILO_THREADE_UNDEFINEDFILENAME, /* Bad filename. */
	STILO_THREADE_UNDEFINEDRESULT,	/* Divide by 0. */
	STILO_THREADE_UNMATCHEDFINO,	/* No fino on ostack. */
	STILO_THREADE_UNMATCHEDMARK,	/* No mark on ostack. */
	STILO_THREADE_UNREGISTERED	/* Other non-enumerated error. */
#define	STILO_THREADE_LAST	STILO_THREADE_UNREGISTERED
} cw_stilo_threade_t;

typedef enum {
	STILOT_NO,
	STILOT_ARRAY,
	STILOT_BOOLEAN,
	STILOT_CONDITION,
	STILOT_DICT,
	STILOT_FILE,
	STILOT_FINO,
	STILOT_HOOK,
	STILOT_INTEGER,
	STILOT_MARK,
	STILOT_MUTEX,
	STILOT_NAME,
	STILOT_NULL,
	STILOT_OPERATOR,
	STILOT_STACK,
	STILOT_STRING,
	STILOT_THREAD
#define	STILOT_LAST	STILOT_THREAD
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
	 * Type.
	 */
	cw_stilot_t	type:5;
	/*
	 * If type is STILOT_OPERARTOR and fast_op is TRUE, this operator can be
	 * handled specially in stilo_thread_loop().
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
};

/*
 * All extended type objects contain a stiloe.  This provides a poor man's
 * inheritance.  Since stil's type system is static, this idiom is adequate.
 */
struct cw_stiloe_s {
#ifdef _LIBSTIL_DBG
	cw_uint32_t	magic;
#endif

	/*
	 * Linkage for GC.  All stiloe's are in a single ring, which the GC uses
	 * to implement a Baker's Treadmill collector.
	 */
	qr(cw_stiloe_t)	link;
	/*
	 * Object type.  We store this in stiloe's as well as stilo's, since
	 * various functions access stiloe's directly, rather than going through
	 * a referring stilo.
	 */
	cw_stilot_t	type:5;
	/*
	 * If TRUE, the string in the key is statically allocated, and should
	 * not be deallocated during destruction.
	 */
	cw_bool_t	name_static:1;
	/*
	 * The GC toggles this value at each collection in order to maintain
	 * state.
	 */
	cw_bool_t	color:1;
	/*
	 * TRUE if this object has been registered with the GC.  It is possible
	 * for an object to be reachable by the GC (on a stack, for instance),
	 * but not be registered yet.
	 */
	cw_bool_t	registered:1;
	/*
	 * If true, accesses to this object are locked.  This applies to arrays,
	 * dictionaries, files, and strings.
	 */
	cw_bool_t	locking:1;
	/*
	 * If TRUE, this stiloe is a reference to another stiloe.
	 */
	cw_bool_t	indirect:1;
};

cw_sint32_t	stilo_compare(cw_stilo_t *a_a, cw_stilo_t *a_b);

/*
 * The code for stilo_dup() is fragile in that it relies on the internal layout
 * of cw_stilo_t.  In order to fix this fragility, we would need to convert the
 * bit fields to a manually managed 32 bit variable with bits that represent
 * various flags.
 *
 * The order of operations is important in order to avoid a GC race.
 */
#ifdef _LIBSTIL_DBG
#define		stilo_dup(a_to, a_from) do {				\
	struct {							\
		cw_uint32_t	magic;					\
		cw_uint32_t	flags;					\
		cw_stiloi_t	data;					\
	} *x_to, *x_from;						\
	_cw_assert(sizeof(*x_to) == sizeof(cw_stilo_t));		\
									\
	x_to = (void *)(a_to);						\
	x_from = (void *)(a_from);					\
	_cw_assert((a_from)->magic == x_from->magic);			\
	_cw_assert((a_from)->o.integer.i == x_from->data);		\
	x_to->flags = 0;						\
	x_to->data = x_from->data;					\
	x_to->flags = x_from->flags;					\
	x_to->magic = x_from->magic;					\
} while (0)
#else
#define		stilo_dup(a_to, a_from) do {				\
	struct {							\
		cw_uint32_t	flags;					\
		cw_stiloi_t	data;					\
	} *x_to, *x_from;						\
									\
	x_to = (void *)(a_to);						\
	x_from = (void *)(a_from);					\
	x_to->flags = 0;						\
	x_to->data = x_from->data;					\
	x_to->flags = x_from->flags;					\
} while (0)
#endif

#define		stilo_type_get(a_stilo)	(a_stilo)->type
cw_stiloe_t	*stilo_stiloe_get(cw_stilo_t *a_stilo);
cw_bool_t	stilo_lcheck(cw_stilo_t *a_stilo);

#define		stilo_attrs_get(a_stilo) (a_stilo)->attrs
#define		stilo_attrs_set(a_stilo, a_attrs) (a_stilo)->attrs = (a_attrs)
