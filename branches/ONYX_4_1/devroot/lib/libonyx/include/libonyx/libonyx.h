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
 * Master header file for libonyx.
 *
 ******************************************************************************/

#ifdef __cplusplus
extern "C" {
#endif

#ifndef CW_LIBONYX_H_
#define CW_LIBONYX_H_

#define CW_LIBONYX_VERSION "<Version = onyx>"

/* Must be defined for pthreads. */
#ifndef _REENTRANT
#define _REENTRANT
#endif

#include "libonyx_defs.h"

/* Global typedefs. */

#if (SIZEOF_SIGNED_CHAR == 1)
#define CW_TYPE_SINT8_DEFINED
typedef signed char cw_sint8_t;
#endif

#if (SIZEOF_SIGNED_CHAR == 1)
#define CW_TYPE_UINT8_DEFINED
typedef unsigned char cw_uint8_t;
#endif

#if (SIZEOF_SIGNED_SHORT == 2)
#define CW_TYPE_SINT16_DEFINED
typedef signed short cw_sint16_t;
#endif

#if (SIZEOF_UNSIGNED_SHORT == 2)
#define CW_TYPE_UINT16_DEFINED
typedef unsigned short cw_uint16_t;
#endif

#if (SIZEOF_INT == 4)
#define CW_TYPE_SINT32_DEFINED
typedef int cw_sint32_t;
#endif

#if (SIZEOF_UNSIGNED == 4)
#define CW_TYPE_UINT32_DEFINED
typedef unsigned cw_uint32_t;
#endif

#if (SIZEOF_LONG == 8)
#define CW_TYPE_SINT64_DEFINED
typedef long cw_sint64_t;
#endif

#if (SIZEOF_UNSIGNED_LONG == 8)
#define CW_TYPE_UINT64_DEFINED
typedef unsigned long cw_uint64_t;
#endif

#ifndef CW_TYPE_SINT64_DEFINED
#if (SIZEOF_LONG_LONG == 8)
#define CW_TYPE_SINT64_DEFINED
typedef long long cw_sint64_t;
#endif
#endif

#ifndef CW_TYPE_UINT64_DEFINED
#if (SIZEOF_UNSIGNED_LONG_LONG == 8)
#define CW_TYPE_UINT64_DEFINED
typedef unsigned long long cw_uint64_t;
#endif
#endif

#if (SIZEOF_FLOAT == 4)
#define CW_TYPE_FP32_DEFINED
typedef float cw_fp32_t;
#endif
    
#if (SIZEOF_DOUBLE == 8)
#define CW_TYPE_FP64_DEFINED
typedef double cw_fp64_t;
#endif

#ifndef CW_TYPE_FP64_DEFINED
#if (SIZEOF_LONG_DOUBLE == 8)
#define CW_TYPE_FP64_DEFINED
typedef long double cw_fp64_t;
#endif
#endif
    
#if (!defined(CW_TYPE_SINT8_DEFINED) || !defined(CW_TYPE_UINT8_DEFINED) \
     || !defined(CW_TYPE_SINT16_DEFINED) || !defined(CW_TYPE_UINT16_DEFINED) \
     || !defined(CW_TYPE_SINT32_DEFINED) || !defined(CW_TYPE_UINT32_DEFINED) \
     || !defined(CW_TYPE_SINT64_DEFINED) || !defined(CW_TYPE_UINT64_DEFINED) \
     || !defined(CW_TYPE_FP32_DEFINED) || !defined(CW_TYPE_FP64_DEFINED))
#error "Lacking mandatory typedefs"
#endif

/* Grossness to make sure things still work, even if TRUE and/or FALSE are/is
 * defined. */
#ifdef FALSE
#define CW_FALSE_DEFINED
#undef FALSE
#endif
#ifdef TRUE
#define CW_TRUE_DEFINED
#undef TRUE
#endif

typedef enum
{
    FALSE,
    TRUE
} cw_bool_t;

/* More grossness to make sure things still work, even if TRUE and/or FALSE
 * are/is defined. */
