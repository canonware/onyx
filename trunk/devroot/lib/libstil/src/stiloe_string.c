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

cw_stiloe_string_t *
stiloe_string_new(cw_stilt_t *a_stilt)
{
	cw_stiloe_string_t	*retval;

	retval = (cw_stiloe_string_t *)_cw_stilt_malloc(a_stilt,
	    sizeof(cw_stiloe_string_t));
	stiloe_new(&retval->stiloe, a_stilt, _CW_STILOT_STRINGTYPE);
	
	retval->e.s.str = NULL;
	retval->e.s.len = -1;

	return retval;
}

void
stiloe_string_delete(cw_stiloe_string_t *a_stiloe_string)
{
	/* XXX */
}

cw_sint32_t
stiloe_string_len_get(cw_stiloe_string_t *a_stiloe_string)
{
	/* XXX Assert. */
	return a_stiloe_string->e.s.len;
}

void
stiloe_string_len_set(cw_stiloe_string_t *a_stiloe_string, cw_stilt_t *a_stilt,
    cw_uint32_t a_len)
{
	_cw_assert(stiloe_is_composite(&(a_stiloe_string->stiloe)) == FALSE);
	_cw_assert(a_stiloe_string->e.s.len == -1);
	
	a_stiloe_string->e.s.str = (cw_uint8_t *)_cw_stilt_malloc(a_stilt,
	    a_len);
	a_stiloe_string->e.s.len = a_len;
}

void
stiloe_string_set(cw_stiloe_string_t *a_stiloe_string, cw_uint32_t a_offset,
    const char *a_str, cw_uint32_t a_len)
{
	/* XXX Bounds assert. */
	memcpy(&a_stiloe_string->e.s.str[a_offset], a_str, a_len);
}
