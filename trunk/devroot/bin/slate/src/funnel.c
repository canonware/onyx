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

#include "slate.h"

/* Protects entry into the curses library. */
static cw_mtx_t funnel_mtx;

static const struct cw_slate_entry slate_funnel_ops[] = {
	SLATE_ENTRY(funnel_lock),
	SLATE_ENTRY(funnel_unlock)
};

void
slate_funnel_init(cw_nxo_t *a_thread)
{
	slate_ops(a_thread, slate_funnel_ops, (sizeof(slate_funnel_ops) /
	    sizeof(struct cw_slate_entry)));

	mtx_new(&funnel_mtx);
}

void
slate_funnel_shutdown(cw_nxo_t *a_thread)
{
	mtx_delete(&funnel_mtx);
}

/* - funnel_lock - */
void
slate_funnel_lock(cw_nxo_t *a_thread)
{
	mtx_lock(&funnel_mtx);
}

/* - funnel_unlock - */
void
slate_funnel_unlock(cw_nxo_t *a_thread)
{
	mtx_unlock(&funnel_mtx);
}
