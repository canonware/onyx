/******************************************************************************
 *
 * <Copyright = jasone>
 * <License>
 *
 ******************************************************************************
 *
 * Version: <Version>
 *
 * cook file for onyx.
 *
 ******************************************************************************/

/* Include generated dependency files. */
#include-cooked [glob [objdir]/bin/onyx/src/"*.d_*"]

/*
 * File lists.
 */

/* Include files that are _not_ installed. */
{
  local priv_incs = onyx.h;
  priv_incs = [fromto % [srcdir]/bin/onyx/include/% [priv_incs]];
  priv_incs = [priv_incs] [objdir]/bin/onyx/include/onyx_defs.h;

  local priv_cincs = onyx_nxcode.c batch_nxcode.c interactive_nxcode.c;
  prix_cincs = [fromto % [srcdir]/bin/onyx/src/% [priv_cincs]];

  PRIV_INCS = [PRIV_INCS] [priv_incs] [priv_cincs];
}

/* Source files. */
ONYX_SRCS = onyx.c;
ONYX_SRCS = [fromto % [srcdir]/bin/onyx/src/% [ONYX_SRCS]];

TONYX_SRCS = tonyx.c;
TONYX_SRCS = [fromto % [srcdir]/bin/onyx/src/% [TONYX_SRCS]];

SRCS = [SRCS] [ONYX_SRCS] [TONYX_SRCS];

