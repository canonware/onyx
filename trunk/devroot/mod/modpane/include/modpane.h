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

#include "modpane_defs.h"

#include <libonyx/libonyx.h>

#define	PANE_ENTRY(name)	{#name, pane_##name}

struct cw_pane_entry {
	const cw_uint8_t	*name;
	cw_nxo_hook_eval_t	*eval_f;
};

void	pane_hooks_init(cw_nxo_t *a_thread, const struct cw_pane_entry
    *a_entries, cw_uint32_t a_nentries);