#ifdef CW_FALSE_DEFINED
#define FALSE (0)
#undef CW_FALSE_DEFINED
#endif
#ifdef CW_TRUE_DEFINED
#define TRUE (1)
#undef CW_TRUE_DEFINED
#endif

/* Minimum initial size of dictionaries.  This number shouldn't be too large,
 * since for dictionaries below this size, an array is used internally instead
 * of a hash table, which means that operations require linear searches. */
#define CW_LIBONYX_DICT_SIZE 8

/* Maximum depth of estack. */
#define CW_LIBONYX_ESTACK_MAX 256

/* Default file buffer size. */
#define CW_LIBONYX_FILE_BUFFER_SIZE 512

/* Size of stack-allocated buffer to use when executing file objects.  This
 * generally doesn't need to be huge, because there is usually additional
 * buffering going on upstream. */
#define CW_LIBONYX_FILE_EVAL_READ_SIZE 128

/* Default minimum period of registration inactivity before a periodic
 * collection is done (if any registrations have occured since the last
 * collection).  On average, the actual inactivity period will be 1.5 times
 * this, but can range from 1 to 2 times this. */
#define CW_LIBONYX_GCDICT_PERIOD 20

/* Default number of bytes of allocation since last collection that will cause
 * an immediate collection. */
#define CW_LIBONYX_GCDICT_THRESHOLD 262144

/* Initial size of globaldict.  This is a bit arbitrary, and some applications
 * could benefit from making it larger or smaller. */
#define CW_LIBONYX_GLOBALDICT_HASH CW_LIBONYX_DICT_SIZE

/* Initial size of onyxdict.  Onyx uses this dictionary as a separate namespace
 * for configuration parameters and variables that shouldn't normally be seen by
 * dstack name lookups. */
#define CW_LIBONYX_ONYXDICT_HASH CW_LIBONYX_DICT_SIZE

/* Extra slots in systemdict left open at creation time.  The embedded Onyx
 * initialization code uses several slots, so if adding additional entries to
 * systemdict and memory usage of the hash table is critical, it will be
 * necessary to count the number of slots used by the embedded Onyx
 * initialization code for the particular configuration, and add that number to
 * the number of extra slots needed by the application.  In general, there
 * should be no reason to change this value though. */
#define CW_LIBONYX_SYSTEMDICT_HASH_SPARE 16

/* Initial size of threadsdict.  Most applications don't use many threads, so
 * the initial size is set pretty low. */
#define CW_LIBONYX_THREADSDICT_HASH CW_LIBONYX_DICT_SIZE

/* Initial size initial name cache hash table.  We know for sure that there will
 * be about 250 names referenced by systemdict, errordict, and currenterror to
 * begin with. */
#define CW_LIBONYX_NAME_HASH 1024

/* Maximum number of stack elements to cache in a stack before reclaiming unused
 * elements. */
#define CW_LIBONYX_STACK_CACHE 16

/* Initial size of threaddict. */
#define CW_LIBONYX_THREADDICT_HASH CW_LIBONYX_DICT_SIZE

/* Initial size of dictionaries created with the dict operator. */
#define CW_SYSTEMDICT_DICT_SIZE CW_LIBONYX_DICT_SIZE

/* Exception numbers.  libonyx reserves 0 through 127. */
#define CW_ONYXX_MIN 0
#define CW_ONYXX_MAX 127

/* Out of memory. */
#define CW_ONYXX_OOM 2
/* Internal use, for the exit operator. */
#define CW_ONYXX_EXIT 3
/* Internal use, for the stop operator, caught by the stopped operator. */
#define CW_ONYXX_STOP 4
/* Internal use, for the quit operator, caught by the start operator. */
#define CW_ONYXX_QUIT 5

