/* -*-mode:c-*-
 ****************************************************************************
 *
 * Copyright (C) 1996-1997 Jason Evans <jasone@canonware.com>
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the Free
 * Software Foundation; either version 2 of the License, or (at your option)
 * any later version.
 * 
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY
 * or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License
 * for more details.
 *
 * You can get a copy of the GNU General Public License at
 * http://www.fsf.org/copyleft/gpl.html, or by writing to the Free Software
 * Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 *
 ****************************************************************************
 *
 * $Source$
 * $Author: jasone $
 * Current revision: $Revision: 2 $
 * Last modified: $Date: 1997-12-07 17:34:03 -0800 (Sun, 07 Dec 1997) $
 *
 * Description: The idea here is to keep cpp from having to process a header
 *              file more than once.  The tradeoff is that every header gets
 *              processed once.  If this eventually proves too much overhead,
 *              we can go to a macro definition system like TurboVision uses
 *              to keep from including headers unless we need them.
 *
 ****************************************************************************
 */

/* 
 * Don't wrap this file, because it needs to be re-entrant since we're not
 * automatically including everything.
 */

/*
 * Always include these once per run.
 */
#ifndef _CONFIG_H_
#  include <config.h>
#  define _CONFIG_H_
#endif

#ifndef _LOG_H_
#  include <log.h>
#  define _LOG_H_
#endif

#ifndef _GLOBAL_DEFS_H_
#  include <global_defs.h>
#  define _GLOBAL_DEFS_H_
#endif

#ifndef _STDIO_H_
#  include <stdio.h>
#  define _STDIO_H_
#endif

#ifndef _STDLIB_H_
#  include <stdlib.h>
#  define _STDLIB_H_
#endif

#if (HAVE_UNISTD_H)
#  ifndef _UNISTD_H_
#    include <unistd.h>
#    define _UNISTD_H_
#  endif
#else
#  error "unistd.h not found.  Cannot continue"
#endif

/* 
 * Project headers. 
 */

/* Skeleton */

#if (0)

#if (defined(_INC_?_H_) || defined(_INC_ALL_))
#  ifndef _?_H_
#    include <?.h>
#    define _?_H_
#  endif
#endif

#endif

/* 
 * System headers.
 */
#if (defined(_INC_STDARG_H_) || defined(_INC_ALL_))
#  ifndef _STDARG_H_
#    include <stdarg.h>
#    define _STDARG_H_
#  endif
#endif

#if (defined(_INC_STRING_H_) || defined(_INC_ALL_))
#  ifndef _STRING_H_
#    include <string.h>
#    define _STRING_H_
#  endif
#endif

#if (defined(_INC_SIGNAL_H_) || defined(_INC_ALL_))
#  ifndef _SIGNAL_H_
#    define _SIGNAL_H_
#    include <signal.h>
#  endif
#endif

#if (defined(_INC_SYS_STAT_H_) || defined(_INC_ALL_))
#  ifndef _SYS_STAT_H_
#    define _SYS_STAT_H_
#    include <sys/stat.h>
#  endif
#endif

#if (defined(_INC_GETOPT_H_) || defined(_INC_ALL_))
#  ifndef _GETOPT_H_
#    define _GETOPT_H_
#    include <getopt.h>
#  endif
#endif

#if (HAVE_PTHREAD_H)
#  if (defined(_INC_PTHREAD_H_) || defined(_INC_ALL_))
#    ifndef _PTHREAD_H_
#      include <pthread.h>
#      define _PTHREAD_H_
#    endif
#  endif
#else
#  error "pthread.h not found.  Cannot continue"
#endif

/* Skeleton */
#if (0) 

#if (defined(_INC_?_H_) || defined(_INC_ALL_))
#  ifndef _?_H_
#    define _?_H_
#    include <?.h>
#  endif
#endif

#endif
