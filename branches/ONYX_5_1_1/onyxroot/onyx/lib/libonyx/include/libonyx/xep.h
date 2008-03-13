/* -*- mode: c ; c-file-style: "canonware-c-style" -*-
 ******************************************************************************
 *
 * <Copyright = jasone>
 * <License>
 *
 ******************************************************************************
 *
 * Version: Onyx <Version = onyx>
 *
 ******************************************************************************/

/* Pseudo-opaque type. */
typedef struct cw_xep_s cw_xep_t;

typedef uint32_t cw_xepv_t;

#define CW_XEPV_NONE 0
#define CW_XEPV_CODE 1

typedef enum
{
    CW_XEPS_TRY,
    CW_XEPS_CATCH
} cw_xeps_t;

struct cw_xep_s
{
    volatile qr(cw_xep_t) link;
    volatile cw_xepv_t value;
    volatile bool is_handled;
    volatile bool is_linked;
    volatile cw_xeps_t state;
    volatile const char *filename;
    volatile uint32_t line_num;
    jmp_buf context;
};

#define xep_begin()							\
    {									\
	cw_xep_t _xep

#define xep_try								\
	xep_p_link(&_xep);						\
	switch (setjmp(_xep.context))					\
	{								\
	    case CW_XEPV_NONE:						\
	    case CW_XEPV_CODE:

#define xep_catch(a_value)						\
		break;							\
	    case (a_value):

#define xep_mcatch(a_value)						\
	    case (a_value):

#define xep_acatch							\
		break;							\
	    default:							\
		if (_xep.state != CW_XEPS_CATCH)			\
		{							\
		    break;						\
		}

#define xep_end()							\
	}								\
	xep_p_unlink(&_xep);						\
    }

#define xep_value() (_xep.value)

#define xep_filename() (_xep.filename)

#define xep_line_num() (_xep.line_num)

void
xep_throw_e(cw_xepv_t a_value, volatile const char *a_filename,
	    uint32_t a_line_num);

#define xep_throw(a_value) xep_throw_e((a_value), __FILE__, __LINE__)

#define xep_retry() xep_p_retry(&_xep)

#define xep_handled() xep_p_handled(&_xep)

/* Private, but visible here so that the cpp macros above don't cause
 * compilation warnings. */
void
xep_p_retry(cw_xep_t *a_xep);

void
xep_p_handled(cw_xep_t *a_xep);

void
xep_p_link(cw_xep_t *a_xep);

void
xep_p_unlink(cw_xep_t *a_xep);
