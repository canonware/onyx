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

	/* XXX Perhaps stiloe_delete() should have the switch statement. */
	
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
		case _CW_STILOT_MSTATETYPE:
		case _CW_STILOT_NUMBERTYPE:
			if (a_stilo->extended == FALSE)
				break;
			/* Fall through if extended. */
		case _CW_STILOT_ARRAYTYPE:
		case _CW_STILOT_CONDITIONTYPE:
		case _CW_STILOT_DICTTYPE:
		case _CW_STILOT_HOOKTYPE:
		case _CW_STILOT_LOCKTYPE:
		case _CW_STILOT_STRINGTYPE:
			stiloe_delete(a_stilo->o.stiloe);
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
		default:
			_cw_error("Programming error");
		}
	}

#ifdef _LIBSTIL_DBG
	a_stilo->magic = _CW_STILO_MAGIC;
#endif
}

cw_stilot_t
stilo_type_get(cw_stilo_t *a_stilo)
{
	_cw_check_ptr(a_stilo);
	_cw_assert(a_stilo->magic == _CW_STILO_MAGIC);

	return a_stilo->type;
}

void
stilo_type_set(cw_stilo_t *a_stilo, cw_stilot_t a_stilot)
{
	_cw_check_ptr(a_stilo);
	_cw_assert(a_stilo->magic == _CW_STILO_MAGIC);

	if (a_stilo->extended) {
		/* Clear the stiloe pointer, just in case. */
		stilo_extended_set(a_stilo, NULL);
		a_stilo->extended = FALSE;
	}

	a_stilo->type = a_stilot;

	switch (a_stilot) {
	case _CW_STILOT_ARRAYTYPE:
	case _CW_STILOT_BOOLEANTYPE:
	case _CW_STILOT_CONDITIONTYPE:
	case _CW_STILOT_DICTTYPE:
	case _CW_STILOT_HOOKTYPE:
	case _CW_STILOT_LOCKTYPE:
	case _CW_STILOT_STRINGTYPE:
		stilo_extended_set(a_stilo, NULL);
		a_stilo->extended = TRUE;
		break;
	case _CW_STILOT_FILETYPE:
		a_stilo->o.file.fd = -1;
		break;
	case _CW_STILOT_NAMETYPE:
		a_stilo->o.name.s.stilt = NULL;
		a_stilo->o.name.stiln = NULL;
		break;
	case _CW_STILOT_NUMBERTYPE:
		a_stilo->o.number.s32 = 0;
		break;
	case _CW_STILOT_OPERATORTYPE:
		a_stilo->o.operator.f = NULL;
		break;
	case _CW_STILOT_NOTYPE:
	case _CW_STILOT_MARKTYPE:
	case _CW_STILOT_MSTATETYPE:	/* Not extended yet. */
	case _CW_STILOT_NULLTYPE:
		break;
	default:
		_cw_error("Programming error");
	}
}

cw_stiloe_t *
stilo_extended_get(cw_stilo_t *a_stilo)
{
	cw_stiloe_t	*retval;

	_cw_check_ptr(a_stilo);
	_cw_assert(a_stilo->magic == _CW_STILO_MAGIC);

	switch (a_stilo->type) {
	case _CW_STILOT_MSTATETYPE:
	case _CW_STILOT_NUMBERTYPE:
		if (a_stilo->extended == FALSE) {
			retval = NULL;
			break;
		}
		/* Fall through if extended. */
	case _CW_STILOT_ARRAYTYPE:
	case _CW_STILOT_CONDITIONTYPE:
	case _CW_STILOT_DICTTYPE:
	case _CW_STILOT_HOOKTYPE:
	case _CW_STILOT_LOCKTYPE:
	case _CW_STILOT_STRINGTYPE:
		retval = a_stilo->o.stiloe;
		break;
	case _CW_STILOT_NOTYPE:
	case _CW_STILOT_BOOLEANTYPE:
	case _CW_STILOT_FILETYPE:
	case _CW_STILOT_MARKTYPE:
	case _CW_STILOT_NAMETYPE:
	case _CW_STILOT_NULLTYPE:
	case _CW_STILOT_OPERATORTYPE:
	default:
		_cw_error("Programming error");
	}
	return retval;
}

