/* -*- mode: c ; c-file-style: "canonware-c-style" -*-
 ******************************************************************************
 *
 * <Copyright = jasone>
 * <License>
 *
 ******************************************************************************
 *
 * Version: Onyx <Version = onyx>
 *
 ******************************************************************************/

#include <libonyx/libonyx.h>
#include "modpane_defs.h"

#include "ds.h"
#include "pn.h"

#include "display.h"
#include "pane.h"

#define MODPANE_ENTRY(name) {#name, modpane_##name}

struct cw_modpane_entry
{
    const cw_uint8_t *name;
    cw_nxo_hook_eval_t *eval_f;
};

void
modpane_hooks_init(cw_nxo_t *a_thread,
		   const struct cw_modpane_entry *a_entries,
		   cw_uint32_t a_nentries);

void
modpane_init(void *a_arg, cw_nxo_t *a_thread);
