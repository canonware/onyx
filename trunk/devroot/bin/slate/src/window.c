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

#include "../include/modslate.h"

static const struct cw_slate_entry slate_window_ops[] = {

};

void
slate_window_init(cw_nxo_t *a_thread)
{
	slate_hooks_init(a_thread, slate_window_ops,
	    (sizeof(slate_window_ops) / sizeof(struct cw_slate_entry)));
}
