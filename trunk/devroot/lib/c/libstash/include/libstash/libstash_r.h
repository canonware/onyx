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
 * Description: Master header file for libstash_r.
 *
 ****************************************************************************/

/* Must be defined for pthreads. */
#ifndef _REENTRANT
#  define _REENTRANT
#endif

/* This is used to determine whether to act as though linking against
   libstash_r. */
#ifndef _CW_REENTRANT
#  define _CW_REENTRANT
#endif

#include "libstash.h"
