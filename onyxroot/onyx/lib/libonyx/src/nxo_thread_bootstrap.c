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

#include "libonyx/libonyx.h"

void
nxo_p_thread_nxcode(cw_nxo_t *a_thread)
{
    cw_onyx_code(a_thread, "{dict begin $userdict currentdict dstack dup sbdup dup spop begin pop $threaddict currentdict def def $currenterror < $newerror false $errorname null $estack stack $istack stack $ostack stack $dstack stack $origin null $line 1 $column 0 > def $errordict < $stop $stop load $handleerror {!currenterror begin !errorname $syntaxerror eq {`At ' !origin null ne {!origin `:' 3 ncat} if !line cvs `:' !column cvs `: ' 5 ncat {stderr exch write} {} until} if `Error ' {stderr exch write} {} until !errorname 1 sprints {stderr exch write} {} until `\nostack: ' {stderr exch write} {} until !ostack 1 sprints {stderr exch write} {} until `\ndstack: ' {stderr exch write} {} until !dstack 1 sprints {stderr exch write} {} until `\n' {stderr exch write} {} until !estack scount 1 sub dup 0 gt {`estack/istack trace (0..' {stderr exch write} {} until dup cvs {stderr exch write} {} until `):\n' {stderr exch write} {} until} if 0 1 dn {dup cvs {stderr exch write} {} until dup !estack dup dn sidup spop dup type $arraytype eq {dup origin {`:' up `:' exch cvs 4 ncat {stderr exch write} {} until} if `: {\n' {stderr exch write} {} until dup length 1 sub 0 1 dn {2 idup !istack dup dn sidup spop over eq {dup < $w 3 > outputs {stderr exch write} {} until `:--> '} {`\t'} ifelse {stderr exch write} {} until over exch get 1 sprints {stderr exch write} {} until `\n' {stderr exch write} {} until} for `}\n' {stderr exch write} {} until pop} {`:\t' {stderr exch write} {} until 1 sprints {stderr exch write} {} until `\n' {stderr exch write} {} until} ifelse pop} for end flush} bind > def end}eval");
}
