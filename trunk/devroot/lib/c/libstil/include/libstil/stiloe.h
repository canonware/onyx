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

#if (0)
typedef struct cw_stiloe_s cw_stiloe_t;
typedef struct cw_stiloei_s cw_stiloei_t;
#endif

typedef enum {
	_CW_STILOEA_NONE = 0,
	_CW_STILOEA_UNLIMITED = 1,
	_CW_STILOEA_READONLY = 2,
	_CW_STILOEA_EXECUTEONLY = 3
}	cw_stiloea_t;

/*
 * Operations on of type array and string can cause subranges of the objects to
 * be referred to by newly created objects.  These objects actually share the
 * data, rather than having their own copy.  The stiloei class contains the
 * information necessary to refer to a stiloe.
 */
struct cw_stiloei_s {
#if (defined(_LIBSTIL_DBG) || defined(_LIBSTIL_DEBUG))
	cw_uint32_t	magic;
#endif

	cw_stiloe_t	*stiloe;
	cw_uint32_t	beg_offset;
	cw_uint32_t	end_offset;
};

/*
 * All extended type objects contain a stiloe.  This provides a poor man's
 * inheritance.  Since stil's type system is static, this idiom is adequate.
 */
struct cw_stiloe_s {
#if (defined(_LIBSTIL_DBG) || defined(_LIBSTIL_DEBUG))
	cw_uint32_t	magic;
#endif

	cw_stilot_t	type:4;
	cw_stiloea_t	access:2;
	/*
	 * If TRUE, there is a watchpoint set on this object.  In general, this
	 * field is not looked at unless the interpreter has been put into
	 * debugging mode. Note that setting a watchpoint on an extended type
	 * causes modification via *any* reference to be watched.
	 */
	cw_bool_t	watchpoint:1;
	/* If TRUE, this object is black (or gray).  Otherwise it is white. */
	cw_bool_t	black:1;
	/*
	 * If TRUE, this object cannot be modified, which means it need not be
	 * locked, even if global.
	 */
	cw_bool_t	immutable:1;
	/*
	 * If TRUE, this object is an indirect reference to a subrange of
	 * another object (array or string).
	 */
	cw_bool_t	indirect:1;
};

void	stiloe_new(cw_stiloe_t *a_stiloe);
void	stiloe_delete(cw_stiloe_t *a_stiloe);

#define	stiloe_is_indirect(a_stiloe) (a_stiloe)->indirect
