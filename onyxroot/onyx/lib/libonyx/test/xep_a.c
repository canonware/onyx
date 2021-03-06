/* -*- mode: c ; c-file-style: "canonware-c-style" -*-
 ******************************************************************************
 *
 * <Copyright = jasone>
 * <License>
 *
 ******************************************************************************
 *
 * Version: Onyx <Version = onyx>
 *
 ******************************************************************************/

#include "../include/libonyx/libonyx.h"

#define CW_XEPV_FOO 128
#define CW_XEPV_BAR 129
#define CW_XEPV_BIZ 130
#define CW_XEPV_BAZ 131
#define CW_XEPV_BAM 132

void
func_a(void)
{
    xep_throw(CW_XEPV_BAR);
}

void
func_b(void)
{
    xep_begin();
    xep_try
    {
	fprintf(stderr, "%s(): CW_XEPV_CODE\n", __func__);
	func_a();
    }
    xep_catch(CW_XEPV_BAR)
    {
	fprintf(stderr, "%s(): CW_XEPV_BAR\n", __func__);
	xep_handled();
    }
    xep_end();
}

void
func_c(void)
{
    xep_begin();
    xep_try
    {
	fprintf(stderr, "%s(): CW_XEPV_CODE\n", __func__);
	func_a();
    }
    xep_end();
}

int
main()
{
    volatile uint32_t i;

    libonyx_init(0, NULL, NULL);
    fprintf(stderr, "Test begin\n");

    for (i = 0; i < 11; i++)
    {
	fprintf(stderr, "%s(): i == %u\n", __func__, i);
	xep_begin();
	xep_try
	{
	    fprintf(stderr, "%s(): CW_XEPV_CODE\n", __func__);
	    if (i == 2)
	    {
		xep_throw(CW_XEPV_FOO);
	    }
	    if (i == 3)
	    {
		func_a();
	    }
	    if (i == 5)
	    {
		func_b();
	    }
	    if (i == 6)
	    {
		func_c();
	    }
	    if (i == 7)
	    {
		xep_throw(CW_XEPV_BIZ);
	    }
	    if (i == 8)
	    {
		xep_throw(CW_XEPV_BAZ);
	    }
	    if (i == 9)
	    {
		xep_throw(CW_XEPV_BAM);
	    }
	}
	xep_catch(CW_XEPV_FOO)
	{
	    cw_assert(xep_value() == CW_XEPV_FOO);
	    fprintf(stderr, "%s(): CW_XEPV_FOO\n", __func__);
	    xep_handled();
	}
	xep_catch(CW_XEPV_BAR)
	{
	    cw_assert(xep_value() == CW_XEPV_BAR);
	    fprintf(stderr, "%s(): CW_XEPV_BAR\n", __func__);
	    xep_handled();
	}
	xep_catch(CW_XEPV_BIZ)
	xep_mcatch(CW_XEPV_BAZ)
	{
	    cw_assert(xep_value() == CW_XEPV_BIZ
		      || xep_value() == CW_XEPV_BAZ);
	    if (xep_value() == CW_XEPV_BIZ)
	    {
		fprintf(stderr, "%s(): CW_XEPV_BIZ\n", __func__);
		i++;
		xep_retry();
	    }
	    else
	    {
		fprintf(stderr, "%s(): CW_XEPV_BAZ\n", __func__);
		xep_handled();
	    }
	}
	xep_acatch
	{
	    if (xep_value() == CW_XEPV_BAM)
	    {
		fprintf(stderr, "%s(): CW_XEPV_BAM\n", __func__);
		xep_handled();
	    }
	}
	xep_end();
    }

    xep_begin();
    xep_try
    {
    }
    xep_acatch
    {
	fprintf(stderr, "%s(): xep_acatch\n", __func__);
    }
    xep_end();

    fprintf(stderr, "Test end\n");
    libonyx_shutdown();
    return 0;
}
