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

#ifdef CW_USE_INLINES
CW_INLINE void
#else
static void
#endif
nx_p_nxcode(cw_nx_t *a_nx)
{
    cw_nxo_t thread;

    nxo_thread_new(&thread, a_nx);
    cw_onyx_code(&thread, "</w//ostack/v//xcheck/u//type/t//true/s//sub/r//spop/q//scount/p//roll/o//putinterval/n//pop/m//load/l//known/k//index/j//ifelse/i//getinterval/h//flush/g//false/f//exch/e//eval/d//dup/c//def/b//cvx/a//catenate>begin{systemdict begin /sprintsdict < /arraytype {d 0 gt {1 k d echeck {n `_{'} {v {`{'} {`['} j} j 2 k length 1 gt {0 1 4 k length 2 s {3 k f get 2 k 1 s 1 k u sprintsdict f get e a ` ' a} for} if 2 k length 0 gt {2 k d length 1 s get 2 k 1 s 1 k u sprintsdict f get e a} if f n f d echeck {n `}_'} {v {`}'} {`]'} j} j a} {n n `-array-'} j} /booleantype {n cvs} /conditiontype {n n `-condition-'} /dicttype {d 0 gt {1 s 0 `<' 3 k {f 4 k 1 k u sprintsdict f get e ` ' a f 4 k 1 k u sprintsdict f get e a a f 1 add d 4 k length lt {f ` ' a} {f} j} foreach 4 1 p n n n `>' a} {n n `-dict-'} j} /filetype {n n `-file-'} /finotype {n n `-fino-'} /hooktype {f hooktag d u /nulltype ne {f 1 s 1 k u sprintsdict f get e `=' f a `=' a} {n n `-hook-'} j} /integertype {n cvs} /marktype {n n `-mark-'} /mutextype {n n `-mutex-'} /nametype {n d cvs f v not {`/' f a} if} /nulltype {n n `null'} /operatortype {n cvs d `-operator-' ne {`--' d 3 1 p a a} if} /pmarktype {n n `-pmark-'} /stacktype {d 0 gt {`(' 2 k q 1 gt {2 k q 1 s -1 1 {3 k d 3 2 p sindex r 2 k 1 s 1 k u sprintsdict f get e a ` ' a} for} if 2 k q 0 gt {2 k d sdup r 2 k 1 s 1 k u sprintsdict f get e a} if 3 1 p n n `)' a} {n n `-stack-'} j} /stringtype {n cvs} /threadtype {n n `-thread-'} > c /sprints {1 k u sprintsdict f get e} c /sprint {sprints print `\n' print h} c /outputsdict < /integertype {d /b l {d /b get} {10} j 2 k f cvrs 1 k /s l {1 k /s get /+ eq {2 k 0 ge {`+' f a} if} if} if 3 2 p n f outputsdict /stringtype get e} /_s {d /w l {d /p l {d /p get} {` '} j 1 k /w get 3 k length d 2 k le {1 k string 0 1 4 k 1 s {1 k f 5 k o} for 4 3 p n 3 k /j l {3 k /j get} {/r} j d /r eq {n 3 1 p s} {/l eq {3 1 p n n 0} {3 1 p s 2 div} j} j 1 k f 5 4 p o f} {n n n} j} {d /n l {1 k length 1 k /n get d 2 k lt {f n d string d 5 4 p 4 3 p 0 f i 0 f o f} {n n} j} if} j n} c /stringtype /_s m [ /dup b /r /known b [ /dup b /r /get b ] b [ 1 ] b /ifelse b 3 2 /roll b /exch b /sprints b /exch b /_s m /eval b ] b bind [ /arraytype /booleantype /conditiontype /dicttype /filetype /finotype /hooktype /marktype /mutextype /nametype /nulltype /operatortype /pmarktype /stacktype /threadtype ] {f d} foreach n currentdict /_s undef > c /outputs {d u /dicttype ne {/typecheck throw} if 1 k u outputsdict f get e} c /output {outputs print} c /product `Canonware Onyx' c /pstack {w {1 sprint} foreach h} c /search {d u /stringtype ne {/typecheck throw} if 1 k u /stringtype ne {/typecheck throw} if d length 2 k length le {0 1 3 k length 3 k length s {d 3 k f 3 k length i d length 1 s 0 1 3 2 p {d 2 k f get f 4 k f get ne {g exit} if} for d u /booleantype eq {n n n} {3 2 p n 3 1 p d 2 k f 0 f i 3 1 p 3 k length add d 2 k length f s i 3 1 p t exit} j} for d u /booleantype ne {n g} if} {n g} j} c /system {d u /arraytype ne {/typecheck throw} if d length 0 eq {/rangecheck throw} if d {{u /stringtype ne {stop} if} foreach} stopped {/typecheck throw} if fork d 0 eq {systemdict begin /throw {1 die} c end n exec} {f n waitpid} j} c /throw {{{/errordict where {n t} {g} j} {/errordict m /handleerror l} {/errordict m /stop l} {/currenterror where {n t} {g} j}} {e not {stderr `Onyx: Missing errordict or currenterror defs\n' write stderr `ostack: ' write w 2 sprints stderr f write stderr `\ndstack: ' write stderr dstack 2 sprints write stderr `\nestack: ' write stderr estack 2 sprints write stderr `\nistack: ' write stderr istack 2 sprints write stderr `\nOnyx: dieing\n' write 1 die} if} foreach d u /nametype ne {/typecheck /systemdict m /throw get e} if /currenterror m begin /newerror t c d /errorname f cvlit c w d r n /ostack f c /dstack dstack d r n c /estack estack d r n c /istack istack d r n c /estack m d sdup r f end /errordict m d 2 k l {f get e} {f n d /handleerror get e /stop get e} j} c /version `<Version>' c end}bind eval end");
    nxo_thread_exit(&thread);
}
