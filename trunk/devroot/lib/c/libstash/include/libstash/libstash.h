/* -*- mode: c ; c-file-style: "canonware-c-style" -*-
 ****************************************************************************
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
extern "C" {
#endif

#ifndef _LIBSTASH_H_A_
#  define _LIBSTASH_H_A_

#  define _LIBSTASH_VERSION_ <Version>

#  include "libstash_defs.h"

/*
 * Global typedefs.
 */

#  if (SIZEOF_SIGNED_CHAR == 1)
#    define _TYPE_SINT8_DEFINED
typedef signed char cw_sint8_t;
#  endif

#  if (SIZEOF_SIGNED_CHAR == 1)
#    define _TYPE_UINT8_DEFINED
typedef unsigned char cw_uint8_t;
#  endif

#  if (SIZEOF_SIGNED_SHORT == 2)
#    define _TYPE_SINT16_DEFINED
typedef signed short cw_sint16_t;
#  endif

#  if (SIZEOF_UNSIGNED_SHORT == 2)
#    define _TYPE_UINT16_DEFINED
typedef unsigned short cw_uint16_t;
#  endif

#  if (SIZEOF_INT == 4)
#    define _TYPE_SINT32_DEFINED
typedef int cw_sint32_t;
#  endif

#  if (SIZEOF_UNSIGNED == 4)
#    define _TYPE_UINT32_DEFINED
typedef unsigned cw_uint32_t;
#  endif

#  if (SIZEOF_LONG == 8)
#    define _TYPE_SINT64_DEFINED
typedef long cw_sint64_t;
#  endif

#  if (SIZEOF_UNSIGNED_LONG == 8)
#    define _TYPE_UINT64_DEFINED
typedef unsigned long cw_uint64_t;
#  endif

#  if (SIZEOF_LONG_LONG == 8)
#    define _TYPE_SINT64_DEFINED
typedef long long cw_sint64_t;
#  endif

#  if (SIZEOF_UNSIGNED_LONG_LONG == 8)
#    define _TYPE_UINT64_DEFINED
typedef unsigned long long cw_uint64_t;
#  endif

#  if (!defined(_TYPE_SINT8_DEFINED) || !defined(_TYPE_UINT8_DEFINED) \
  || !defined(_TYPE_SINT16_DEFINED) || !defined(_TYPE_UINT16_DEFINED) \
  || !defined(_TYPE_SINT32_DEFINED) || !defined(_TYPE_UINT32_DEFINED) \
  || !defined(_TYPE_SINT64_DEFINED) || !defined(_TYPE_UINT64_DEFINED))
#    error "Lacking mandatory typedefs"
#  endif

/* Grossness to make sure things still work, even if TRUE and/or FALSE are/is
 * defined. */
#  ifdef FALSE
#    define _CW_FALSE_DEFINED
#    undef FALSE
#  endif
#  ifdef TRUE
#    define _CW_TRUE_DEFINED
#    undef TRUE
#  endif
  
typedef enum
{
  FALSE,
  TRUE
} cw_bool_t;

/* More grossness to make sure things still work, even if TRUE and/or FALSE
 * are/is defined. */
#  ifdef _CW_FALSE_DEFINED
#    define FALSE (0)
#    undef _CW_FALSE_DEFINED
#  endif
#  ifdef _CW_TRUE_DEFINED
#    define TRUE (1)
#    undef _CW_TRUE_DEFINED
#  endif
  
/*
 * Project headers to always be included.
 */

#endif /* _LIBSTASH_H_A_ */

/* This needs to be done every time this file is included, in case both
 * libstash.h and, say, libsqrl.h are included.  Unfortunately, it has to be in
 * the middle of this file because what follows depends on stuff that is
 * included in/by stash_incs.h. */
#include "libstash_incs.h"

#ifndef _LIBSTASH_H_B_
#  define _LIBSTASH_H_B_

/*
 * libstash initialization and shutdown function prototypes.
 */
cw_bool_t
libstash_init(void);
  
void
libstash_shutdown(void);
  
/*
 * Global variables.
 */
extern cw_mem_t * cw_g_mem;
extern cw_dbg_t * cw_g_dbg;
extern cw_log_t * cw_g_log;

/*
 * Global macros we use everywhere.
 */
#if (defined(_LIBSTASH_DBG) || defined(_LIBSTASH_DEBUG))
#  define _cw_malloc(a) mem_malloc(cw_g_mem, a, __FILE__, __LINE__)
#  define _cw_calloc(a, b) mem_calloc(cw_g_mem, a, b, __FILE__, __LINE__)
#  define _cw_realloc(a, b) mem_realloc(cw_g_mem, a, b, __FILE__, __LINE__)
#  define _cw_free(a) {mem_free(cw_g_mem, a, __FILE__, __LINE__); (a) = NULL;}
#  define _cw_dealloc(a) {mem_dealloc((void *) cw_g_mem, a, \
                                      __FILE__, __LINE__); (a) = NULL;}
#else
#  define _cw_malloc(a) mem_malloc(cw_g_mem, a)
#  define _cw_calloc(a, b) mem_calloc(cw_g_mem, a, b)
#  define _cw_realloc(a, b) mem_realloc(cw_g_mem, a, b)
#  define _cw_free(a) mem_free(cw_g_mem, a)
#  define _cw_dealloc(a) mem_dealloc((void *) cw_g_mem, a)
#endif

#ifdef WORDS_BIGENDIAN
#  define _cw_ntohq(a) (a)
#  define _cw_htonq(a) (a)
#else
#  define _cw_ntohq(a) (cw_uint64_t) \
                       (((cw_uint64_t) \
                          (ntohl((cw_uint32_t) ((a) >> 32)))) \
			| \
                       (((cw_uint64_t) \
                          (ntohl((cw_uint32_t) ((a) & 0x00000000ffffffff)))) \
                           << 32))
#  define _cw_htonq(a) (cw_uint64_t) \
                       (((cw_uint64_t) \
                          (htonl((cw_uint32_t) ((a) >> 32)))) \
			| \
                       (((cw_uint64_t) \
                          (htonl((cw_uint32_t) ((a) & 0x00000000ffffffff)))) \
                                << 32))
#endif

#endif /* _LIBSTASH_H_B_ */

#ifdef __cplusplus
};
#endif
