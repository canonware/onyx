/* -*- mode: c ; c-file-style: "canonware-c-style" -*-
 ******************************************************************************
 *
 * <Copyright = jasone>
 * <License>
 *
 ******************************************************************************
 *
 * Version: Slate <Version = slate>
 *
 ******************************************************************************/

#include <libonyx/libonyx.h>
#include "modslate_defs.h"

#include <curses.h>
#include <panel.h>

#include "rb.h"
#include "buf.h"
#include "hist.h"

#include "buffer.h"
#include "display.h"
#include "frame.h"
#include "window.h"
#include "funnel.h"

/* Iteration counts for destruction of various object types.  Dependencies:
 *
 * buffer  : marker extent
 * display : frame window
 * funnel  : display frame
 * module  : buffer funnel
 */
#define MODSLATE_GC_ITER_BUFFER	1
#define MODSLATE_GC_ITER_DISPLAY 1
#define MODSLATE_GC_ITER_FUNNEL	2
#define MODSLATE_GC_ITER_MODULE	3

#define MODSLATE_ENTRY(name) {#name, modslate_##name}

struct cw_modslate_entry
{
    const cw_uint8_t *name;
    cw_nxo_hook_eval_t *eval_f;
};

/* Code funnel for curses API calls.  This funnel is special in that it is
 * referred to by all hooks that are created via modslate_hooks_init().  As
 * such, it cannot be deleted until the same GC sweep that unloads the module.
 * This makes it safe to use modslate_funnel_c_{enter,leave}() in the hook
 * destructors. */
extern cw_nxo_t modslate_curses_funnel;

void
modslate_hooks_init(cw_nxo_t *a_thread,
		    const struct cw_modslate_entry *a_entries,
		    cw_uint32_t a_nentries);

cw_nxn_t
modslate_hook_type(cw_nxo_t *a_hook, const cw_uint8_t *a_type);

void
modslate_hook_p(void *a_data, cw_nxo_t *a_thread, const cw_uint8_t *a_type);

void
modslate_init(void *a_arg, cw_nxo_t *a_thread);
