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
stiloe_new(cw_stiloe_t *a_stiloe)
{
	memset(a_stiloe, sizeof(cw_stiloe_t), 0);
#ifdef _LIBSTIL_DBG
	a_stiloe->magic = _CW_STILOE_MAGIC;
#endif
}

void
stiloe_delete(cw_stiloe_t *a_stiloe)
{
	/* XXX */
}
