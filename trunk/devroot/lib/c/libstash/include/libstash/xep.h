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

/* Pseudo-opaque type. */
typedef struct cw_xep_s cw_xep_t;

typedef cw_sint32_t cw_xepv_t;

#define	_CW_XEPV_NONE		 0
#define	_CW_XEPV_CODE		-1
#define	_CW_XEPV_FINALLY	-2
#define	_CW_XEPV_OOM		-3
#define _CW_XEPV_OUT_PARSE	-4
/*
 * Exceptions should be numbered negatively, and care should be taken to avoid
 * duplicates.  -1 through -127 are reserved by libstash.
 */

typedef enum {
	_CW_XEPS_TRY,
	_CW_XEPS_CATCH,
	_CW_XEPS_FINALLY
} cw_xeps_t;

struct cw_xep_s {
	qr(cw_xep_t)	link;
	jmp_buf		context;
	cw_xepv_t	value;
	cw_bool_t	is_handled;
	cw_xeps_t	state;
	const char	*filename;
	cw_uint32_t	line_num;
};

#define xep_begin()							\
	{								\
		cw_xep_t	_xep

#define	xep_try								\
		xep_p_link(&_xep);					\
		switch (setjmp(_xep.context)) {				\
		case _CW_XEPV_NONE:					\
		case _CW_XEPV_CODE:

#define xep_catch(a_value)						\
			break;						\
		case (a_value):

#define xep_mcatch(a_value)						\
		case (a_value):

#define	xep_acatch							\
			break;						\
		default:

#define xep_finally xep_catch(_CW_XEPV_FINALLY)

#define	xep_end()							\
		}							\
		xep_p_unlink(&_xep);					\
	}

#define	xep_value()	(_xep.value)

void	xep_throw_e(cw_xepv_t a_value, const char *a_filename, cw_uint32_t
    a_line_num);
#define xep_throw(a_value)	xep_throw_e((a_value), __FILE__, __LINE__)
void	xep_retry(void);
void	xep_handled(void);

/*
 * Private, but visible here so that the cpp macros above don't cause
 * compilation warnings.
 */ 
void	xep_p_link(cw_xep_t *a_xep);
void	xep_p_unlink(cw_xep_t *a_xep);
