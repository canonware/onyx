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

#include <libonyx/libonyx.h>

#define GTK_DISABLE_DEPRECATED
#include <gtk/gtk.h>

#define MODGTK_ENTRY(name) {#name, modgtk_##name}

struct cw_modgtk_entry
{
    const cw_uint8_t *name;
    cw_nxo_hook_eval_t *eval_f;
};

void
modgtk_hooks_init(cw_nxo_t *a_thread, const struct cw_modgtk_entry *a_entries,
		  cw_uint32_t a_nentries);