/*
 * Onyx tests.
 */
{
  local otests =
#if [enable_posix_file]
#if [enable_threads]
                 monitor_a.nx.in monitor_b.nx.in monitor_c.nx.in
#endif
#if [enable_posix]
                 nsleep_a.nx.in nsleep_b.nx.in nsleep_c.nx.in nsleep_d.nx.in
                 rand_a.nx.in setenv_a.nx.in setenv_b.nx.in setenv_c.nx.in
                 srand_a.nx.in srand_b.nx.in srand_c.nx.in srand_d.nx.in
                 unsetenv_a.nx.in unsetenv_b.nx.in truncate_a.nx.in
                 truncate_b.nx.in truncate_c.nx.in truncate_d.nx.in
                 truncate_e.nx.in
#endif

                 abs_a.nx.in abs_b.nx.in abs_c.nx.in add_a.nx.in add_b.nx.in
                 add_c.nx.in add_d.nx.in and_a.nx.in and_b.nx.in and_c.nx.in
                 and_d.nx.in and_e.nx.in and_f.nx.in begin_a.nx.in begin_b.nx.in
                 begin_c.nx.in bind_a.nx.in catenate_a.nx.in catenate_b.nx.in
                 catenate_c.nx.in catenate_d.nx.in clear_a.nx.in
                 cleardstack_a.nx.in cleartomark_a.nx.in copy_a.nx.in
                 copy_b.nx.in copy_c.nx.in copy_d.nx.in copy_e.nx.in
                 copy_f.nx.in copy_g.nx.in count_a.nx.in countdstack_a.nx.in
                 countestack_a.nx.in counttomark_a.nx.in counttomark_b.nx.in
                 counttomark_c.nx.in cve_a.nx.in cve_b.nx.in cvlit_a.nx.in
                 cvlit_b.nx.in cvn_a.nx.in cvn_b.nx.in cvn_c.nx.in cvrs_a.nx.in
                 cvrs_b.nx.in cvrs_c.nx.in cvrs_d.nx.in cvrs_e.nx.in
                 cvrs_f.nx.in cvs_a.nx.in cvs_b.nx.in cvx_a.nx.in cvx_b.nx.in
                 def_a.nx.in def_b.nx.in dict_a.nx.in div_a.nx.in div_b.nx.in
                 div_c.nx.in div_d.nx.in div_e.nx.in dstack_a.nx.in dup_a.nx.in
                 dup_b.nx.in end_a.nx.in end_b.nx.in eq_a.nx.in eq_b.nx.in
                 eval.nx.in eval_a.nx.in eval_b.nx.in exch_a.nx.in exch_b.nx.in
                 exch_c.nx.in exit_a.nx.in exp_a.nx.in exp_b.nx.in exp_c.nx.in
                 exp_d.nx.in exp_e.nx.in for_a.nx.in for_b.nx.in for_c.nx.in
                 for_d.nx.in for_e.nx.in foreach_a.nx.in foreach_b.nx.in
                 foreach_c.nx.in gc_roll.nx.in ge_a.nx.in ge_b.nx.in ge_c.nx.in
                 ge_d.nx.in ge_e.nx.in ge_f.nx.in ge_g.nx.in ge_h.nx.in
                 get_a.nx.in get_b.nx.in get_c.nx.in get_d.nx.in get_e.nx.in
                 get_f.nx.in get_g.nx.in get_h.nx.in getinterval_a.nx.in
                 getinterval_b.nx.in getinterval_c.nx.in getinterval_d.nx.in
                 getinterval_e.nx.in getinterval_f.nx.in getinterval_g.nx.in
                 getinterval_h.nx.in getinterval_i.nx.in gt_a.nx.in gt_b.nx.in
                 gt_c.nx.in gt_d.nx.in gt_e.nx.in gt_f.nx.in gt_g.nx.in
                 gt_h.nx.in hello.nx.in if_a.nx.in if_b.nx.in if_c.nx.in
                 ifelse_a.nx.in ifelse_b.nx.in ifelse_c.nx.in immediate_a.nx.in
                 immediate_b.nx.in index_a.nx.in index_b.nx.in index_c.nx.in
                 index_d.nx.in index_e.nx.in index_f.nx.in integer.nx.in
                 integer_a.nx.in integer_b.nx.in integer_c.nx.in integer_d.nx.in
                 known_a.nx.in known_b.nx.in known_c.nx.in le_a.nx.in le_b.nx.in
                 le_c.nx.in le_d.nx.in le_e.nx.in le_f.nx.in le_g.nx.in
                 le_h.nx.in length_a.nx.in length_b.nx.in length_c.nx.in
                 loop_a.nx.in loop_b.nx.in lt_a.nx.in lt_b.nx.in lt_c.nx.in
                 lt_d.nx.in lt_e.nx.in lt_f.nx.in lt_g.nx.in lt_h.nx.in
                 mark_a.nx.in mod_a.nx.in mod_b.nx.in mod_c.nx.in mod_d.nx.in
                 mod_e.nx.in mul_a.nx.in mul_b.nx.in mul_c.nx.in mul_d.nx.in
                 ndup_a.nx.in ndup_b.nx.in ndup_c.nx.in ndup_d.nx.in
                 ndup_e.nx.in ne_a.nx.in ne_b.nx.in neg_a.nx.in neg_b.nx.in
                 neg_c.nx.in not_a.nx.in not_b.nx.in not_c.nx.in npop_a.nx.in
                 npop_b.nx.in npop_c.nx.in npop_d.nx.in npop_e.nx.in or_a.nx.in
                 or_b.nx.in or_c.nx.in or_d.nx.in or_e.nx.in or_f.nx.in
                 output_a.nx.in pop_a.nx.in pop_b.nx.in put_a.nx.in put_b.nx.in
                 put_c.nx.in put_d.nx.in put_e.nx.in put_f.nx.in put_g.nx.in
                 put_h.nx.in putinterval_a.nx.in putinterval_b.nx.in
                 putinterval_c.nx.in putinterval_d.nx.in putinterval_e.nx.in
                 putinterval_f.nx.in putinterval_g.nx.in putinterval_h.nx.in
                 putinterval_i.nx.in repeat_a.nx.in repeat_b.nx.in
                 repeat_c.nx.in repeat_d.nx.in roll_a.nx.in roll_b.nx.in
                 roll_c.nx.in roll_d.nx.in roll_e.nx.in roll_f.nx.in
                 roll_g.nx.in sclear_a.nx.in sclear_b.nx.in sclear_c.nx.in
                 scleartomark_a.nx.in scleartomark_b.nx.in scleartomark_c.nx.in
                 scleartomark_d.nx.in scount_a.nx.in scount_b.nx.in
                 scount_c.nx.in scounttomark_a.nx.in scounttomark_b.nx.in
                 scounttomark_c.nx.in sdup_a.nx.in sdup_b.nx.in sdup_c.nx.in
                 sdup_d.nx.in search.nx.in sexch_a.nx.in sexch_b.nx.in
                 sexch_c.nx.in sexch_d.nx.in shift_a.nx.in shift_b.nx.in
                 shift_c.nx.in shift_d.nx.in sindex_a.nx.in sindex_b.nx.in
                 sindex_c.nx.in sindex_d.nx.in sindex_e.nx.in sindex_f.nx.in
                 sindex_g.nx.in spop_a.nx.in spop_b.nx.in spop_c.nx.in
                 spop_d.nx.in sprint_a.nx.in spush_a.nx.in spush_b.nx.in
                 spush_c.nx.in spush_d.nx.in sroll_a.nx.in sroll_b.nx.in
                 sroll_c.nx.in sroll_d.nx.in sroll_e.nx.in sroll_f.nx.in
                 sroll_g.nx.in sroll_h.nx.in sroll_i.nx.in stack_a.nx.in
                 start_a.nx.in start_b.nx.in stopped_a.nx.in stopped_b.nx.in
                 stopped_c.nx.in string_a.nx.in string_b.nx.in string_c.nx.in
                 string_d.nx.in sub_a.nx.in sub_b.nx.in sub_c.nx.in sub_d.nx.in
                 sym_lp_a.nx.in sym_gt_a.nx.in sym_gt_b.nx.in sym_gt_c.nx.in
                 sym_lt_a.nx.in sym_rp_a.nx.in sym_rp_b.nx.in
                 syntaxerror_a.nx.in syntaxerror_b.nx.in syntaxerror_c.nx.in
                 syntaxerror_d.nx.in syntaxerror_e.nx.in syntaxerror_f.nx.in
                 syntaxerror_g.nx.in syntaxerror_h.nx.in syntaxerror_i.nx.in
                 syntaxerror_j.nx.in syntaxerror_k.nx.in throw_a.nx.in
                 throw_b.nx.in throw_c.nx.in token_a.nx.in token_b.nx.in
                 token_c.nx.in type_a.nx.in type_b.nx.in undef_a.nx.in
                 undef_b.nx.in undef_c.nx.in where_a.nx.in where_b.nx.in
                 xcheck_a.nx.in xcheck_b.nx.in xor_a.nx.in xor_b.nx.in
                 xor_c.nx.in xor_d.nx.in xor_e.nx.in xor_f.nx.in

#endif
                 ;

  otests = [fromto % [srcdir]/bin/onyx/test/% [otests]];

  OTESTS = [OTESTS] [otests];
}

