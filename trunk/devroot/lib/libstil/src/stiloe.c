/****************************************************************************
 *
 * <Copyright = "jasone">
 * <License>
 *
 ****************************************************************************
 *
 * Version: <Version>
 *
 ****************************************************************************/

#include "../include/libstil/libstil.h"

#ifdef _LIBSTIL_DBG
#define _CW_STILOE_MAGIC 0x0fa6e798
#endif

void
stiloe_new(cw_stiloe_t *a_stiloe, cw_stilt_t *a_stilt, cw_stilot_t a_type)
{
	memset(a_stiloe, 0, sizeof(cw_stiloe_t));

	a_stiloe->type = a_type;
	a_stiloe->stilt = a_stilt;

#ifdef _LIBSTIL_DBG
	a_stiloe->magic = _CW_STILOE_MAGIC;
#endif
}

void
stiloe_delete(cw_stiloe_t *a_stiloe)
{
	/* XXX */
}

void
stiloe_gc_register(cw_stiloe_t *a_stiloe)
{
	/* XXX */
}
