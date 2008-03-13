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

typedef struct cw_nxo_s cw_nxo_t;
typedef struct cw_nxoe_s cw_nxoe_t;

/* Declared here to avoid circular header dependencies. */
typedef struct cw_nx_s cw_nx_t;
typedef struct cw_nxa_s cw_nxa_t;
typedef void cw_op_t(cw_nxo_t *);

typedef enum {
	NXOT_NO,
	NXOT_ARRAY,
	NXOT_BOOLEAN,
#ifdef CW_THREADS
	NXOT_CONDITION,
#endif
	NXOT_DICT,
	NXOT_FILE,
	NXOT_FINO,
	NXOT_HOOK,
	NXOT_INTEGER,
	NXOT_MARK,
#ifdef CW_THREADS
	NXOT_MUTEX,
#endif
	NXOT_NAME,
	NXOT_NULL,
	NXOT_OPERATOR,
	NXOT_PMARK,
	NXOT_STACK,
	NXOT_STRING,
	NXOT_THREAD
#define	NXOT_LAST	NXOT_THREAD
}	cw_nxot_t;

/* Attribute. */
typedef enum {
	NXOA_LITERAL,
	NXOA_EXECUTABLE,
	NXOA_EVALUATABLE
}	cw_nxoa_t;

typedef cw_sint64_t cw_nxoi_t;

/*
 * Main object structure.
 */
struct cw_nxo_s {
#ifdef CW_DBG
	cw_uint32_t	magic;
#define CW_NXO_MAGIC	0x398754ba
#endif

	/*
	 * We can't use bit fields here, since we need explicit knowledge of
	 * structure layout to avoid GC races in nxo_dup() and nxo_p_new().
	 * Therefore, use a single 32 bit variable and do the bit manipulation
	 * manually.
	 *
	 * . : Unused.
	 *
	 * C : Op code.  If this is an operator, and NXN_ZERO < op code <=
	 *     NXN_LAST, op code corresponds to the name of this operator.  This
	 *     can be used to print the operator name or inline fast_op's in the
	 *     interpreter loop.
	 *
	 * B : Array bound.  TRUE if the bind operator has processed this array.
	 *     This is used to avoid infinite recursion in the bind operator
	 *     when binding recursive procedures.
	 *
	 * A : Attribute.  An nxo is LITERAL, EXECUTABLE, or EVALUATABLE.
	 *
	 * P : Fast op.  If type is NXOT_OPERATOR and fast op is TRUE, this
	 *     operator can be handled specially in nxo_thread_loop().
	 *
	 * T : Type.
	 *
	 * ........ .....CCC CCCCCCCB AAPTTTTT
	 *
	 */
	cw_uint32_t	flags;

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
#ifdef CW_DBG
	cw_uint32_t	magic;
#define CW_NXOE_MAGIC	0x0fa6e798
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
#ifdef CW_THREADS
	/*
	 * If true, accesses to this object are locked.  This applies to arrays,
	 * dictionaries, files, and strings.
	 */
	cw_bool_t	locking:1;
#endif
	/*
	 * If TRUE, this nxoe is a reference to another nxoe.
	 */
	cw_bool_t	indirect:1;
};

cw_sint32_t nxo_compare(cw_nxo_t *a_a, cw_nxo_t *a_b);
cw_nxoe_t *nxo_nxoe_get(cw_nxo_t *a_nxo);
#ifdef CW_THREADS
cw_bool_t nxo_lcheck(cw_nxo_t *a_nxo);
#endif

#ifndef CW_USE_INLINES
void	nxo_dup(cw_nxo_t *a_to, cw_nxo_t *a_from);
cw_nxot_t nxo_type_get(cw_nxo_t *a_nxo);
cw_nxoa_t nxo_attr_get(cw_nxo_t *a_nxo);
void	nxo_attr_set(cw_nxo_t *a_nxo, cw_nxoa_t a_attr);
void	nxo_p_new(cw_nxo_t *a_nxo, cw_nxot_t a_type);
#endif

#if (defined(CW_USE_INLINES) || defined(CW_NXO_C_))
CW_INLINE void
nxo_dup(cw_nxo_t *a_to, cw_nxo_t *a_from)
{
	cw_check_ptr(a_to);
	cw_dassert(a_to->magic == CW_NXO_MAGIC);

	cw_check_ptr(a_from);
	cw_dassert(a_from->magic == CW_NXO_MAGIC);

	/*
	 * The order of operations is important in order to avoid a GC race.
	 */
	a_to->flags = 0;
	a_to->o = a_from->o;
	a_to->flags = a_from->flags;
}

CW_INLINE cw_nxot_t
nxo_type_get(cw_nxo_t *a_nxo)
{
	cw_check_ptr(a_nxo);
	cw_dassert(a_nxo->magic == CW_NXO_MAGIC);

	return ((cw_nxot_t)(a_nxo->flags & 0x1f));
}

CW_INLINE cw_nxoa_t
nxo_attr_get(cw_nxo_t *a_nxo)
{
	cw_check_ptr(a_nxo);
	cw_dassert(a_nxo->magic == CW_NXO_MAGIC);

	return ((cw_nxoa_t)(((a_nxo)->flags >> 6) & 3));
}

CW_INLINE void
nxo_attr_set(cw_nxo_t *a_nxo, cw_nxoa_t a_attr)
{
	cw_check_ptr(a_nxo);
	cw_dassert(a_nxo->magic == CW_NXO_MAGIC);

	a_nxo->flags = (a_nxo->flags & 0xffffff3f) | (a_attr << 6);
}

/*
 * Private, but various object constructor macros need its definition.
 */
CW_INLINE void
nxo_p_new(cw_nxo_t *a_nxo, cw_nxot_t a_type)
{
	/*
	 * The order of operations is important in order to avoid a GC race.
	 */
	a_nxo->flags = 0;
#ifdef CW_DBG
	a_nxo->magic = CW_NXO_MAGIC;
#endif

	/*
	 * o.integer.i is assumed to be at least as big as all the other fields
	 * in the union.
	 */
	a_nxo->o.integer.i = 0;
	a_nxo->flags = a_type;
}
#endif	/* (defined(CW_USE_INLINES) || defined(CW_NXO_C_)) */
