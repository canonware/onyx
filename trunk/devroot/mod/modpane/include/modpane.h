/* -*- mode: c ; c-file-style: "canonware-c-style" -*-
 ******************************************************************************
 *
 * <Copyright = jasone>
 * <License>
 *
 ******************************************************************************
 *
 * Version: <Version>
 *
 ******************************************************************************/

#include "modpane_defs.h"

#include <libonyx/libonyx.h>

#include "ds.h"
#include "cl.h"
#include "pn.h"

#include "display.h"
#include "cell.h"
#include "pane.h"

#define MODPANE_ENTRY(name) {#name, modpane_##name}

struct cw_modpane_entry
{
    const cw_uint8_t *name;
    cw_nxo_hook_eval_t *eval_f;
};

void
modpane_hooks_init(cw_nxo_t *a_thread, const struct cw_modpane_entry *a_entries,
		   cw_uint32_t a_nentries);
