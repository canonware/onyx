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

#  ifndef _STDIO_H_
#    include <stdio.h>
#    define _STDIO_H_
#  endif

#  ifndef _STDLIB_H_
#    include <stdlib.h>
#    define _STDLIB_H_
#  endif

#  ifndef _UNISTD_H_
#    include <unistd.h>
#    define _UNISTD_H_
#  endif

#  ifndef _STRING_H_
#    include <string.h>
#    define _STRING_H_
#  endif

#  ifndef _STRINGS_H_
#    include <strings.h>
#    define _STRINGS_H_
#  endif

/*
 * Always include these once per run.
 */

#  ifdef _CW_REENTRANT
#    ifndef _INC_THREAD_H_
#      define _INC_THREAD_H_
#    endif
#  endif

#  ifdef _CW_REENTRANT
#    ifndef _INC_LOCKS_H_
#      define _INC_LOCKS_H_
#    endif
#  endif

#  ifndef _INC_DBG_H_
#    define _INC_DBG_H_
#  endif

#  ifndef _INC_LOG_H_
#    define _INC_LOG_H_
#  endif

#  ifndef _INC_MEM_H_
#    define _INC_MEM_H_
#  endif

/*
 * Define dependencies between the headers.
 */

#  ifdef _INC_BUF_H_
#    ifndef _INC_LIST_H_
#      define _INC_LIST_H_
#    endif
#  endif

#  ifdef _INC_OH_H_
#    ifndef _INC_LIST_H_
#      define _INC_LIST_H_
#    endif
#  endif

#  ifdef _INC_LOCKS_H_
#    ifndef _INC_LIST_H_
#      define _INC_LIST_H_
#    endif
#  endif

#  ifdef _INC_DBG_H_
#    ifndef _INC_LIST_H_
#      define _INC_LIST_H_
#    endif
#    ifndef _INC_OH_H_
#      define _INC_OH_H_
#    endif
#  endif

#  ifdef _INC_LEX_H_
#    ifndef _INC_LIST_H_
#      define _INC_LIST_H_
#    endif
#    ifndef _INC_BUF_H_
#      define _INC_BUF_H_
#    endif
#  endif

#  ifdef _INC_RES_H_
#    ifndef _INC_OH_H_
#      define _INC_OH_H_
#    endif
#  endif

#  ifdef _INC_TREE_H_
#    ifndef _INC_LIST_H_
#      define _INC_LIST_H_
#    endif
#  endif

/*
 * Include files.  These must be listed in reverse dependency order (for
 * example, list.h must come before oh.h
 */

#  ifdef _INC_THREAD_H_
#    ifndef _THREAD_H_
#      include "thread.h"
#      define _THREAD_H_
#    endif
#  endif

#  ifdef _INC_LIST_H_
#    ifndef _LIST_H_
#      include "list.h"
#      define _LIST_H_
#    endif
#  endif

#  ifdef _INC_LOCKS_H_
#    ifndef _LOCKS_H_
#      include "locks.h"
#      define _LOCKS_H_
#    endif
#  endif

#  ifdef _INC_BUF_H_
#    ifndef _BUF_H_
#      include "buf.h"
#      define _BUF_H_
#    endif
#  endif

#  ifdef _INC_OH_H_
#    ifndef _OH_H_
#      include "oh.h"
#      define _OH_H_
#    endif
#  endif

#  ifdef _INC_DBG_H_
#    ifndef _DBG_H_
#      include "dbg.h"
#      define _DBG_H_
#    endif
#  endif

#  ifdef _INC_LOG_H_
#    ifndef _LOG_H_
#      include "log.h"
#      define _LOG_H_
#    endif
#  endif

#  ifdef _INC_MEM_H_
#    ifndef _MEM_H_
#      include "mem.h"
#      define _MEM_H_
#    endif
#  endif

#  ifdef _INC_LEX_H_
#    ifndef _LEX_H_
#      include "lex.h"
#      define _LEX_H_
#    endif
#  endif

#  ifdef _INC_BHP_H_
#    ifndef _BHP_H_
#      include "bhp.h"
#      define _BHP_H_
#    endif
#  endif

#  ifdef _INC_GLOB_H_
#    ifndef _GLOB_H_
#      include "glob.h"
#      define _GLOB_H_
#    endif
#  endif

#  ifdef _INC_MATRIX_H_
#    ifndef _MATRIX_H_
#      include "matrix.h"
#      define _MATRIX_H_
#    endif
#  endif

#  ifdef _INC_RES_H_
#    ifndef _RES_H_
#      include "res.h"
#      define _RES_H_
#    endif
#  endif

#  ifdef _INC_TREE_H_
#    ifndef _TREE_H_
#      include "tree.h"
#      define _TREE_H_
#    endif
#  endif
