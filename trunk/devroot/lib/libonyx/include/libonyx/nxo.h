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

#ifdef _LIBONYX_DBG
#define _CW_NXO_MAGIC	0x398754ba
#define _CW_NXOE_MAGIC	0x0fa6e798
#endif

typedef struct cw_nxo_s cw_nxo_t;
typedef struct cw_nxoe_s cw_nxoe_t;

/* Declared here to avoid circular header dependencies. */
typedef struct cw_nx_s cw_nx_t;
typedef struct cw_nxa_s cw_nxa_t;
typedef void cw_op_t(cw_nxo_t *);

/* Interpreter errors. */
typedef enum {
	NXO_THREADE_NONE,		/* No error. */
	NXO_THREADE_DSTACKUNDERFLOW,	/* No poppable dictionary on dstack. */
	NXO_THREADE_ESTACKOVERFLOW,	/* estack too deep. */
	NXO_THREADE_INTERRUPT,		/* Interrupt. */
	NXO_THREADE_INVALIDACCESS,	/* Permission error. */
	NXO_THREADE_INVALIDCONTEXT,	/* Bad thread context. */
	NXO_THREADE_INVALIDEXIT,	/* exit operator called outside loop. */
	NXO_THREADE_INVALIDFILEACCESS,	/* Insufficient file permissions. */
	NXO_THREADE_IOERROR,		/* read()/write()/etc. error. */
	NXO_THREADE_LIMITCHECK,		/* Value outside legal range. */
	NXO_THREADE_RANGECHECK,		/* Out of bounds string or array use. */
	NXO_THREADE_STACKUNDERFLOW,	/* Not enough objects on ostack. */
	NXO_THREADE_SYNTAXERROR,	/* Scanner syntax error. */
	NXO_THREADE_TIMEOUT,		/* Timeout. */
	NXO_THREADE_TYPECHECK,		/* Incorrect argument type. */
	NXO_THREADE_UNDEFINED,		/* Object not found in dstack. */
	NXO_THREADE_UNDEFINEDFILENAME,	/* Bad filename. */
	NXO_THREADE_UNDEFINEDRESULT,	/* Divide by 0. */
	NXO_THREADE_UNMATCHEDFINO,	/* No fino on ostack. */
	NXO_THREADE_UNMATCHEDMARK,	/* No mark on ostack. */
	NXO_THREADE_UNREGISTERED	/* Other non-enumerated error. */
#define	NXO_THREADE_LAST	NXO_THREADE_UNREGISTERED
} cw_nxo_threade_t;

typedef enum {
	NXOT_NO,
	NXOT_ARRAY,
	NXOT_BOOLEAN,
	NXOT_CONDITION,
	NXOT_DICT,
	NXOT_FILE,
	NXOT_FINO,
	NXOT_HOOK,
	NXOT_INTEGER,
	NXOT_MARK,
	NXOT_MUTEX,
	NXOT_NAME,
	NXOT_NULL,
	NXOT_OPERATOR,
	NXOT_STACK,
	NXOT_STRING,
	NXOT_THREAD
#define	NXOT_LAST	NXOT_THREAD
}	cw_nxot_t;

/* Attributes. */
typedef enum {
	NXOA_LITERAL,
	NXOA_EXECUTABLE
}	cw_nxoa_t;

/*
 * The nx language was designed to use 64 bit signed integers.  However, 64
 * bit processors nxl aren't the defacto standard, and nx performance can
 * suffer considerably due to the compiler emulating 64 bit integers.  It is
 * possible to use nx with signed 32 bit integers, but this introduces a
 * number of unpleasant limitations.  For example, files are limited to 2 GB in
 * size.
 */
#define	_CW_NXOI_SIZEOF	8
#if (_CW_NXOI_SIZEOF == 8)
typedef cw_sint64_t cw_nxoi_t;
#elif (_CW_NXOI_SIZEOF == 4)
typedef cw_sint32_t cw_nxoi_t;
#else
#error "Unsupported nxoi size"
#endif

/*
 * Main object structure.
 */
struct cw_nxo_s {
#ifdef _LIBONYX_DBG
	cw_uint32_t	magic;
#endif

	/*
	 * Type.
	 */
	cw_nxot_t	type:5;
	/*
	 * If type is NXOT_OPERARTOR and fast_op is TRUE, this operator can be
	 * handled specially in nxo_thread_loop().
	 */
	cw_bool_t	fast_op:1;
	/*
	 * If this is an operator, and op_code <= NXN_LAST, op_code
	 * corresponds to the name of this operator.  This can be used to print
	 * the operator name or inline fast_op's in the interpreter loop.
	 */
	cw_nxn_t	op_code:10;
	/*
	 * Attributes.  A nxo is either LITERAL or EXECUTABLE.
	 */
	cw_nxoa_t	attrs:1;
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
			cw_nxoi_t	i;
		}	integer;
		struct {
			cw_op_t		*f;
		}	operator;
		cw_nxoe_t	*nxoe;
	}	o;
};

/*
 * All extended type objects contain a nxoe.  This provides a poor man's
 * inheritance.  Since onyx's type system is non-extenible, this idiom is
 * adequate.
 */
struct cw_nxoe_s {
#ifdef _LIBONYX_DBG
	cw_uint32_t	magic;
#endif

	/*
	 * Linkage for GC.  All nxoe's are in a single ring, which the GC uses
	 * to implement a Baker's Treadmill collector.
	 */
	qr(cw_nxoe_t)	link;
	/*
	 * Object type.  We store this in nxoe's as well as nxo's, since various
	 * functions access nxoe's directly, rather than going through a
	 * referring nxo.
	 */
	cw_nxot_t	type:5;
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
	 * If TRUE, this nxoe is a reference to another nxoe.
	 */
	cw_bool_t	indirect:1;
};

cw_sint32_t nxo_compare(cw_nxo_t *a_a, cw_nxo_t *a_b);

/*
 * The code for nxo_dup() is fragile in that it relies on the internal layout
 * of cw_nxo_t.  In order to fix this fragility, we would need to convert the
 * bit fields to a manually managed 32 bit variable with bits that represent
 * various flags.
 *
 * The order of operations is important in order to avoid a GC race.
 */
#ifdef _LIBONYX_DBG
#define		nxo_dup(a_to, a_from) do {				\
	struct {							\
		cw_uint32_t	magic;					\
		cw_uint32_t	flags;					\
		cw_nxoi_t	data;					\
	} *x_to, *x_from;						\
	_cw_assert(sizeof(*x_to) == sizeof(cw_nxo_t));			\
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
#define		nxo_dup(a_to, a_from) do {				\
	struct {							\
		cw_uint32_t	flags;					\
		cw_nxoi_t	data;					\
	} *x_to, *x_from;						\
									\
	x_to = (void *)(a_to);						\
	x_from = (void *)(a_from);					\
	x_to->flags = 0;						\
	x_to->data = x_from->data;					\
	x_to->flags = x_from->flags;					\
} while (0)
#endif

#define	nxo_type_get(a_nxo)	(a_nxo)->type
cw_nxoe_t *nxo_nxoe_get(cw_nxo_t *a_nxo);
cw_bool_t nxo_lcheck(cw_nxo_t *a_nxo);

#define	nxo_attrs_get(a_nxo) (a_nxo)->attrs
#define	nxo_attrs_set(a_nxo, a_attrs) (a_nxo)->attrs = (a_attrs)
