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

#define MODSLATE_ENTRY(name) {#name, modslate_##name}

struct cw_modslate_entry
{
    const cw_uint8_t *name;
    cw_nxo_hook_eval_t *eval_f;
};

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
