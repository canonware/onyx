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

#include "../include/libstash/libstash.h"

#define	_CW_XEPV_FOO	-128
#define	_CW_XEPV_BAR	-129
#define	_CW_XEPV_BIZ	-130

void
func_a(void)
{
	xep_raise(_CW_XEPV_BIZ);
}

void
func_b(void)
{
	xep_try {
	case _CW_XEPV_CODE:
		_cw_out_put_e("_CW_XEPV_CODE\n");
		func_a();
		break;
	case _CW_XEPV_BIZ:
		_cw_out_put_e("_CW_XEPV_BIZ\n");
		xep_handled();
		break;
	} xep_end;
}

void
func_c(void)
{
	xep_try {
	case _CW_XEPV_CODE:
		_cw_out_put_e("_CW_XEPV_CODE\n");
		func_a();
		break;
	case _CW_XEPV_FINALLY:
		_cw_out_put_e("_CW_XEPV_FINALLY\n");
		break;
	} xep_end;
}

int
main()
{
	volatile cw_uint32_t	i;

	libstash_init();
	_cw_out_put("Test begin\n");

	for (i = 0; i < 8; i++) {
		_cw_out_put_e("i == [i]\n", i);
		xep_try {
		case _CW_XEPV_CODE:
			_cw_out_put_e("_CW_XEPV_CODE\n");
			if (i == 2)
				xep_raise(_CW_XEPV_FOO);
			if (i == 3)
				func_a();
			if (i == 5)
				func_b();
			if (i == 6)
				func_c();
			break;
		case _CW_XEPV_FOO:
			_cw_out_put_e("_CW_XEPV_FOO\n");
			xep_handled();
			xep_raise(_CW_XEPV_BAR);
			break;
		case _CW_XEPV_BAR:
			_cw_out_put_e("_CW_XEPV_BAR\n");
			xep_handled();
			break;
		case _CW_XEPV_BIZ:
			_cw_out_put_e("_CW_XEPV_BIZ\n");
			xep_handled();
			break;
		case _CW_XEPV_FINALLY:
			_cw_out_put_e("_CW_XEPV_FINALLY\n");
			break;
		} xep_end;
	}
	

	_cw_out_put("Test end\n");
	libstash_shutdown();
	return 0;
}
