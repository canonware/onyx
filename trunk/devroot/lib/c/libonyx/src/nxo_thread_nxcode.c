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

#ifdef _CW_USE_INLINES
_CW_INLINE void
#else
static void
#endif
nxo_p_thread_nxcode(cw_nxo_t *a_thread)
{
	_cw_onyx_code(a_thread, "{dict begin /userdict currentdict dstack 3 sindex spop begin pop /threaddict currentdict def def /currenterror < /stop /stop load /newerror false /errorname `' /estack ( ) /istack ( ) /ostack ( ) /dstack ( ) /recordstacks true /line 1 /column 0 > def /errordict < /throw where {pop [ /dstackunderflow /estackoverflow /invalidaccess /invalidcontext /invalidexit /invalidfileaccess /ioerror /limitcheck /rangecheck /stackunderflow /syntaxerror /typecheck /undefined /undefinedfilename /undefinedresult /unmatchedmark /unmatchedfino /unregistered ] {dup [ exch /throw load /eval cvx ] cvx} foreach} if /handleerror {/currenterror load begin /errorname load /syntaxerror eq {`At line ' print /line load cvs print `, column ' print /column load cvs print `: ' print} if `Error ' print /errorname load 1 sprint /recordstacks load {`ostack: ' print /ostack load 1 sprint `dstack: ' print /dstack load 1 sprint /estack load scount exch pop 1 sub dup 0 gt {`estack/istack trace (0..' print dup cvs print `):\n' print} if 0 1 3 2 roll {dup cvs print dup /estack load exch sindex spop exch pop dup type /arraytype eq {`: {\n' print dup length 1 sub 0 1 3 -1 roll {2 index /istack load exch sindex spop exch pop 1 index eq {` ' print dup cvs print `:-->\t' print} {`\t' print} ifelse 1 index exch get 1 sprint} for `}\n' print pop} {`:\t' print 1 sprint} ifelse pop} for} if end flush} bind > def end}
eval");
}
