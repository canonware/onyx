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
#define	_CW_XEPV_CODE		 _CW_XEPV_NONE
#define	_CW_XEPV_FINALLY	-1
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

#define	xep_value()	((cw_xepv_t)_xep.value)

#define	xep_try								\
	for (;;) {							\
		cw_xep_t	_xep;					\
									\
		xep_p_link(&_xep);					\
		switch (setjmp(_xep.context))

#define	xep_end								\
		if (xep_p_unlink(&_xep) == FALSE)			\
			break;						\
	}

#define	xep_rend							\
		{							\
			cw_xepv_t	value = xep_value();		\
									\
			if (xep_p_unlink(&_xep))			\
				return xep_value();			\
			else if (x <= 0)				\
				break;					\
		}							\
	}

void	xep_raise_e(cw_xepv_t a_value, const char *a_filename, cw_uint32_t
    a_line_num);
#define xep_raise(a_value)	xep_raise_e((a_value), __FILE__, __LINE__)
void	xep_retry(void);
void	xep_handled(void);

/*
 * Private, but visible here so that the cpp macros above don't cause
 * compilation warnings.
 */ 
void	xep_p_link(cw_xep_t *a_xep);
cw_bool_t xep_p_unlink(cw_xep_t *a_xep);
