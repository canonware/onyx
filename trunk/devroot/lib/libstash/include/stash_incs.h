/* -*-mode:c-*-
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
 * Don't wrap this file, because it needs to be re-entrant since we're not
 * automatically including everything.
 *
 * XXX Is this still true?
 */

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
#    ifndef _THREAD_H_
#      include <thread.h>
#      define _THREAD_H_
#    endif
#  endif

#  ifdef _CW_REENTRANT
#    ifndef _LOCKS_H_
#      ifndef _LIST_H_
#        include <list.h>
#        define _LIST_H_
#      endif
#      include <locks.h>
#      define _LOCKS_H_
#    endif
#  endif

#  ifndef _DBG_H_
#    include <dbg.h>
#    define _DBG_H_
#  endif

#  ifndef _LOG_H_
#    include <log.h>
#    define _LOG_H_
#  endif

#  ifndef _MEM_H_
#    include <mem.h>
#    define _MEM_H_
#  endif

/*
 * Other project headers we don't always want to include.
 */

#  if (defined(_INC_BHP_H_))
#    ifndef _BHP_H_
#      include <bhp.h>
#      define _BHP_H_
#    endif
#  endif

#  if (defined(_INC_BUF_H_))
#    ifndef _BUF_H_
#      ifndef _LIST_H_
#        include <list.h>
#      endif
#      include <buf.h>
#      define _BUF_H_
#    endif
#  endif

#  if (defined(_INC_GLOB_H_))
#    ifndef _GLOB_H_
#      include <glob.h>
#      define _GLOB_H_
#    endif
#  endif

#  if (defined(_INC_LEX_H_))
#    ifndef _LEX_H_
#      ifndef _BUF_H_
#        ifndef _LIST_H_
#          include <list.h>
#        endif
#        include <buf.h>
#        define _BUF_H_
#      endif
#      include <lex.h>
#      define _LEX_H_
#    endif
#  endif

#  if (defined(_INC_LIST_H_))
#    ifndef _LIST_H_
#      include <list.h>
#      define _LIST_H_
#    endif
#  endif

#  if (defined(_INC_MATRIX_H_))
#    ifndef _MATRIX_H_
#      include <matrix.h>
#      define _MATRIX_H_
#    endif
#  endif

#  if (defined(_INC_OH_H_))
#    ifndef _OH_H_
#      ifndef _LIST_H_
#        include <list.h>
#        define _LIST_H_
#      endif
#      include <oh.h>
#      define _OH_H_
#    endif
#  endif

#  if (defined(_INC_RES_H_))
#    ifndef _RES_H_
#      ifndef _OH_H_
#        include <oh.h>
#        define _OH_H_
#      endif
#      include <res.h>
#      define _RES_H_
#    endif
#  endif

/* Skeleton */
#  if (0)

#  if (defined(_INC_?_H_))
#    ifndef _?_H_
#      include <?.h>
#      define _?_H_
#    endif
#  endif

#endif
