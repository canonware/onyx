/******************************************************************************
 *
 * <Copyright = jasone>
 * <License>
 *
 ******************************************************************************
 *
 * Version: <Version>
 *
 ******************************************************************************/

#include "slate_defs.h"

#include <libonyx/libonyx.h>

#include <sys/param.h>
#include <curses.h>
#include <panel.h>

#include "slate_ops.h"
#include "funnel.h"
#include "buf.h"
#include "hist.h"

#include "buffer.h"
#include "display.h"
#include "frame.h"
#include "window.h"

#ifdef WORDS_BIGENDIAN
#define _cw_ntohq(a) (a)
#define _cw_htonq(a) (a)
#else
#define _cw_ntohq(a)							\
	(cw_uint64_t) (((cw_uint64_t) (ntohl((cw_uint32_t) ((a) >>	\
	    32)))) | (((cw_uint64_t) (ntohl((cw_uint32_t) ((a) &	\
	    0x00000000ffffffff)))) << 32))
#define _cw_htonq(a)							\
        (cw_uint64_t) (((cw_uint64_t) (htonl((cw_uint32_t) ((a) >>	\
            32)))) | (((cw_uint64_t) (htonl((cw_uint32_t) ((a) &	\
            0x00000000ffffffff)))) << 32))
#endif
