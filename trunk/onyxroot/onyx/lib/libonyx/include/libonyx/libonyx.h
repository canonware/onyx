/******************************************************************************
 *
 * <Copyright = jasone>
 * <License>
 *
 ******************************************************************************
 *
 * Version: <Version>
 *
 * Master header file for libonyx.
 *
 ******************************************************************************/

#ifdef __cplusplus
extern  "C" {
#endif

#ifndef _LIBONYX_H_
#define _LIBONYX_H_

#define _CW_LIBONYX_VERSION "<Version>"

/* Must be defined for pthreads. */
#ifndef _REENTRANT
#define _REENTRANT
#endif

#include "libonyx_defs.h"

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

#ifndef _TYPE_SINT64_DEFINED
#if (SIZEOF_LONG_LONG == 8)
#define _TYPE_SINT64_DEFINED
typedef long long cw_sint64_t;
#endif
#endif

#ifndef _TYPE_UINT64_DEFINED
#if (SIZEOF_UNSIGNED_LONG_LONG == 8)
#define _TYPE_UINT64_DEFINED
typedef unsigned long long cw_uint64_t;
#endif
#endif

#if (!defined(_TYPE_SINT8_DEFINED) || !defined(_TYPE_UINT8_DEFINED) ||	\
    !defined(_TYPE_SINT16_DEFINED) || !defined(_TYPE_UINT16_DEFINED) ||	\
    !defined(_TYPE_SINT32_DEFINED) || !defined(_TYPE_UINT32_DEFINED) ||	\
    !defined(_TYPE_SINT64_DEFINED) || !defined(_TYPE_UINT64_DEFINED))
#error "Lacking mandatory typedefs"
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
 * Minimum initial size of dictionaries.
 */
#define	_CW_LIBONYX_DICT_SIZE		   16

/*
 * Maximum depth of estack.
 */
#define	_CW_LIBONYX_ESTACK_MAX		  256

/*
 * Default file buffer size.
 */
#define	_CW_LIBONYX_FILE_BUFFER_SIZE	  512

/*
 * Size of stack-allocated buffer to use when executing file objects.  This
 * generally doesn't need to be huge, because there is usually additional
 * buffering going on upstream.
 */
#define	_CW_LIBONYX_FILE_EVAL_READ_SIZE	  128

/*
 * Default minimum period of registration inactivity before a periodic
 * collection is done (if any registrations have occured since the last
 * collection).  On average, the actual inactivity period will be 1.5 times
 * this, but can range from 1 to 2 times this.
 */
#define	_CW_LIBONYX_GCDICT_PERIOD	   20

/*
 * Default number of bytes of allocation since last collection that will cause
 * an immediate collection.
 */
#define	_CW_LIBONYX_GCDICT_THRESHOLD   262144

/*
 * Initial size of globaldict.  This is a bit arbitrary, and some applications
 * could benefit from making it larger or smaller.
 */
#define	_CW_LIBONYX_GLOBALDICT_HASH	   64

/*
 * Initial size of threadsdict.  Most applications don't use many threads, so
 * the initial size is set pretty low.
 */
#define	_CW_LIBONYX_THREADSDICT_HASH	   16

/*
 * Initial size initial name cache hash table.  We know for sure that there will
 * be about 250 names referenced by systemdict, errordict, and currenterror to
 * begin with.
 */
#define _CW_LIBONYX_NAME_HASH		 1024

/*
 * Maximum number of stack elements to cache in a stack before reclaiming unused
 * elements.
 */
#define	_CW_LIBONYX_STACK_CACHE		   16

/*
 * Initial size of threaddict.
 */
#define	_CW_LIBONYX_THREADDICT_HASH	    4

/*
 * Exception numbers.  libonyx reserves 0 through 127.
 */
#define	_CW_ONYXX_MIN			    0
#define	_CW_ONYXX_MAX			  127

/* Out of memory. */
#define	_CW_ONYXX_OOM			    3
/* Internal use, for the exit operator. */
#define	_CW_ONYXX_EXIT			    4
/* Internal use, for the stop operator, caught by the stopped operator. */
#define	_CW_ONYXX_STOP			    5
/* Internal use, for the quit operator, caught by the start operator. */
#define	_CW_ONYXX_QUIT			    6

#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <unistd.h>
#include <limits.h>
#include <string.h>
#include <strings.h>
#include <sys/types.h>
#include <sys/uio.h>
#ifdef _CW_THREADS
#include <pthread.h>
#include <semaphore.h>
#include <sched.h>
#endif
#include <setjmp.h>
#include <signal.h>

/*
 * Generic typedef used for memory allocation hooks.  This typedef is compatible
 * with functions such as mem_malloc_e().
 */
typedef void *cw_opaque_alloc_t (const void *, size_t, const char *,
    cw_uint32_t);

/*
 * Generic typedef used for memory reallocation hooks.  This typedef is
 * compatible with functions such as mem_realloc_e().
 */
typedef void *cw_opaque_realloc_t (const void *, void *, size_t, size_t, const
    char *, cw_uint32_t);

/*
 * Generic typedef used for memory deallocation hooks.  This typedef is
 * compatible with functions such as mem_free_e(), pezz_put_e() and
 * pool_put_e().
 */
typedef void cw_opaque_dealloc_t (const void *, const void *, size_t, const char
    *, cw_uint32_t);

