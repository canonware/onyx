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
#define	_CW_XEPV_BAZ	-131
#define	_CW_XEPV_BANG	-132
#define	_CW_XEPV_BAM	-133

void
func_a(void)
{
	xep_throw(_CW_XEPV_BIZ);
}

void
func_b(void)
{
	xep_begin();
	xep_try {
		out_put_e(cw_g_out, NULL, 0, __FUNCTION__, "_CW_XEPV_CODE\n");
		func_a();
	}
	xep_catch(_CW_XEPV_BIZ) {
		out_put_e(cw_g_out, NULL, 0, __FUNCTION__, "_CW_XEPV_BIZ\n");
		xep_handled();
	}
	xep_end();
}

void
func_c(void)
{
	xep_begin();
	xep_try {
		out_put_e(cw_g_out, NULL, 0, __FUNCTION__, "_CW_XEPV_CODE\n");
		func_a();
	}
	xep_finally {
		out_put_e(cw_g_out, NULL, 0, __FUNCTION__,
		    "_CW_XEPV_FINALLY\n");
	}
	xep_end();
}

int
main()
{
	volatile cw_uint32_t	i;

	libstash_init();
	_cw_out_put("Test begin\n");

	for (i = 0; i < 11; i++) {
		out_put_e(cw_g_out, NULL, 0, __FUNCTION__, "i == [i]\n", i);
		xep_begin();
		xep_try {
			out_put_e(cw_g_out, NULL, 0, __FUNCTION__,
			    "_CW_XEPV_CODE\n");
			if (i == 2)
				xep_throw(_CW_XEPV_FOO);
			if (i == 3)
				func_a();
			if (i == 5)
				func_b();
			if (i == 6)
				func_c();
			if (i == 7)
				xep_throw(_CW_XEPV_BAZ);
			if (i == 8)
				xep_throw(_CW_XEPV_BANG);
			if (i == 9)
				xep_throw(_CW_XEPV_BAM);
		}
		xep_catch(_CW_XEPV_FOO) {
			_cw_assert(xep_value() == _CW_XEPV_FOO);
			out_put_e(cw_g_out, NULL, 0, __FUNCTION__,
			    "_CW_XEPV_FOO\n");
			xep_handled();
			xep_throw(_CW_XEPV_BAR);
		}
		xep_catch(_CW_XEPV_BAR) {
			_cw_assert(xep_value() == _CW_XEPV_BAR);
			out_put_e(cw_g_out, NULL, 0, __FUNCTION__,
			    "_CW_XEPV_BAR\n");
			xep_handled();
		}
		xep_catch(_CW_XEPV_BIZ) {
			_cw_assert(xep_value() == _CW_XEPV_BIZ);
			out_put_e(cw_g_out, NULL, 0, __FUNCTION__,
			    "_CW_XEPV_BIZ\n");
			xep_handled();
		}
		xep_catch(_CW_XEPV_BAZ)
		xep_mcatch(_CW_XEPV_BANG) {
			_cw_assert(xep_value() == _CW_XEPV_BAZ || xep_value() ==
				   _CW_XEPV_BANG);
			if (xep_value() == _CW_XEPV_BAZ) {
				out_put_e(cw_g_out, NULL, 0, __FUNCTION__,
				    "_CW_XEPV_BAZ\n");
				xep_retry();
			} else {
				out_put_e(cw_g_out, NULL, 0, __FUNCTION__,
				    "_CW_XEPV_BANG\n");
				xep_handled();
			}
		}
		xep_acatch {
			if (xep_value() == _CW_XEPV_BAM) {
				out_put_e(cw_g_out, NULL, 0, __FUNCTION__,
				    "_CW_XEPV_BAM\n");
				xep_handled();
			}
		}
		xep_finally {
			out_put_e(cw_g_out, NULL, 0, __FUNCTION__,
			    "_CW_XEPV_FINALLY\n");
		}
		xep_end();
	}
	

	_cw_out_put("Test end\n");
	libstash_shutdown();
	return 0;
}