/* LaTeX source files. */
{
  local tex_srcs = ;
  tex_srcs = [fromto % [srcdir]/bin/onyx/doc/latex/% [tex_srcs]];
  tex_srcs = [tex_srcs] [objdir]/bin/onyx/doc/latex/manual.tex;

  TEX_SRCS = [TEX_SRCS] [tex_srcs];
}

/* Binaries. */
ONYX = [objdir]/bin/onyx/bin/onyx;
TONYX = [objdir]/bin/onyx/bin/tonyx;
BINS = [BINS] [ONYX] [TONYX];

[TONYX] : [ONYX];

/*
 * User cook'ables.
 */

BOOTSTRAP = [BOOTSTRAP] onyx_bootstrap;

onyx_bootstrap : [ONYX]
{
  cat [srcdir]/bin/onyx/src/onyx_nxcode.nx
    | [ONYX] [srcdir]/bin/onyx/src/onyx_nxcode_gen.nx
    > [srcdir]/bin/onyx/src/onyx_nxcode.c;

  cat [srcdir]/bin/onyx/src/batch_nxcode.nx
    | [ONYX] [srcdir]/bin/onyx/src/batch_nxcode_gen.nx
    > [srcdir]/bin/onyx/src/batch_nxcode.c;

  cat [srcdir]/bin/onyx/src/interactive_nxcode.nx
    | [ONYX] [srcdir]/bin/onyx/src/interactive_nxcode_gen.nx
    > [srcdir]/bin/onyx/src/interactive_nxcode.c;
}

onyx : [ONYX];
tonyx : [TONYX];

onyx_install : [ONYX]
#if [enable_libonyx]
               libonyx_install_s
#endif
{
  [INSTALL] -d [PREFIX]/bin;
  [CC] [A_CFLAGS] -o [PREFIX]/bin/onyx
       [fromto [srcdir]/%0%.c [objdir]/%0%.o_a [ONYX_SRCS]]
#if [enable_libedit]
       [LIBEDIT_A]
#endif
#if [enable_libonyx]
#if [matches elf [abi]]
       [RPATH][PREFIX]/lib
#endif
#endif
       [LDFLAGS] -lonyx
       [EXTRALIBS];
  [CHMOD] 0755 [PREFIX]/bin/onyx;

  [INSTALL] -d [PREFIX]/man/man1;
  [INSTALL] -m 0444 [objdir]/bin/onyx/man/man1/onyx.1 [PREFIX]/man/man1;
}

onyx_uninstall :
{
  rm -f [PREFIX]/bin/onyx;
  rm -f [PREFIX]/man/man1/onyx.1;
}

onyx_clean :
{
}

onyx_distclean :
{
  rm -f [objdir]/bin/onyx/doc/latex/manual.tex;
  rm -f [objdir]/bin/onyx/man/man1/onyx.1;
}

/*
 * Dependencies.
 */

[TONYX] : [fromto [srcdir]/%0%.c [objdir]/%0%.o_a [TONYX_SRCS]]
  set mkdir
{
  [CC] [A_CFLAGS] -o [target]
       [fromto [srcdir]/%0%.c [objdir]/%0%.o_a [TONYX_SRCS]];
}

[ONYX] : [fromto [srcdir]/%0%.c [objdir]/%0%.o_a [ONYX_SRCS]]
#if [enable_libonyx]
          [LIBONYX_S]
#endif
#if [enable_libedit]
          [LIBEDIT_A]
#endif
  set mkdir
{
  [CC] [A_CFLAGS] -o [target]
       [fromto [srcdir]/%0%.c [objdir]/%0%.o_a [ONYX_SRCS]]
#if [enable_libedit]
       [LIBEDIT_A]
#endif
#if [enable_libonyx]
#if [matches elf [abi]]
       [RPATH][abs_objdir]/lib/libonyx/lib
#endif
#endif
       [LDFLAGS] -lonyx
       [EXTRALIBS];
}