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
 ****************************************************************************/

/*
 * System headers to always be included.
 */


/*
 * Include libstash.h now so that any dependencies on its classes are
 * satisfied before continuing on.
 */

#include <libstash/libstash.h>
#include <libsock/libsock.h>

/*
 * Include files.  These must be listed in reverse dependency order (for
 * example, sock.h must come before socks.h.
 */

#include "kasi.h"
#include "kasit.h"
