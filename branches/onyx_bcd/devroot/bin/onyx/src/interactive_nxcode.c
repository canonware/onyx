/* -*- mode: c ; c-file-style: "canonware-c-style" -*-
 * This file is automatically generated.
 ******************************************************************************
 *
 * <Copyright = jasone>
 * <License>
 *
 ******************************************************************************
 *
 * Version: <Version>
 *
 ******************************************************************************/

static void
interactive_nxcode(cw_nxo_t *a_thread)
{
    cw_onyx_code(a_thread, "{systemdict begin /promptstring {count cvs `onyx:' exch catenate `> ' catenate} bind def end threaddict begin /resume /stop load def end errordict begin /stop {stdin cvx stopped pop} bind def end product print `, version ' print version print `.\n' print flush stdin cvx}bind eval");
}
