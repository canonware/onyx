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

#include <libonyx/libonyx.h>

void
nxo_p_thread_nxcode(cw_nxo_t *a_thread)
{
    cw_onyx_code(a_thread, "{dict begin /userdict currentdict dstack dup 3 sindex dup spop begin pop /threaddict currentdict def def /currenterror < /newerror false /errorname `' /estack ( ) /istack ( ) /ostack ( ) /dstack ( ) /line 1 /column 0 > def /errordict < /stop /stop load /handleerror {/currenterror load begin /errorname load /syntaxerror eq {stderr dup `At line ' write dup /line load cvs write dup `, column ' write dup /column load cvs write `: ' write} if stderr dup `Error ' write dup /errorname load 1 sprints write dup `\nostack: ' write dup /ostack load 1 sprints write dup `\ndstack: ' write dup /dstack load 1 sprints write `\n' write /estack load scount 1 sub dup 0 gt {stderr dup `estack/istack trace (0..' write 1 index cvs stderr exch write `):\n' write} if 0 1 3 2 roll {dup cvs stderr exch write dup /estack load dup 3 2 roll sindex spop dup type /arraytype eq {`: {\n' stderr exch write dup length 1 sub 0 1 3 -1 roll {2 index /istack load dup 3 2 roll sindex spop 1 index eq {dup < /w 3 > outputs stderr exch write `:--> '} {`\t'} ifelse stderr exch write 1 index exch get 1 sprints stderr exch write stderr `\n' write} for stderr `}\n' write pop} {stderr `:\t' write 1 sprints stderr exch write stderr `\n' write} ifelse pop} for end flush} bind > def end}eval");
}
