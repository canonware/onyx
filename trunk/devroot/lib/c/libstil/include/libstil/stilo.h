/* -*- mode: c ; c-file-style: "canonware-c-style" -*-
 ****************************************************************************
 *
 * <Copyright = "jasone">
 * <License>
 *
 ****************************************************************************
 *
 * Version: <Version>
 *
 ****************************************************************************/

typedef struct cw_stilo_s cw_stilo_t;

/* Defined here to resolve circular dependencies. */
typedef struct cw_stiloe_array_s cw_stiloe_array_t;
typedef struct cw_stiloe_condition_s cw_stiloe_condition_t;
typedef struct cw_stiloe_dict_s cw_stiloe_dict_t;
typedef struct cw_stiloe_lock_s cw_stiloe_lock_t;
typedef struct cw_stiloe_mstate_s cw_stiloe_mstate_t;
typedef struct cw_stiloe_number_s cw_stiloe_number_t;
typedef struct cw_stiloe_packedarray_s cw_stiloe_packedarray_t;
typedef struct cw_stiloe_string_s cw_stiloe_string_t;
typedef struct cw_stiln_s cw_stiln_t;
typedef struct cw_stilt_s cw_stilt_t;

typedef enum {
	_CW_STILOT_NOTYPE = 0,
	_CW_STILOT_ARRAYTYPE = 1,
	_CW_STILOT_BOOLEANTYPE = 2,
	_CW_STILOT_CONDITIONTYPE = 3,
	_CW_STILOT_DICTTYPE = 4,
	_CW_STILOT_FILETYPE = 5,
	_CW_STILOT_LOCKTYPE = 6,
	_CW_STILOT_MARKTYPE = 7,
	_CW_STILOT_MSTATETYPE = 8,
	_CW_STILOT_NAMETYPE = 9,
	_CW_STILOT_NULLTYPE = 10,
	_CW_STILOT_NUMBERTYPE = 11,
	_CW_STILOT_OPERATORTYPE = 12,
	_CW_STILOT_PACKEDARRAYTYPE = 13,
	_CW_STILOT_STRINGTYPE = 14
} cw_stilot_t;

/*
 * Main object structure.
 */
struct cw_stilo_s {
#if (defined(_LIBSTIL_DBG) || defined(_LIBSTIL_DEBUG))
	cw_uint32_t magic;
#endif

	cw_stilot_t type:4;
	/*
	 * If non-zero, this is an extended number or mstate.  Both number
	 * and mstate objects can switch between simple and extended
	 * representations.
	 */
	cw_bool_t extended:1;
	/*
	 * Name objects use this bit to indicate if a name is an indirect
	 * reference. Each stilt maintains a cache of stiln pointers, each
	 * holding a single reference to the names hash in stil.  If this is
	 * an indirect reference, the unreferencing operation should
	 * actually be done with the stilt's stiln cache.  Note that this is
	 * the (very) common case.  Use the stiln pointer to get the actual
	 * name "value" for the unreferencing operation.  This is safe,
	 * because this stilt is guaranteed to be holding a reference to the
	 * stiln.
	 */
	cw_bool_t indirect_name:1;
	/*
	 * If TRUE, there is a breakpoint set on this object.  In general,
	 * this field is not looked at unless the interpreter has been put
	 * into debugging mode.
	 */
	cw_bool_t breakpoint:1;
	/*
	 * If TRUE, there is a watchpoint set on this object.  In general,
	 * this field is not looked at unless the interpreter has been put
	 * into debugging mode. Note that setting a watchpoint on a
	 * reference to an extended type only detects changes that are made
	 * via that particular reference to the extension.
	 */
	cw_bool_t watchpoint:1;
	/*
	 * Reference count.  We use only one bit:
	 *
	 * 1 == One reference. 0 == Overflow (GC knows about the stiloe).
	 */
	cw_uint32_t ref_count:1;
	/* Reference to a local or global object? */
	cw_bool_t global:1;
	/*
	 * This bit is used to protect modifications to a stiloe pointer.
	 * Since a thread can be suspended at any time, it is critical to
	 * mark the pointer invalid while modifying it so that the collector
	 * knows not to try using a possibly corrupt pointer.
	 */
	cw_bool_t valid:1;

	union {
		struct {
			cw_stiloe_array_t *stiloe;
		}       array;
		struct {
			cw_bool_t val;
		}       boolean;
		struct {
			cw_stiloe_condition_t *stiloe;
		}       condition;
		struct {
			cw_stiloe_dict_t *stiloe;
		}       dict;
		struct {
			cw_sint32_t fd;
		}       file;
		struct {
			cw_stiloe_lock_t *stiloe;
		}       lock;
		struct {
			cw_uint32_t garbage;
		}       mark;
		struct {
			/*
			 * If not extended, the mstate is:
			 *
			 * accuracy : 32 point    : 0 base     : 10
			 *
			 * Otherwise, the mstate is in ext.
			 */
			cw_stiloe_mstate_t *stiloe;
		}       mstate;
		struct {
			cw_stiln_t *stiln;
		}       name;
		struct {
			cw_uint32_t garbage;
		}       null;
		struct {
			/*
			 * If not (flags.extended), this number is
			 * representable as a 32 bit signed integer.
			 * Otherwise the ext contains the value.
			 */
			union {
				cw_stiloe_number_t *stiloe;
				cw_sint32_t s32;
			}       val;
		}       number;
		struct {
			void    (*f) (cw_stilt_t *);
		}       operator;
		struct {
			cw_stiloe_packedarray_t *stiloe;
		}       packedarray;
		struct {
			cw_stiloe_string_t *stiloe;
		}       string;
	}       o;
};

void    stilo_new(cw_stilo_t *a_stilo);

void    stilo_delete(cw_stilo_t *a_stilo);

cw_stilot_t stilo_type(cw_stilo_t *a_stilo);

void    stilo_copy(cw_stilo_t *a_to, cw_stilo_t *a_from);

cw_bool_t stilo_cast(cw_stilo_t *a_stilo, cw_stilot_t a_stilot);
