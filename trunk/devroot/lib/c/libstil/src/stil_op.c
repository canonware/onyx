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

void
stil_op_print(cw_stilt_t *a_stilt)
{

}

void
stil_op_pstack(cw_stilt_t *a_stilt)
{
	cw_stils_t	*stils;
	cw_stilo_t	*stilo;
	cw_sint32_t	fd;

	stils = stilt_data_stack_get(a_stilt);
	fd = stilt_stdout_get(a_stilt);

	for (stilo = stils_get(stils, 0); stilo != NULL; stilo =
		 stils_get_down(stils, stilo)) {
		switch (stilo_type_get(stilo)) {
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
			/* XXX */
			_cw_error("Programming error");
		case _CW_STILOT_STRINGTYPE: {
			cw_stiloe_t	*stiloe;
			cw_uint8_t	*str;
			cw_sint32_t	len;

			stiloe = stilo_extended_get(stilo);
			str = stiloe_string_get(stiloe);
			len = stiloe_string_len_get(stiloe);

			_cw_out_put_f(fd, "(");
			if (len > 0)
				_cw_out_put_fn(fd, len, "[s]", str);
			_cw_out_put_f(fd, ")\n");

			break;
		}
		case _CW_STILOT_NOTYPE:
		default:
			_cw_error("Programming error");
		}
		
	}
}
