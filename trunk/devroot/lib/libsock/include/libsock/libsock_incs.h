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
 * Define dependencies between the headers.  These must be listed in forward
 * dependency order.
 */

#ifdef _LIBSOCK_USE_SOCKS
#  ifndef _LIBSOCK_USE_SOCK
#    define _LIBSOCK_USE_SOCK
#  endif
#endif

#ifdef _LIBSOCK_USE_SOCKB
#  ifndef _LIBSOCK_USE_SOCK
#    define _LIBSOCK_USE_SOCK
#  endif
#  ifndef _LIBSTASH_USE_BUF
#    define _LIBSTASH_USE_BUF
#  endif
#  ifndef _LIBSTASH_USE_LIST
#    define _LIBSTASH_USE_LIST
#  endif
#endif

#ifdef _LIBSOCK_USE_SOCK
#  ifndef _LIBSOCK_USE_SOCKB
#    define _LIBSOCK_USE_SOCKB
#  endif
#  ifndef _LIBSTASH_USE_BUF
#    define _LIBSTASH_USE_BUF
#  endif
#  ifndef _LIBSTASH_USE_LIST
#    define _LIBSTASH_USE_LIST
#  endif
#endif
 
/*
 * Include libstash_r.h now so that any dependencies on its classes are
 * satisfied before continuing on.
 */

#include <libstash/libstash_r.h>

/*
 * Include files.  These must be listed in reverse dependency order (for
 * example, sock.h must come before socks.h.
 */

#ifdef _LIBSOCK_USE_SOCK
#  ifndef _SOCK_H_
#    include "sock.h"
#    define _SOCK_H_
#  endif
#endif
 
#ifdef _LIBSOCK_USE_SOCKB
#  ifndef _SOCKB_H_
#    include "sockb.h"
#    define _SOCKB_H_
#  endif
#endif
 
#ifdef _LIBSOCK_USE_SOCKS
#  ifndef _SOCKS_H_
#    include "socks.h"
#    define _SOCKS_H_
#  endif
#endif
