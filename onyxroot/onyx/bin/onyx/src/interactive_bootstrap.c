/* -*- mode: c ; c-file-style: "canonware-c-style" -*-
 * This file is automatically generated.
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

#include "onyx.h"

void
interactive_nxcode(cw_nxo_t *a_thread)
{
    cw_onyx_code(a_thread, "{systemdict begin $promptstring {count cvs `onyx:' exch `> ' 3 ncat} bind def end threaddict begin $resume {currenterror begin $newerror false def $errorname null def !estack dup scount snpop pop !istack dup scount snpop pop !ostack dup scount snpop pop !dstack dup scount snpop pop end stop} bind def end errordict begin $stop {stdin cvx stopped pop} bind def end $modprompt mclass :singleton :load}bind eval");
}