#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <unistd.h>
#include <limits.h>
#include <string.h>
#include <strings.h>
#include <sys/types.h>
#include <sys/uio.h>
#ifdef HAVE_SYS_SOCKET_H
#include <sys/socket.h> /* For htonl() and ntohl() on Darwin. */
#endif
#ifdef HAVE_NETINET_IN_H
#include <netinet/in.h> /* For htonl() and ntohl() on Darwin. */
#endif
#ifdef HAVE_ARPA_INET_H
#include <arpa/inet.h> /* For htonl() and ntohl(). */
#endif
#ifdef HAVE_MACHINE_ENDIAN_H
#include <machine/endian.h> /* For htonl() and ntohl() on Darwin. */
#endif
#ifdef CW_PTH
#include <pth.h>
#endif
#ifdef CW_PTHREADS
#include <pthread.h>
#include <semaphore.h>
#include <sched.h>
#endif
#ifdef CW_MTHREADS
#include <mach/task.h>
#include <mach/thread_act.h>
#include <mach/mach_init.h>
#endif
#include <setjmp.h>
#include <signal.h>

/* Generic typedef used for memory allocation hooks.  This typedef is compatible
 * with functions such as mem_malloc_e(). */
typedef void *cw_opaque_alloc_t (const void *, size_t, const char *,
				 cw_uint32_t);

/* Generic typedef used for memory reallocation hooks.  This typedef is
 * compatible with functions such as mem_realloc_e(). */
typedef void *cw_opaque_realloc_t (const void *, void *, size_t, size_t,
				   const char *, cw_uint32_t);

/* Generic typedef used for memory deallocation hooks.  This typedef is
 * compatible with functions such as mem_free_e(). */
typedef void cw_opaque_dealloc_t (const void *, const void *, size_t,
				  const char *, cw_uint32_t);

#include "qs.h"
#include "qr.h"
#include "ql.h"
#include "xep.h"
#ifdef CW_THREADS
#include "mtx.h"
#include "cnd.h"
#include "tsd.h"
#include "thd.h"
#endif
#include "mb.h"
#include "ch.h"
#include "dch.h"
#include "mem.h"
#ifdef CW_THREADS
#include "mq.h"
#endif

/* libonyx initialization and shutdown function prototypes. */
void
libonyx_init(void);
    
void
libonyx_shutdown(void);

/* Global variables. */
extern cw_mem_t *cw_g_mem;

#if (defined(CW_POSIX) && defined(CW_THREADS))
extern cw_mtx_t cw_g_gethostbyname_mtx;
extern cw_mtx_t cw_g_getprotobyname_mtx;
extern cw_mtx_t cw_g_getservbyname_mtx;
#endif

/* Used for deallocation via an opaque function pointer.  These macros are used
 * to call functions such as mem_free_e(). */
#ifdef CW_DBG
#define cw_opaque_alloc(a_func, a_arg, a_size)				\
    (a_func)((void *) (a_arg), (size_t) (a_size), __FILE__, __LINE__)
#define cw_opaque_realloc(a_func, a_ptr, a_arg, a_size, a_old_size)	\
    (a_func)((void *) (a_arg), (void *) (a_ptr), (size_t) (a_size),	\
	     (size_t) (a_old_size), __FILE__, __LINE__)
#define cw_opaque_dealloc(a_func, a_arg, a_ptr, a_size)			\
    (a_func)((void *) (a_arg), (void *) (a_ptr), (size_t) (a_size),	\
	     __FILE__, __LINE__)
#else
#define cw_opaque_alloc(a_func, a_arg, a_size)				\
    (a_func)((void *) (a_arg), (size_t) (a_size), NULL, 0)
#define cw_opaque_realloc(a_func, a_ptr, a_arg, a_size, a_old_size)	\
    (a_func)((void *) (a_arg), (void *) (a_ptr), (size_t) (a_size),	\
	     (size_t) (a_old_size), NULL, 0)
#define cw_opaque_dealloc(a_func, a_arg, a_ptr, a_size)			\
    (a_func)((void *) (a_arg), (void *) (a_ptr), (size_t) (a_size),	\
	     NULL, 0)
#endif

#ifdef WORDS_BIGENDIAN
#define cw_ntohq(a) (a)
#define cw_htonq(a) (a)
#else
#define cw_ntohq(a)							\
    (cw_uint64_t) (((cw_uint64_t) (ntohl((cw_uint32_t) ((a) >> 32))))	\
		   | (((cw_uint64_t) (ntohl((cw_uint32_t)		\
		   ((a) & 0x00000000ffffffff)))) << 32))
