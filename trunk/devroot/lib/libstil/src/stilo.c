/* -*- mode: c ; c-file-style: "canonware-c-style" -*-
 ****************************************************************************
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
#define _CW_STILO_MAGIC 0x398754ba
#endif

void
stilo_new(cw_stilo_t *a_stilo)
{
	_cw_check_ptr(a_stilo);

	bzero(a_stilo, sizeof(cw_stilo_t));
#ifdef _LIBSTIL_DBG
	a_stilo->magic = _CW_STILO_MAGIC;
#endif
}

void
stilo_delete(cw_stilo_t *a_stilo)
{
	_cw_check_ptr(a_stilo);
	_cw_assert(a_stilo->magic == _CW_STILO_MAGIC);

	switch (a_stilo->type) {
	case _CW_STILOT_NOTYPE:
	case _CW_STILOT_BOOLEANTYPE:
	case _CW_STILOT_FILETYPE:
	case _CW_STILOT_MARKTYPE:
	case _CW_STILOT_NULLTYPE:
	case _CW_STILOT_OPERATORTYPE:
		/* Simple type; do nothing. */
		break;
	case _CW_STILOT_ARRAYTYPE:
		stiloe_array_unref(a_stilo->o.array.stiloe);
		break;
	case _CW_STILOT_CONDITIONTYPE:
		stiloe_condition_unref(a_stilo->o.condition.stiloe);
		break;
	case _CW_STILOT_DICTTYPE:
		break;
	case _CW_STILOT_LOCKTYPE:
		break;
	case _CW_STILOT_MSTATETYPE:
		if (a_stilo->extended)
			;/* XXX */
		break;
	case _CW_STILOT_NAMETYPE:
		break;
	case _CW_STILOT_NUMBERTYPE:
		if (a_stilo->extended)
			;/* XXX */
		break;
	case _CW_STILOT_PACKEDARRAYTYPE:
		break;
	case _CW_STILOT_STRINGTYPE:
		break;
	default:
		_cw_error("Programming error");
	}

#ifdef _LIBSTIL_DBG
	a_stilo->magic = _CW_STILO_MAGIC;
#endif
}

cw_stilot_t
stilo_type(cw_stilo_t *a_stilo)
{
	_cw_check_ptr(a_stilo);
	_cw_assert(a_stilo->magic == _CW_STILO_MAGIC);

	return a_stilo->type;
}

void
stilo_copy(cw_stilo_t *a_to, cw_stilo_t *a_from)
{
	_cw_check_ptr(a_to);
	_cw_assert(a_to->magic == _CW_STILO_MAGIC);
	_cw_check_ptr(a_from);
	_cw_assert(a_from->magic == _CW_STILO_MAGIC);
}

cw_bool_t
stilo_cast(cw_stilo_t *a_stilo, cw_stilot_t a_stilot)
{
	_cw_check_ptr(a_stilo);
	_cw_assert(a_stilo->magic == _CW_STILO_MAGIC);

	return TRUE;		/* XXX */
}
