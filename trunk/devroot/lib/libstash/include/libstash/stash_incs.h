/* -*- mode: c ; c-file-style: "canonware-c-style" -*-
 ****************************************************************************
 *
 * <Copyright = "jasone">
 * <License>
 *
 ****************************************************************************
 *
 * Version: <Version>
 *
 * <<< Description >>>
 *
 * The idea here is to keep cpp from having to process a header file more
 * than once, and to capture class dependencies so that it isn't necessary
 * to manually include multiple headers just to use one class.
 *
 ****************************************************************************/

/* 
 * System headers to always be included.
 */

#ifndef _STDIO_H_
#  include <stdio.h>
#  define _STDIO_H_
#endif
 
#ifndef _STDLIB_H_
#  include <stdlib.h>
#  define _STDLIB_H_
#endif
 
#ifndef _UNISTD_H_
#  include <unistd.h>
#  define _UNISTD_H_
#endif
 
#ifndef _STRING_H_
#  include <string.h>
#  define _STRING_H_
#endif
 
#ifndef _STRINGS_H_
#  include <strings.h>
#  define _STRINGS_H_
#endif

/*
 * Always include these once per run.
 */

#ifdef _CW_REENTRANT
#  ifndef _STASH_USE_THREAD
#    define _STASH_USE_THREAD
#  endif
#endif
 
#ifdef _CW_REENTRANT
#  ifndef _STASH_USE_LOCKS
#    define _STASH_USE_LOCKS
#  endif
#endif
 
#ifndef _STASH_USE_DBG
#  define _STASH_USE_DBG
#endif
 
#ifndef _STASH_USE_LOG
#  define _STASH_USE_LOG
#endif
 
#ifndef _STASH_USE_MEM
#  define _STASH_USE_MEM
#endif

/*
 * Define dependencies between the headers.
 */

#ifdef _STASH_USE_BUF
#  ifndef _STASH_USE_LIST
#    define _STASH_USE_LIST
#  endif
#endif
 
#ifdef _STASH_USE_OH
#  ifndef _STASH_USE_LIST
#    define _STASH_USE_LIST
#  endif
#endif
 
#ifdef _STASH_USE_LOCKS
#  ifndef _STASH_USE_LIST
#    define _STASH_USE_LIST
#  endif
#endif
 
#ifdef _STASH_USE_DBG
#  ifndef _STASH_USE_LIST
#    define _STASH_USE_LIST
#  endif
#  ifndef _STASH_USE_OH
#    define _STASH_USE_OH
#  endif
#endif
 
#ifdef _STASH_USE_LEX
#  ifndef _STASH_USE_LIST
#    define _STASH_USE_LIST
#  endif
#  ifndef _STASH_USE_BUF
#    define _STASH_USE_BUF
#  endif
#endif
 
#ifdef _STASH_USE_RES
#  ifndef _STASH_USE_OH
#    define _STASH_USE_OH
#  endif
#endif

/*
 * Include files.  These must be listed in reverse dependency order (for
 * example, list.h must come before oh.h
 */

#ifdef _STASH_USE_THREAD
#  ifndef _THREAD_H_
#    include "thread.h"
#    define _THREAD_H_
#  endif
#endif
 
#ifdef _STASH_USE_LIST
#  ifndef _LIST_H_
#    include "list.h"
#    define _LIST_H_
#  endif
#endif
 
#ifdef _STASH_USE_LOCKS
#  ifndef _LOCKS_H_
#    include "locks.h"
#    define _LOCKS_H_
#  endif
#endif
 
#ifdef _STASH_USE_BUF
#  ifndef _BUF_H_
#    include "buf.h"
#    define _BUF_H_
#  endif
#endif
 
#ifdef _STASH_USE_OH
#  ifndef _OH_H_
#    include "oh.h"
#    define _OH_H_
#  endif
#endif
 
#ifdef _STASH_USE_DBG
#  ifndef _DBG_H_
#    include "dbg.h"
#    define _DBG_H_
#  endif
#endif
 
#ifdef _STASH_USE_LOG
#  ifndef _LOG_H_
#    include "log.h"
#    define _LOG_H_
#  endif
#endif
 
#ifdef _STASH_USE_MEM
#  ifndef _MEM_H_
#    include "mem.h"
#    define _MEM_H_
#  endif
#endif
 
#ifdef _STASH_USE_LEX
#  ifndef _LEX_H_
#    include "lex.h"
#    define _LEX_H_
#  endif
#endif
 
#ifdef _STASH_USE_BHP
#  ifndef _BHP_H_
#    include "bhp.h"
#    define _BHP_H_
#  endif
#endif
 
#ifdef _STASH_USE_GLOB
#  ifndef _GLOB_H_
#    include "glob.h"
#    define _GLOB_H_
#  endif
#endif
 
#ifdef _STASH_USE_MATRIX
#  ifndef _MATRIX_H_
#    include "matrix.h"
#    define _MATRIX_H_
#  endif
#endif
 
#ifdef _STASH_USE_RES
#  ifndef _RES_H_
#    include "res.h"
#    define _RES_H_
#  endif
#endif
 
#ifdef _STASH_USE_TREEN
#  ifndef _TREEN_H_
#    include "treen.h"
#    define _TREEN_H_
#  endif
#endif
