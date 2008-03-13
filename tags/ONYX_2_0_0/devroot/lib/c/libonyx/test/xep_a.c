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

#include "../include/libonyx/libonyx.h"

#define	_CW_XEPV_FOO	128
#define	_CW_XEPV_BAR	129
#define	_CW_XEPV_BIZ	130
#define	_CW_XEPV_BAZ	131
#define	_CW_XEPV_BANG	132
#define	_CW_XEPV_BAM	133

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
		fprintf(stderr, "%s(): _CW_XEPV_CODE\n", __FUNCTION__);
		func_a();
	}
	xep_catch(_CW_XEPV_BIZ) {
		fprintf(stderr, "%s(): _CW_XEPV_BIZ\n", __FUNCTION__);
		xep_handled();
	}
	xep_end();
}

void
func_c(void)
{
	xep_begin();
	xep_try {
		fprintf(stderr, "%s(): _CW_XEPV_CODE\n", __FUNCTION__);
		func_a();
	}
	xep_finally {
		fprintf(stderr, "%s(): _CW_XEPV_FINALLY\n", __FUNCTION__);
	}
	xep_end();
}

int
main()
{
	volatile cw_uint32_t	i;

	libonyx_init();
	fprintf(stderr, "Test begin\n");

	for (i = 0; i < 11; i++) {
		fprintf(stderr, "%s(): i == %u\n", __FUNCTION__, i);
		xep_begin();
		xep_try {
			fprintf(stderr, "%s(): _CW_XEPV_CODE\n", __FUNCTION__);
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
			fprintf(stderr, "%s(): _CW_XEPV_FOO\n", __FUNCTION__);
			xep_handled();
			xep_throw(_CW_XEPV_BAR);
		}
		xep_catch(_CW_XEPV_BAR) {
			_cw_assert(xep_value() == _CW_XEPV_BAR);
			fprintf(stderr, "%s(): _CW_XEPV_BAR\n", __FUNCTION__);
			xep_handled();
		}
		xep_catch(_CW_XEPV_BIZ) {
			_cw_assert(xep_value() == _CW_XEPV_BIZ);
			fprintf(stderr, "%s(): _CW_XEPV_BIZ\n", __FUNCTION__);
			xep_handled();
		}
		xep_catch(_CW_XEPV_BAZ)
		xep_mcatch(_CW_XEPV_BANG) {
			_cw_assert(xep_value() == _CW_XEPV_BAZ || xep_value() ==
				   _CW_XEPV_BANG);
			if (xep_value() == _CW_XEPV_BAZ) {
				fprintf(stderr, "%s(): _CW_XEPV_BAZ\n",
				    __FUNCTION__);
				i++;
				xep_retry();
			} else {
				fprintf(stderr, "%s(): _CW_XEPV_BANG\n",
				    __FUNCTION__);
				xep_handled();
			}
		}
		xep_acatch {
			if (xep_value() == _CW_XEPV_BAM) {
				fprintf(stderr, "%s(): _CW_XEPV_BAM\n",
				    __FUNCTION__);
				xep_handled();
			}
		}
		xep_finally {
			fprintf(stderr, "%s(): _CW_XEPV_FINALLY\n",
			    __FUNCTION__);
		}
		xep_end();
	}

	xep_begin();
	xep_try {
	}
	xep_acatch {
		fprintf(stderr, "%s(): xep_acatch\n", __FUNCTION__);
	}
	xep_end();

	fprintf(stderr, "Test end\n");
	libonyx_shutdown();
	return 0;
}
