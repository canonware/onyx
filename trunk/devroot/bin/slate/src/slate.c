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

static const struct cw_slate_entry slate_slate_ops[] = {

};

void
slate_slate_init(cw_nxo_t *a_thread)
{
	slate_hooks_init(a_thread, slate_slate_ops, (sizeof(slate_slate_ops) /
	    sizeof(struct cw_slate_entry)));
}
