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
 * <<< Description >>>
 *
 * Macros that bypass the mem class, to avoid circular dependencies inside
 * libstash.
 *
 ****************************************************************************/

#ifdef _cw_malloc
#  undef _cw_malloc
#endif
#define _cw_malloc(a) malloc(a)

#ifdef _cw_calloc
#  undef _cw_calloc
#endif
#define _cw_calloc(a, b) calloc(a, b)

#ifdef _cw_realloc
#  undef _cw_realloc
#endif
#define _cw_realloc(a, b) realloc(a, b)

#ifdef _cw_free
#  undef _cw_free
#endif
#define _cw_free(a) free(a)

#ifdef _cw_dealloc
#  undef _cw_dealloc
#endif
#define _cw_dealloc(a) free(a)