#define cw_htonq(a)							\
    (cw_uint64_t) (((cw_uint64_t) (htonl((cw_uint32_t) ((a) >> 32))))	\
		   | (((cw_uint64_t) (htonl((cw_uint32_t)		\
		   ((a) & 0x00000000ffffffff)))) << 32))
#endif

/* assert()-alike.  It's a bit prettier and cleaner, but the same idea. */
#define cw_error(a)							\
    do									\
    {									\
	fprintf(stderr, "%s:%u:%s(): Error: %s\n", __FILE__,		\
		__LINE__, __FUNCTION__, a);				\
		abort();						\
    } while (0)

#ifdef CW_ASSERT
#define cw_not_reached()						\
    do									\
    {									\
	fprintf(stderr, "%s:%u:%s(): Unreachable code reached\n",	\
		__FILE__, __LINE__, __FUNCTION__);			\
		abort();						\
    } while (0)

#define cw_assert(a)							\
    do									\
    {									\
	if (!(a))							\
	{								\
	    fprintf(stderr, "%s:%u:%s(): Failed assertion: \"%s\"\n",	\
		    __FILE__, __LINE__, __FUNCTION__, #a);		\
	    abort();							\
	}								\
    } while (0)

/* Macro to do the drudgery of assuring that a pointer is non-NULL. */
#define cw_check_ptr(x)							\
    do									\
    {									\
	if (((x) == NULL) || ((x) == (void *) 0xa5a5a5a5)		\
	    || ((x) == (void *) 0x5a5a5a5a))				\
	{								\
	    fprintf(stderr, "%s:%u:%s(): Invalid pointer: %s (%p)\n",	\
		    __FILE__, __LINE__, __FUNCTION__, #x, (x));		\
	    abort();							\
	}								\
    } while (0)
#else
#define cw_not_reached()
#define cw_assert(a)
#define cw_check_ptr(a)
#endif

/* cw_dasssert() is used internally in places that the assertion should only
 * be made if CW_DBG is defined, such as checking magic variables that only
 * exist in that case. */
#if (defined(CW_DBG) && defined(CW_ASSERT))
#define cw_dassert(a)							\
    do									\
    {									\
	if (!(a))							\
	{								\
	    fprintf(stderr, "%s:%u:%s(): Failed assertion: \"%s\"\n",	\
		    __FILE__, __LINE__, __FUNCTION__, #a);		\
	    abort();							\
	}								\
    } while (0)
#else
#define cw_dassert(a)
#endif

/* Convenience macro for determining the offset of a field within a
 * structure. */
#define cw_offsetof(a_type, a_field)					\
    ((cw_uint32_t) &(((a_type *)NULL)->a_field))

#include "nxn.h"
#include "nxo.h"
#include "nxo_no.h"
#include "nxo_array.h"
#include "nxo_boolean.h"
#ifdef CW_THREADS
#include "nxo_condition.h"
#endif
#include "nxo_dict.h"
#include "nxo_file.h"
#include "nxo_fino.h"
#include "nxo_hook.h"
#include "nxo_integer.h"
#include "nxo_mark.h"
#ifdef CW_THREADS
#include "nxo_mutex.h"
#endif
#include "nxo_name.h"
#include "nxo_null.h"
#include "nxo_operator.h"
#include "nxo_pmark.h"
#ifdef CW_REAL
#include "nxo_real.h"
#endif
#include "nxa.h"
#include "nxo_stack.h"
#include "nxo_string.h"
#include "nxo_thread.h"
#include "nx.h"
#include "systemdict.h"
#include "gcdict.h"

/* Convenience macro for static embedded onyx code. */
#define cw_onyx_code(a_thread, a_code)					\
    do									\
    {									\
	cw_nxo_threadp_t threadp;					\
	static const cw_uint8_t	code[] = (a_code);			\
									\
	nxo_threadp_new(&threadp);					\
	nxo_thread_interpret((a_thread), &threadp, code,		\
			     sizeof(code) - 1);				\
	nxo_thread_flush((a_thread), &threadp);				\
	nxo_threadp_delete(&threadp, (a_thread));			\
    } while (0)

#endif	/* CW_LIBONYX_H_ */

#ifdef __cplusplus
};
#endif
