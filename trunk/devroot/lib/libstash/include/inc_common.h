/* -*-mode:c-*-
 ****************************************************************************
 *
 * Copyright (c) 1996-1998
 * Jason Evans <jasone@canonware.com>.  All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer as
 *    the first lines of this file unmodified.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY JASON EVANS ``AS IS'' AND ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
 * OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
 * IN NO EVENT SHALL JASON EVANS BE LIABLE FOR ANY DIRECT, INDIRECT,
 * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT
 * NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 * DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 * THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF
 * THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 ****************************************************************************
 *
 * $Source$
 * $Author: jasone $
 * Current revision: $Revision: 7 $
 * Last modified: $Date: 1998-01-16 00:20:54 -0800 (Fri, 16 Jan 1998) $
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
 * The main header for the whole project.
 */

#ifndef _CONFIG_H_
#  include <config.h>
#  define _CONFIG_H_
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
