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

#include <pcre.h>

#define MODPCRE_ENTRY(name) {#name, modpcre_##name}

struct cw_modpcre_entry
{
    const cw_uint8_t *name;
    cw_nxo_hook_eval_t *eval_f;
};

void
modpcre_hooks_init(cw_nxo_t *a_thread, const struct cw_modpcre_entry *a_entries,
		   cw_uint32_t a_nentries);
