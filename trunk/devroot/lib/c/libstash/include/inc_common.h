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
 * Current revision: $Revision: 3 $
 * Last modified: $Date: 1997-12-14 22:01:05 -0800 (Sun, 14 Dec 1997) $
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

#ifndef _RESOURCE_H_
#  include <resource.h>
#  define _RESOURCE_H_
#endif

/* 
 * Other project headers we don't always want to include.
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