void
stilo_extended_set(cw_stilo_t *a_stilo, cw_stiloe_t *a_stiloe)
{
	cw_stilot_t	type;

	_cw_check_ptr(a_stilo);
	_cw_assert(a_stilo->magic == _CW_STILO_MAGIC);
	
	/* Mark the stilo as invalid. */
	type = a_stilo->type;
	a_stilo->type = _CW_STILOT_NOTYPE;

	switch (type) {
	case _CW_STILOT_MSTATETYPE:
	case _CW_STILOT_NUMBERTYPE:
		/*
		 * Consder (a_stiloe == NULL) to mean that this stilo is being
		 * toggled to the non-extended state.
		 */
		if (a_stiloe != NULL) {
			a_stilo->extended = TRUE;
			a_stilo->o.stiloe = a_stiloe;
		} else
			a_stilo->extended = FALSE;
		break;
	case _CW_STILOT_ARRAYTYPE:
	case _CW_STILOT_CONDITIONTYPE:
	case _CW_STILOT_DICTTYPE:
	case _CW_STILOT_HOOKTYPE:
	case _CW_STILOT_LOCKTYPE:
	case _CW_STILOT_STRINGTYPE:
		a_stilo->o.stiloe = a_stiloe;
		break;
	case _CW_STILOT_NOTYPE:
	case _CW_STILOT_BOOLEANTYPE:
	case _CW_STILOT_FILETYPE:
	case _CW_STILOT_MARKTYPE:
	case _CW_STILOT_NAMETYPE:
	case _CW_STILOT_NULLTYPE:
	case _CW_STILOT_OPERATORTYPE:
	default:
		_cw_out_put_e("Invalid type: [i]\n", type);
		abort();
	}

	/* Mark the stilo as valid again. */
	a_stilo->type = type;
}

void
stilo_copy(cw_stilo_t *a_to, cw_stilo_t *a_from)
{
	cw_stilot_t	type;

	_cw_check_ptr(a_to);
	_cw_check_ptr(a_from);
	_cw_assert(a_from->magic == _CW_STILO_MAGIC);

	/* Mark the stilo as invalid. */
	type = a_from->type;
	a_from->type = _CW_STILOT_NOTYPE;

	/* Copy. */
	memcpy(a_to, a_from, sizeof(cw_stilo_t));

	/*
	 * Overflow the reference count, if necessary.  The order of events here
	 * is critical to thread safety (the GC can get screwed up if we do this
	 * in the wrong order).
	 */
	switch (type) {
	case _CW_STILOT_MSTATETYPE:
	case _CW_STILOT_NUMBERTYPE:
		if (a_from->extended == FALSE)
			break;
		/* Fall through if extended. */
	case _CW_STILOT_ARRAYTYPE:
	case _CW_STILOT_CONDITIONTYPE:
	case _CW_STILOT_DICTTYPE:
	case _CW_STILOT_HOOKTYPE:
	case _CW_STILOT_LOCKTYPE:
	case _CW_STILOT_STRINGTYPE:
		if (a_from->ref_count == 0) {
			/* Overflow the reference count. */
			a_from->ref_count = 1;
			a_to->ref_count = 1;

			/* Add the stiloe to the GC's sequence set. */
			stiloe_gc_register(a_from->o.stiloe);
		}
		break;
	case _CW_STILOT_NOTYPE:
	case _CW_STILOT_BOOLEANTYPE:
	case _CW_STILOT_FILETYPE:
	case _CW_STILOT_MARKTYPE:
	case _CW_STILOT_NAMETYPE:
	case _CW_STILOT_NULLTYPE:
	case _CW_STILOT_OPERATORTYPE:
	default:
		_cw_error("Programming error");
	}

	/* Reset debug flags on new copy. */
	a_to->breakpoint = FALSE;
	a_to->watchpoint = FALSE;

	/* Mark the the stilo's as valid again. */
	a_from->type = type;
	a_to->type = type;
}

void
stilo_move(cw_stilo_t *a_to, cw_stilo_t *a_from)
{
	_cw_check_ptr(a_to);
	_cw_check_ptr(a_from);
	_cw_assert(a_from->magic == _CW_STILO_MAGIC);

	memcpy(a_to, a_from, sizeof(cw_stilo_t));

#ifdef _LIBSTIL_DBG
	a_from->type = _CW_STILOT_NOTYPE;
	memset(a_from, 0x5a, sizeof(cw_stilo_t));
#endif
}
