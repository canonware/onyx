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
	
	/*
	 * Delete extended types if they only have one reference.  Otherwise,
	 * the GC is responsible for determining when an object can be deleted.
	 */
	if (a_stilo->ref_count == 0) {
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
			stiloe_array_delete(a_stilo->o.array.stiloe);
			break;
		case _CW_STILOT_CONDITIONTYPE:
			stiloe_condition_delete(a_stilo->o.condition.stiloe);
			break;
		case _CW_STILOT_DICTTYPE:
			stiloe_dict_delete(a_stilo->o.dict.stiloe);
			break;
		case _CW_STILOT_HOOKTYPE:
			stiloe_hook_delete(a_stilo->o.hook.stiloe);
			break;
		case _CW_STILOT_LOCKTYPE:
			stiloe_lock_delete(a_stilo->o.lock.stiloe);
			break;
		case _CW_STILOT_MSTATETYPE:
			if (a_stilo->extended)
				stiloe_mstate_delete(a_stilo->o.mstate.stiloe);
			break;
		case _CW_STILOT_NAMETYPE:
			if (a_stilo->indirect_name) {
				stiltn_unref(a_stilo->o.name.s.stilt,
				    a_stilo->o.name.stiln);
			} else {
				stil_stiln_unref(stilt_get_stil(a_stilo->o.name.s.stilt),
				    a_stilo->o.name.stiln,
				    a_stilo->o.name.s.key); 
			}	
			break;
		case _CW_STILOT_NUMBERTYPE:
			if (a_stilo->extended)
				stiloe_number_delete(a_stilo->o.number.val.stiloe);
			break;
		case _CW_STILOT_STRINGTYPE:
			break;
		default:
			_cw_error("Programming error");
		}
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

cw_stiloe_t *
stilo_get_extended(cw_stilo_t *a_stilo)
{
	cw_stiloe_t	*retval;

	switch (a_stilo->type) {
	case _CW_STILOT_NOTYPE:
	case _CW_STILOT_ARRAYTYPE:
	case _CW_STILOT_BOOLEANTYPE:
	case _CW_STILOT_CONDITIONTYPE:
	case _CW_STILOT_DICTTYPE:
	case _CW_STILOT_FILETYPE:
	case _CW_STILOT_HOOKTYPE:
	case _CW_STILOT_LOCKTYPE:
	case _CW_STILOT_MARKTYPE:
	case _CW_STILOT_MSTATETYPE:
	case _CW_STILOT_NAMETYPE:
	case _CW_STILOT_NULLTYPE:
	case _CW_STILOT_NUMBERTYPE:
	case _CW_STILOT_OPERATORTYPE:
		_cw_error("Programming error");
	case _CW_STILOT_STRINGTYPE:
		retval = (cw_stiloe_t *)a_stilo->o.string.stiloe;
		break;
	default:
		_cw_error("Programming error");
	}
	return retval;
}

void
stilo_copy(cw_stilo_t *a_to, cw_stilo_t *a_from)
{
	_cw_check_ptr(a_to);
	_cw_assert(a_to->magic == _CW_STILO_MAGIC);
	_cw_check_ptr(a_from);
	_cw_assert(a_from->magic == _CW_STILO_MAGIC);
}

void
stilo_move(cw_stilo_t *a_to, cw_stilo_t *a_from)
{
	/* XXX */
}

cw_bool_t
stilo_cast(cw_stilo_t *a_stilo, cw_stilt_t *a_stilt, cw_stilot_t a_stilot)
{
	_cw_check_ptr(a_stilo);
	_cw_assert(a_stilo->magic == _CW_STILO_MAGIC);

	switch (a_stilo->type) {
	case _CW_STILOT_NOTYPE:
		switch (a_stilot) {
		case _CW_STILOT_STRINGTYPE:
			a_stilo->type = _CW_STILOT_STRINGTYPE;
			a_stilo->o.string.stiloe = (cw_stiloe_string_t
			    *)stiloe_string_new(a_stilt);
			break;
		default:
			_cw_error("Programming error");
		}
		break;
	case _CW_STILOT_ARRAYTYPE:
	case _CW_STILOT_BOOLEANTYPE:
	case _CW_STILOT_CONDITIONTYPE:
	case _CW_STILOT_DICTTYPE:
	case _CW_STILOT_FILETYPE:
	case _CW_STILOT_HOOKTYPE:
	case _CW_STILOT_LOCKTYPE:
	case _CW_STILOT_MARKTYPE:
	case _CW_STILOT_MSTATETYPE:
	case _CW_STILOT_NAMETYPE:
	case _CW_STILOT_NULLTYPE:
	case _CW_STILOT_NUMBERTYPE:
	case _CW_STILOT_OPERATORTYPE:
	case _CW_STILOT_STRINGTYPE:
	default:
		_cw_error("Programming error");
	}


	return TRUE;	/* XXX */
}
