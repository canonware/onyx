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

typedef struct cw_stilo_s cw_stilo_t;
typedef struct cw_stiloe_s cw_stiloe_t;
typedef struct cw_stiloe_dicto_s cw_stiloe_dicto_t;

/* Declared here to avoid a circular header dependency. */
typedef void cw_op_t(cw_stilt_t *);

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

typedef enum {
	_CW_STILOA_NONE = 0,
	_CW_STILOA_EXECUTEONLY = 1,
	_CW_STILOA_READONLY = 2,
	_CW_STILOA_UNLIMITED = 3
}	cw_stiloa_t;

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
	cw_stiloa_t	access:2;
	/*
	 * Every object is either literal (FALSE) or executable (TRUE).
	 */
	cw_bool_t	executable:1;
	/*
	 * A literal object if TRUE.
	 */
	cw_bool_t	literal:1;
	/*
	 * If TRUE, this is an extended (not the same as composite) object.
	 * This only pertains to number stilo's, which can switch between simple
	 * and extended representations.
	 *
	 * For a number stilo, if not extended, this number is representable as
	 * a 32 bit signed integer.  Otherwise o.stiloe contains the value.
	 */
	cw_bool_t	extended:1;
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
			cw_sint32_t	fd;
		}	file;
		struct {
			cw_sint32_t	s32;
		}	number;
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

/*
 * The ... arg is used for array, dict, name, operator, and string construction.
 */
void		stilo_new(cw_stilo_t *a_stilo, cw_stilt_t *a_stilt, cw_stilot_t
    a_type, ...);
void		stilo_new_v(cw_stilo_t *a_stilo, cw_stilt_t *a_stilt,
    cw_stilot_t a_type, va_list a_p);

void		stilo_delete(cw_stilo_t *a_stilo, cw_stilt_t *a_stilt);

cw_stilot_t	stilo_type_get(cw_stilo_t *a_stilo);

cw_bool_t	stilo_executable_get(cw_stilo_t *a_stilo);
void		stilo_executable_set(cw_stilo_t *a_stilo, cw_bool_t
    a_executable);

cw_bool_t	stilo_literal_get(cw_stilo_t *a_stilo);
void		stilo_literal_set(cw_stilo_t *a_stilo, cw_bool_t a_literal);

void		stilo_cast(cw_stilo_t *a_stilo, cw_stilot_t a_stilot);
void		stilo_copy(cw_stilo_t *a_to, cw_stilo_t *a_from, cw_stilt_t
    *a_stilt);
void		stilo_dup(cw_stilo_t *a_to, cw_stilo_t *a_from);
void		stilo_move(cw_stilo_t *a_to, cw_stilo_t *a_from);

void		stilo_print(cw_stilo_t *a_stilo, cw_sint32_t a_fd, cw_bool_t
    a_syntactic, cw_bool_t a_newline);

/* XXX For the GC only. */
cw_stiloe_t	*stilo_l_stiloe_get(cw_stilo_t *a_stilo);
cw_stiloe_t	*stiloe_l_ref_iterate(cw_stiloe_t *a_stiloe, cw_bool_t a_reset);

/*
 * array.
 */
cw_sint32_t	stilo_array_len_get(cw_stilo_t *a_stilo);
cw_stilo_t	*stilo_array_el_get(cw_stilo_t *a_stilo, cw_uint32_t a_offset);
cw_stilo_t	*stilo_array_get(cw_stilo_t *a_stilo);
void		stilo_array_set(cw_stilo_t *a_stilo, cw_uint32_t a_offset,
    cw_stilo_t *a_arr, cw_uint32_t a_len, cw_stilt_t *a_stilt);

/*
 * dict.
 */
/* Destroys a_key and a_val. */
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
 * name.
 */
cw_uint32_t	stilo_name_hash(const void *a_key);
cw_bool_t	stilo_name_key_comp(const void *a_k1, const void *a_k2);

/*
 * string.
 */
cw_sint32_t	stilo_string_len_get(cw_stilo_t *a_stilo);
cw_uint8_t	*stilo_string_el_get(cw_stilo_t *a_stilo, cw_uint32_t a_offset);
cw_uint8_t	*stilo_string_get(cw_stilo_t *a_stilo);
void		stilo_string_set(cw_stilo_t *a_stilo, cw_uint32_t a_offset,
    const cw_uint8_t *a_str, cw_uint32_t a_len);
