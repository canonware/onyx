/****************************************************************************
 *
 * <Copyright = jasone>
 * <License>
 *
 ****************************************************************************
 *
 * Version: <Version>
 *
 * Description: Master header file for libstash.
 *
 ****************************************************************************/

#ifdef __cplusplus
extern  "C" {
#endif

#ifndef _LIBSTASH_H_
#define _LIBSTASH_H_

#define _LIBSTASH_VERSION_ <Version>

/* Must be defined for pthreads. */
#ifndef _REENTRANT
#define _REENTRANT
#endif

#include "libstash_defs.h"

/*
 * Global typedefs.
 */

#if (SIZEOF_SIGNED_CHAR == 1)
#define _TYPE_SINT8_DEFINED
typedef signed char cw_sint8_t;
#endif

#if (SIZEOF_SIGNED_CHAR == 1)
#define _TYPE_UINT8_DEFINED
typedef unsigned char cw_uint8_t;
#endif

#if (SIZEOF_SIGNED_SHORT == 2)
#define _TYPE_SINT16_DEFINED
typedef signed short cw_sint16_t;
#endif

#if (SIZEOF_UNSIGNED_SHORT == 2)
#define _TYPE_UINT16_DEFINED
typedef unsigned short cw_uint16_t;
#endif

#if (SIZEOF_INT == 4)
#define _TYPE_SINT32_DEFINED
typedef int cw_sint32_t;
#endif

#if (SIZEOF_UNSIGNED == 4)
#define _TYPE_UINT32_DEFINED
typedef unsigned cw_uint32_t;
#endif

#if (SIZEOF_LONG == 8)
#define _TYPE_SINT64_DEFINED
typedef long cw_sint64_t;
#endif

#if (SIZEOF_UNSIGNED_LONG == 8)
#define _TYPE_UINT64_DEFINED
typedef unsigned long cw_uint64_t;
#endif

#if (SIZEOF_LONG_LONG == 8)
#define _TYPE_SINT64_DEFINED
typedef long long cw_sint64_t;
#endif

#if (SIZEOF_UNSIGNED_LONG_LONG == 8)
#define _TYPE_UINT64_DEFINED
typedef unsigned long long cw_uint64_t;
#endif

#if (!defined(_TYPE_SINT8_DEFINED) || !defined(_TYPE_UINT8_DEFINED) ||	\
    !defined(_TYPE_SINT16_DEFINED) || !defined(_TYPE_UINT16_DEFINED) ||	\
    !defined(_TYPE_SINT32_DEFINED) || !defined(_TYPE_UINT32_DEFINED) ||	\
    !defined(_TYPE_SINT64_DEFINED) || !defined(_TYPE_UINT64_DEFINED))
#error "Lacking mandatory typedefs"
#endif

#if (SIZEOF_FLOAT == 4)
#define _TYPE_FP32_DEFINED
typedef float cw_fp32_t;
#endif

#if (SIZEOF_DOUBLE == 8)
#define _TYPE_FP64_DEFINED
typedef double cw_fp64_t;
#endif

#if (SIZEOF_LONG_DOUBLE == 12)
#define _TYPE_FP96_DEFINED
typedef long double cw_fp96_t;
#elif (SIZEOF_LONG_DOUBLE == 16)
#define _TYPE_FP128_DEFINED
typedef long double cw_fp128_t;
#endif

/*
 * Grossness to make sure things still work, even if TRUE and/or FALSE are/is
 * defined.
 */
#ifdef FALSE
#define _CW_FALSE_DEFINED
#undef FALSE
#endif
#ifdef TRUE
#define _CW_TRUE_DEFINED
#undef TRUE
#endif

typedef enum {
	FALSE,
	TRUE
} cw_bool_t;

/*
 * Generic typedef used for memory deallocation hooks.  This typedef is
 * compatible with functions such as mem_free(), pezz_put() and pool_put().
 */
typedef void cw_opaque_dealloc_t (const void *, const void *, const char *,
    cw_uint32_t);

/*
 * More grossness to make sure things still work, even if TRUE and/or FALSE
 * are/is defined.
 */
#ifdef _CW_FALSE_DEFINED
#define FALSE (0)
#undef _CW_FALSE_DEFINED
#endif
#ifdef _CW_TRUE_DEFINED
#define TRUE (1)
#undef _CW_TRUE_DEFINED
#endif

/*
 * Project headers to always be included.
 */

#include "libstash_incs.h"

/*
 * libstash initialization and shutdown function prototypes.
 */
cw_bool_t libstash_init(void);

void    libstash_shutdown(void);

/*
 * Global variables.
 */
extern cw_mem_t *cw_g_mem;
extern cw_dbg_t *cw_g_dbg;
extern cw_out_t *cw_g_out;

/*
 * Global macros we use everywhere.
 */

/*
 * Used for deallocation via an opaque function pointer.  These macros are used
 * to call functions such as mem_free(), pezz_put(), and pool_put().
 */
#if (defined(_LIBSTASH_DBG) || defined(_LIBSTASH_DEBUG))
#define _cw_opaque_dealloc(a_func, a_arg, a_ptr)			\
	(a_func)((void *)(a_arg), (void *)(a_ptr), __FILE__, __LINE__)
#else
#define _cw_opaque_dealloc(a_func, a_arg, a_ptr)			\
	(a_func)((void *)(a_arg), (void *)(a_ptr), NULL, 0)
#endif

#ifdef WORDS_BIGENDIAN
#define _cw_ntohq(a) (a)
#define _cw_htonq(a) (a)
#else
#define _cw_ntohq(a)							\
	(cw_uint64_t) (((cw_uint64_t) (ntohl((cw_uint32_t) ((a) >>	\
	    32)))) | (((cw_uint64_t) (ntohl((cw_uint32_t) ((a) &	\
	    0x00000000ffffffff)))) << 32))
#define _cw_htonq(a)							\
	(cw_uint64_t) (((cw_uint64_t) (htonl((cw_uint32_t) ((a) >>	\
	    32)))) | (((cw_uint64_t) (htonl((cw_uint32_t) ((a) &	\
	    0x00000000ffffffff)))) << 32))
#endif

/*
 * assert()-alike.  It's a bit prettier and cleaner, but the same idea.
 */
#define _cw_error(a)							\
	do {								\
		out_put_e(cw_g_out, __FILE__, __LINE__, __FUNCTION__,	\
		    "Error: [s]\n", a);					\
		abort();						\
	} while (0)

#if (defined(_LIBSTASH_DBG) || defined(_LIBSTASH_DEBUG))
#define _cw_assert(a)							\
	do {								\
		if (!(a)) {						\
			out_put_e(cw_g_out, __FILE__, __LINE__,		\
			    __FUNCTION__, "Failed assertion: \"[s]\"\n", \
			    #a);					\
			abort();					\
		}							\
	} while (0)

/* Macro to do the drudgery of assuring that a pointer is non-NULL. */
#define _cw_check_ptr(x)						\
	do {								\
		if (((x) == NULL) || ((x) == (void *) 0xa5a5a5a5) ||	\
		    ((x) == (void *) 0x5a5a5a5a)) {			\
			out_put_e(cw_g_out, __FILE__, __LINE__,		\
			    __FUNCTION__,				\
			    "[s] (0x[p]) is an invalid pointer\n", #x, (x)); \
			abort();					\
		}							\
	} while (0)
#else
#define _cw_assert(a)
#define _cw_check_ptr(a)
#endif

#endif /* _LIBSTASH_H_ */

#ifdef __cplusplus
};
#endif
