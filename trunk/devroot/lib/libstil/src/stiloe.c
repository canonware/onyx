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

void	stiloe_l_string_init(cw_stiloe_t *a_stiloe);
void	stiloe_l_string_delete(cw_stiloe_t *a_stiloe);

cw_stiloe_t *
stiloe_new(cw_stilt_t *a_stilt, cw_stilot_t a_type)
{
	cw_stiloe_t	*retval;

	switch (a_type) {
	case _CW_STILOT_ARRAYTYPE:
	case _CW_STILOT_CONDITIONTYPE:
	case _CW_STILOT_DICTTYPE:
	case _CW_STILOT_HOOKTYPE:
	case _CW_STILOT_LOCKTYPE:
	case _CW_STILOT_MSTATETYPE:
	case _CW_STILOT_NUMBERTYPE:
	case _CW_STILOT_OPERATORTYPE:
		/* XXX */
		_cw_error("Programming error");
	case _CW_STILOT_STRINGTYPE:
		retval = (cw_stiloe_t *)_cw_stilt_malloc(a_stilt,
		    sizeof(cw_stiloe_string_t));
		stiloe_l_string_init(retval);
		break;
	case _CW_STILOT_NOTYPE:
	case _CW_STILOT_BOOLEANTYPE:
	case _CW_STILOT_FILETYPE:
	case _CW_STILOT_MARKTYPE:
	case _CW_STILOT_NAMETYPE:
	case _CW_STILOT_NULLTYPE:
	default:
		_cw_error("Programming error");
	}

	memset(retval, 0, sizeof(cw_stiloe_t));

	retval->type = a_type;
	retval->stilt = a_stilt;

#ifdef _LIBSTIL_DBG
	retval->magic = _CW_STILOE_MAGIC;
#endif
	return retval;
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
