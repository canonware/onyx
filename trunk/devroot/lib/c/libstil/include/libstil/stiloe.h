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

#ifdef _LIBSTIL_DBG
#define _CW_STILOE_MAGIC 0x0fa6e798
#endif

typedef enum {
	_CW_STILOEA_NONE = 0,
	_CW_STILOEA_EXECUTEONLY = 1,
	_CW_STILOEA_READONLY = 2,
	_CW_STILOEA_UNLIMITED = 3
}	cw_stiloea_t;

/*
 * Operations on of type array and string can cause subranges of the objects to
 * be referred to by newly created objects.  These composite objects actually
 * share the data, rather than having their own copy.  The stiloec class
 * contains the information necessary to refer to a stiloe.
 */
struct cw_stiloec_s {
#if (defined(_LIBSTIL_DBG) || defined(_LIBSTIL_DEBUG))
	cw_uint32_t	magic;
#endif

	cw_stiloe_t	*stiloe;
	cw_uint32_t	beg_offset;
	cw_uint32_t	len;
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
	/* Allocated locally or globally? */
	cw_bool_t	global:1;
	/*
	 * If TRUE, this object cannot be modified, which means it need not be
	 * locked, even if global.
	 */
	cw_bool_t	immutable:1;
	/*
	 * If TRUE, this object is a reference to a subrange of another object
	 * (array or string).
	 */
	cw_bool_t	composite:1;
	/*
	 * We need a way to add a stiloe to the sequence set if there become
	 * multiple references to this stiloe.
	 */
	cw_stilt_t	*stilt;
};

cw_stiloe_t	*stiloe_new(cw_stilt_t *a_stilt, cw_stilot_t a_type);
void	stiloe_delete(cw_stiloe_t *a_stiloe);

void	stiloe_gc_register(cw_stiloe_t *a_stiloe);
cw_stiloe_t	*stiloe_ref_iterate(cw_stiloe_t *a_stiloe, cw_bool_t a_reset);
