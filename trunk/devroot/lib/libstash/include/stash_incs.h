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

/*
 * Always include these once per run.
 */

#ifndef _THREAD_H_
#  include <thread.h>
#  define _THREAD_H_
#endif

#ifndef _LOCKS_H_
#  ifndef _LIST_H_
#    include <list.h>
#    define _LIST_H_
#  endif
#  include <locks.h>
#  define _LOCKS_H_
#endif

#ifndef _DBG_H_
#  include <dbg.h>
#  define _DBG_H_
#endif

#ifndef _LOG_H_
#  include <log.h>
#  define _LOG_H_
#endif

#ifndef _MEM_H_
#  include <mem.h>
#  define _MEM_H_
#endif

/*
 * Other project headers we don't always want to include.
 */

#if (defined(_INC_BHP_H_))
#  ifndef _BHP_H_
#    include <bhp.h>
#    define _BHP_H_
#  endif
#endif

#if (defined(_INC_BR_H_))
#  ifndef _BR_H_
#    ifndef _OH_H_
#      include <oh.h>
#    endif
#    ifndef _RES_H_
#      include <res.h>
#    endif
#    ifndef _BRBLK_H_
#      include <brblk.h>
#    endif
#    ifndef _BRBS_H_
#      include <brbs.h>
#    endif
#    include <br.h>
#    define _BR_H_
#  endif
#endif

#if (defined(_INC_BRBLK_H_))
#  ifndef _BRBLK_H_
#    include <brblk.h>
#    define _BRBLK_H_
#  endif
#endif

#if (defined(_INC_BRBS_H_))
#  ifndef _BRBS_H_
#    ifndef _BRBLK_H_
#      include <brblk.h>
#    endif
#    include <brbs.h>
#    define _BRBS_H_
#  endif
#endif

#if (defined(_INC_BRF_H_))
#  ifndef _BRF_H_
#    ifndef _BRBLK_H_
#      include <brblk.h>
#    endif
#    ifndef _BRBS_H_
#      include <brbs.h>
#    endif
#    include <brf.h>
#    define _BRF_H_
#  endif
#endif

#if (defined(_INC_BUF_H_))
#  ifndef _BUF_H_
#    ifndef _LIST_H_
#      include <list.h>
#    endif
#    include <buf.h>
#    define _BUF_H_
#  endif
#endif

#if (defined(_INC_GLOB_H_))
#  ifndef _GLOB_H_
#    include <glob.h>
#    define _GLOB_H_
#  endif
#endif

#if (defined(_INC_BT_H_))
#  ifndef _JT_H_
#    include <jt.h>
#    define _JT_H_
#  endif
#endif

#if (defined(_INC_LIST_H_))
#  ifndef _LIST_H_
#    include <list.h>
#    define _LIST_H_
#  endif
#endif

#if (defined(_INC_MATRIX_H_))
#  ifndef _MATRIX_H_
#    include <matrix.h>
#    define _MATRIX_H_
#  endif
#endif

#if (defined(_INC_OH_H_))
#  ifndef _OH_H_
#    include <oh.h>
#    define _OH_H_
#  endif
#endif

#if (defined(_INC_RES_H_))
#  ifndef _RES_H_
#    ifndef _OH_H_
#      include <oh.h>
#      define _OH_H_
#    endif
#    include <res.h>
#    define _RES_H_
#  endif
#endif

#if (defined(_INC_SOCK_H_))
#  ifndef _SOCK_H_
#    ifndef _BUF_H_
#      include <buf.h>
#      define _BUF_H_
#    endif
#    include <sock.h>
#    define _SOCK_H_
#  endif
#endif

#if (defined(_INC_SOCKS_H_))
#  ifndef _SOCKS_H_
#    ifndef _SOCK_H_
#      ifndef _BUF_H_
#        include <buf.h>
#        define _BUF_H_
#      endif
#      include <sock.h>
#      define _SOCK_H_
#    endif
#    include <socks.h>
#    define _SOCKS_H_
#  endif
#endif

#if (defined(_INC_ZT_H_))
#  ifndef _ZT_H_
#    include <zt.h>
#    define _ZT_H_
#  endif
#endif

/* Skeleton */
#if (0)

#if (defined(_INC_?_H_))
#  ifndef _?_H_
#    include <?.h>
#    define _?_H_
#  endif
#endif

#endif
