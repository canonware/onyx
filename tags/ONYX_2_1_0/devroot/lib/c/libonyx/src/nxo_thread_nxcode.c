/* This file is automatically generated.
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

#ifdef _CW_USE_INLINES
_CW_INLINE void
#else
static void
#endif
nxo_p_thread_nxcode(cw_nxo_t *a_thread)
{
	_cw_onyx_code(a_thread, "{dict begin /userdict currentdict dstack dup 3 sindex dup spop begin pop /threaddict currentdict def def /currenterror < /newerror false /errorname `' /estack ( ) /istack ( ) /ostack ( ) /dstack ( ) /line 1 /column 0 > def /errordict < /stop /stop load /handleerror {/currenterror load begin /errorname load /syntaxerror eq {`At line ' print /line load cvs print `, column ' print /column load cvs print `: ' print} if `Error ' print /errorname load 1 sprint `ostack: ' print /ostack load 1 sprint `dstack: ' print /dstack load 1 sprint /estack load scount 1 sub dup 0 gt {`estack/istack trace (0..' print dup cvs print `):\n' print} if 0 1 3 2 roll {dup cvs print dup /estack load dup 3 2 roll sindex spop dup type /arraytype eq {`: {\n' print dup length 1 sub 0 1 3 -1 roll {2 index /istack load dup 3 2 roll sindex spop 1 index eq {dup < /w 3 > output `:--> ' print} {`\t' print} ifelse 1 index exch get 1 sprint} for `}\n' print pop} {`:\t' print 1 sprint} ifelse pop} for end flush} bind > def end}eval");
}