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

#include "modslate_defs.h"

#include <libonyx/libonyx.h>

#include <sys/param.h>
#include <curses.h>
#include <panel.h>

#include "buf.h"
#include "hist.h"

#include "buffer.h"
#include "slate.h"
#include "display.h"
#include "frame.h"
#include "window.h"

#define	SLATE_ENTRY(name)	{#name, slate_##name}

struct cw_slate_entry {
	const cw_uint8_t	*name;
	cw_nxo_hook_eval_t	*eval_f;
};

void	slate_hooks_init(cw_nxo_t *a_thread, const struct cw_slate_entry
    *a_entries, cw_uint32_t a_nentries);

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
