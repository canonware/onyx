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
#endif
typedef struct cw_stiloer_s cw_stiloer_t;

typedef enum {
	_CW_STILOEA_NONE = 0,
	_CW_STILOEA_UNLIMITED = 1,
	_CW_STILOEA_READONLY = 2,
	_CW_STILOEA_EXECUTEONLY = 3
} cw_stiloea_t;

/*
 * All extended type objects contain a stiloe.  This provides a poor man's
 * inheritance.  Since stil's type system is static, this idiom is adequate.
 */
struct cw_stiloe_s {
#if (defined(_LIBSTIL_DBG) || defined(_LIBSTIL_DEBUG))
	cw_uint32_t magic;
#endif

	cw_stilot_t type:4;
	cw_stiloea_t access:2;
	/*
	 * If TRUE, there is a watchpoint set on this object.  In general,
	 * this field is not looked at unless the interpreter has been put
	 * into debugging mode. Note that setting a watchpoint on an
	 * extended type causes modification via *any* reference to be
	 * watched.
	 */
	cw_bool_t watchpoint:1;
	/* If TRUE, this object is black (or gray).  Otherwise it is white. */
	cw_bool_t black:1;
	/*
	 * If TRUE, this object cannot be modified, which means it need not
	 * be locked, even if global.
	 */
	cw_bool_t immutable:1;
};

void	stiloe_new(cw_stiloe_t *a_stiloe);

void	stiloe_delete(cw_stiloe_t *a_stiloe);
