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

/* Protects entry into the curses library. */
static cw_mtx_t slate_mtx;

static const struct cw_slate_entry slate_slate_ops[] = {
	SLATE_ENTRY(slate_lock),
	SLATE_ENTRY(slate_unlock)
};

void
slate_slate_init(cw_nxo_t *a_thread)
{
	slate_hooks_init(a_thread, slate_slate_ops, (sizeof(slate_slate_ops) /
	    sizeof(struct cw_slate_entry)));

	mtx_new(&slate_mtx);
}

/* - slate_lock - */
void
slate_slate_lock(void *a_data, cw_nxo_t *a_thread)
{
	mtx_lock(&slate_mtx);
}

/* - slate_unlock - */
void
slate_slate_unlock(void *a_data, cw_nxo_t *a_thread)
{
	mtx_unlock(&slate_mtx);
}