#include "qs.h"
#include "qr.h"
#include "ql.h"
#include "xep.h"
#ifdef _CW_THREADS
#include "mtx.h"
#include "cnd.h"
#include "tsd.h"
#include "thd.h"
#endif
#include "ch.h"
#include "dch.h"
#include "mem.h"
#ifdef _CW_THREADS
#include "mq.h"
#endif

/*
 * libonyx initialization and shutdown function prototypes.
 */
void	libonyx_init(void);
void	libonyx_shutdown(void);

/*
 * Global variables.
 */
extern cw_mem_t	*cw_g_mem;

/*
 * Used for deallocation via an opaque function pointer.  These macros are used
 * to call functions such as mem_free(), pezz_put(), and pool_put().
 */
#ifdef _CW_DBG
#define	_cw_opaque_alloc(a_func, a_arg, a_size)				\
	(a_func)((void *)(a_arg), (size_t)(a_size), __FILE__, __LINE__)
#define	_cw_opaque_realloc(a_func, a_ptr, a_arg, a_size, a_old_size)	\
	(a_func)((void *)(a_arg), (void *)(a_ptr), (size_t)(a_size),	\
	    (size_t)(a_old_size), __FILE__, __LINE__)
#define _cw_opaque_dealloc(a_func, a_arg, a_ptr, a_size)		\
	(a_func)((void *)(a_arg), (void *)(a_ptr), (size_t)(a_size),	\
	    __FILE__, __LINE__)
#else
#define	_cw_opaque_alloc(a_func, a_arg, a_size)				\
	(a_func)((void *)(a_arg), (size_t)(a_size), NULL, 0)
#define	_cw_opaque_realloc(a_func, a_ptr, a_arg, a_size)		\
	(a_func)((void *)(a_arg), (void *)(a_ptr), (size_t)(a_size),	\
	    (size_t)(a_old_size), NULL, 0)
#define _cw_opaque_dealloc(a_func, a_arg, a_ptr, a_size)		\
	(a_func)((void *)(a_arg), (void *)(a_ptr), (size_t)(a_size),	\
	    NULL, 0)
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
		fprintf(stderr, "%s:%u:%s(): Error: %s\n", __FILE__,	\
		    __LINE__, __FUNCTION__, a);				\
		abort();						\
	} while (0)

#ifdef _CW_ASSERT
#define _cw_not_reached()						\
	do {								\
		fprintf(stderr,						\
		    "%s:%u:%s(): Unreachable code reached\n", __FILE__,	\
		    __LINE__, __FUNCTION__);				\
		abort();						\
	} while (0)

#define _cw_assert(a)							\
	do {								\
		if (!(a)) {						\
			fprintf(stderr,					\
			    "%s:%u:%s(): Failed assertion: \"%s\"\n",	\
			    __FILE__, __LINE__, __FUNCTION__, #a);	\
			abort();					\
		}							\
	} while (0)

/* Macro to do the drudgery of assuring that a pointer is non-NULL. */
#define _cw_check_ptr(x)						\
	do {								\
		if (((x) == NULL) || ((x) == (void *) 0xa5a5a5a5) ||	\
		    ((x) == (void *) 0x5a5a5a5a)) {			\
			fprintf(stderr,					\
			    "%s:%u:%s(): Invalid pointer: %s (%p)\n",	\
			    __FILE__, __LINE__, __FUNCTION__, #x, (x));	\
			abort();					\
		}							\
	} while (0)
#else
#define _cw_not_reached()
#define _cw_assert(a)
#define _cw_check_ptr(a)
#endif

/*
 * _cw_dasssert() is used internally in places that the assertion should only
 * be made if _CW_DBG is defined, such as checking magic variables that only
 * exist in that case.
 */
#if (defined(_CW_DBG) && defined(_CW_ASSERT))
#define _cw_dassert(a)							\
	do {								\
		if (!(a)) {						\
			fprintf(stderr,					\
			    "%s:%u:%s(): Failed assertion: \"%s\"\n",	\
			    __FILE__, __LINE__, __FUNCTION__, #a);	\
			abort();					\
		}							\
	} while (0)
#else
#define _cw_dassert(a)
#endif

#include "nxn.h"
#include "nxo.h"
#include "nxo_no.h"
#include "nxo_array.h"
#include "nxo_boolean.h"
#ifdef _CW_THREADS
#include "nxo_condition.h"
#endif
#include "nxo_dict.h"
#include "nxo_file.h"
#include "nxo_fino.h"
#include "nxo_hook.h"
#include "nxo_integer.h"
#include "nxo_mark.h"
#ifdef _CW_THREADS
#include "nxo_mutex.h"
#endif
#include "nxo_name.h"
#include "nxo_null.h"
#include "nxo_operator.h"
#include "nxo_pmark.h"
#include "nxo_stack.h"
#include "nxo_string.h"
#include "nxo_thread.h"
#include "nxa.h"
#include "nx.h"
#include "systemdict.h"
#include "gcdict.h"

/* Convenience macro for static embedded onyx code. */
#define	_cw_onyx_code(a_thread, a_code) do {				\
	cw_nxo_threadp_t	threadp;				\
	static const cw_uint8_t	code[] = (a_code);			\
									\
	nxo_threadp_new(&threadp);					\
	nxo_thread_interpret((a_thread), &threadp, code,		\
	    sizeof(code) - 1);						\
	nxo_thread_flush((a_thread), &threadp);				\
	nxo_threadp_delete(&threadp, (a_thread));			\
} while (0)

#endif	/* _LIBONYX_H_ */

#ifdef __cplusplus
};
#endif
