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
nx_p_nxcode(cw_nx_t *a_nx)
{
	cw_nxo_t	thread;

	nxo_thread_new(&thread, a_nx);
	_cw_onyx_code(&thread, "{systemdict begin /:: {mark} bind def /: {counttomark dup 0 eq {pop errordict begin unmatchedmark end} {-1 roll load {counttomark dup 1 le {pop exit} if -1 roll get} loop exch pop eval} ifelse} bind def /product {`Canonware onyx'} bind def /pstack {ostack {1 sprint} foreach flush} bind def /search {dup type /stringtype ne {errordict begin typecheck end} if 1 index type /stringtype ne {errordict begin typecheck end} if dup length 2 index length le {0 1 3 index length 3 index length sub {dup 3 index exch 3 index length getinterval dup length 1 sub 0 1 3 2 roll {dup 2 index exch get exch 4 index exch get ne {false exit} if} for dup type /booleantype eq {pop pop pop} {3 2 roll pop 3 1 roll dup 2 index exch 0 exch getinterval 3 1 roll 3 index length add dup 2 index length exch sub getinterval 3 1 roll true exit} ifelse} for dup type /booleantype ne {pop false} if} {pop false} ifelse} bind def /system {fork dup 0 eq {errordict begin /handleerror {quit} def end pop exec} {exch pop waitpid} ifelse} bind def /throw {{{/errordict where {pop true} {false} ifelse} {/errordict load /handleerror known} {/currenterror where {pop true} {false} ifelse} {/currenterror load /recordstacks known} {/currenterror load /stop known}} {eval not {`Onyx: Missing errordict or currenterror defs\n' print `ostack: ' print ostack 2 sprint `dstack: ' print dstack 2 sprint `estack: ' print estack 2 sprint `istack: ' print istack 2 sprint `Onyx: dieing\n' print flush 1 die} if} foreach dup type /nametype ne {/errordict load /typecheck get eval} if /currenterror load begin /newerror true def /errorname exch cvx def /recordstacks load {ostack /ostack cvx exch def /dstack cvx dstack spop pop def /estack cvx estack spop pop spop pop def /istack cvx istack spop pop spop pop def} if end estack 1 sindex spop exch pop /errordict load /handleerror get eval /currenterror load /stop get eval} bind def /version {`<Version>'} bind def end}
eval");
	nxo_thread_exit(&thread);
}
