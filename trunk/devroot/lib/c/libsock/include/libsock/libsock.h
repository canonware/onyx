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
 * Description: Master header file for libstash.
 *
 ****************************************************************************/

#ifdef __cplusplus
extern "C" {
#endif

#ifndef _LIBSOCK_H_
#  define _LIBSOCK_H_

#  define _LIBSOCK_VERSION_ <Version>

/* This guarantees that the socket code can handle something over 1000
 * simultaneous clients, rather than the default of under 256 as on some
 * systems. */
#  ifdef FD_SETSIZE
#    undef FD_SETSIZE
#  endif
#  define FD_SETSIZE 1024

/* Need to include libstash_r.h here for the namespace mangling foo.  It gets
 * included again inside libsock_incs.h.  Oh well, that's okay. */
#  include <libstash/libstash_r.h>

/* Namespace setup. */

#  define _CW_LIBSOCK_PREFIX _libsock_
#  define _CW_NS_LIBSOCK(x) _cw_ns_xglue(_CW_LIBSOCK_PREFIX,,x)

/* Project headers to always be included. */

#  include "libsock_incs.h"

#endif /* _LIBSOCK_H_ */

#ifdef __cplusplus
};
#endif

