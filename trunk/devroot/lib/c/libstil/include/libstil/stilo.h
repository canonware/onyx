/****************************************************************************
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
typedef struct cw_stiloe_s cw_stiloe_t;
typedef struct cw_stiloec_s cw_stiloec_t;
typedef struct cw_stiloe_array_s cw_stiloe_array_t;
typedef struct cw_stiloe_condition_s cw_stiloe_condition_t;
typedef struct cw_stiloe_dict_s cw_stiloe_dict_t;
typedef struct cw_stiloe_hook_s cw_stiloe_hook_t;
typedef struct cw_stiloe_lock_s cw_stiloe_lock_t;
typedef struct cw_stiloe_mstate_s cw_stiloe_mstate_t;
typedef struct cw_stiloe_number_s cw_stiloe_number_t;
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
	_CW_STILOT_HOOKTYPE = 6,
	_CW_STILOT_LOCKTYPE = 7,
	_CW_STILOT_MARKTYPE = 8,
	_CW_STILOT_MSTATETYPE = 9,
	_CW_STILOT_NAMETYPE = 10,
	_CW_STILOT_NULLTYPE = 11,
	_CW_STILOT_NUMBERTYPE = 12,
	_CW_STILOT_OPERATORTYPE = 13,
	_CW_STILOT_STRINGTYPE = 14
}	cw_stilot_t;

/*
 * Main object structure.
 */
struct cw_stilo_s {
#if (defined(_LIBSTIL_DBG) || defined(_LIBSTIL_DEBUG))
	cw_uint32_t	magic;
#endif

	/*
	 * Type.
	 *
	 * The type _CW_STILOT_NOTYPE can be used to protect modifications to a
	 * stiloe pointer.  Since a thread can be suspended at any time, it is
	 * critical to mark the pointer invalid while modifying it so that the
	 * collector knows not to try using a possibly corrupt pointer.
	 */
	cw_stilot_t	type:4;
	/*
	 * Every object is either literal (FALSE) or executable (TRUE).
	 */
	cw_bool_t	executable:1;
	/*
	 * A literal object if TRUE.
	 */
	cw_bool_t	literal:1;
	/*
	 * If TRUE, this is an extended object.  This has extra significance if
	 * the object is a number or mstate.  Both number and mstate objects can
	 * switch between simple and extended representations.
	 *
	 * ---
	 * For a mstate object, if not extended, the mstate is:
	 *
	 * accuracy : 32
	 * point    : 0
	 * base     : 10
	 *
	 * Otherwise, the mstate is in o.stiloe.
	 *
	 * ---
	 * For a number object, if not extended, this number is representable as
	 * a 32 bit signed integer.  Otherwise o.stiloe contains the value.
	 */
	cw_bool_t	extended:1;
	/*
	 * Name objects use this bit to indicate if a name is an indirect
	 * reference.  Each stilt maintains a cache of stiln pointers, each
	 * holding a single reference to the names hash in stil.  If this is an
	 * indirect reference, the unreferencing operation should actually be
	 * done with the stilt's stiln cache.  Note that this is the (very)
	 * common case.  Only global dictionaries have direct references.  Use
	 * the stiln pointer to get the actual name "value" for the
	 * unreferencing operation.  This is safe, because this stilt is
	 * guaranteed to be holding a reference to the stiln.
	 */
	cw_bool_t	indirect_name:1;
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
	/*
	 * Reference count.  We use only one bit:
	 *
	 * 0 == One reference. 1 == Overflow (GC knows about the stiloe).
	 */
	cw_uint32_t	ref_count:1;

	union {
		struct {
			cw_bool_t	val;
		}	boolean;
		struct {
			cw_sint32_t	fd;
		}	file;
		struct {
			union {
				const void	*key; /*
						       * Key used for global
						       * names.
						       */
				cw_stilt_t	*stilt;
			}	s;
			cw_stiln_t	*stiln;
		}	name;
		struct {
			cw_sint32_t	s32;
		}	number;
		struct {
			void	(*f)(cw_stilt_t *);
		}	operator;
		cw_stiloe_t	*stiloe;
	}	o;
};

void		stilo_new(cw_stilo_t *a_stilo);
void		stilo_delete(cw_stilo_t *a_stilo);

cw_stilot_t	stilo_type_get(cw_stilo_t *a_stilo);
void		stilo_type_set(cw_stilo_t *a_stilo, cw_stilot_t a_stilot);

cw_bool_t	stilo_executable_get(cw_stilo_t *a_stilo);
void		stilo_executable_set(cw_stilo_t *a_stilo, cw_bool_t
    a_executable);

cw_bool_t	stilo_literal_get(cw_stilo_t *a_stilo);
void		stilo_literal_set(cw_stilo_t *a_stilo, cw_bool_t a_literal);

cw_stiloe_t	*stilo_extended_get(cw_stilo_t *a_stilo);
void		stilo_extended_set(cw_stilo_t *a_stilo, cw_stiloe_t *a_stiloe);

void		stilo_copy(cw_stilo_t *a_to, cw_stilo_t *a_from);
void		stilo_move(cw_stilo_t *a_to, cw_stilo_t *a_from);

void		stilo_print(cw_stilo_t *a_stilo, cw_sint32_t a_fd, cw_bool_t
    a_syntactic, cw_bool_t a_newline);


cw_sint32_t	stilo_array_len_get(cw_stilo_t *a_stilo);
void		stilo_array_len_set(cw_stilo_t *a_stilo, cw_uint32_t a_len);
cw_stilo_t	*stilo_array_el_get(cw_stilo_t *a_stilo, cw_uint32_t a_offset);
cw_stilo_t	*stilo_array_get(cw_stilo_t *a_stilo);
void		stilo_array_set(cw_stilo_t *a_stilo, cw_uint32_t a_offset,
    cw_stilo_t *a_arr, cw_uint32_t a_len);
