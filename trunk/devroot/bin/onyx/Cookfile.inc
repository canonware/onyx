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

/* Include files that _are_ installed. */
{
  local incs = ;
  incs = [fromto % [srcdir]/bin/onyx/include/% [incs]];

  INCS = [INCS] [incs];
}

/* Include files that are _not_ installed. */
{
  local priv_incs = ;
  priv_incs = [fromto % [srcdir]/bin/onyx/include/% [priv_incs]];
  priv_incs = [priv_incs] [objdir]/bin/onyx/include/onyx_defs.h;

  PRIV_INCS = [PRIV_INCS] [priv_incs];
}

/* Source files. */
ONYX_SRCS = onyx.c;
ONYX_SRCS = [fromto % [srcdir]/bin/onyx/src/% [ONYX_SRCS]];
SRCS = [SRCS] [ONYX_SRCS];

/* Simple C tests.  Output is diff'ed with the expected (.exp file) output. */
{
  local ctests = ;
  ctests = [fromto % [srcdir]/bin/onyx/test/% [ctests]];

  CTESTS = [CTESTS] [ctests];
}

/*
 * Simple onyx tests.  Output is diff'ed with the expected (.exp file)
 * output.
 */
{
  local otests = abs_a.nx.in abs_b.nx.in abs_c.nx.in add_a.nx.in add_b.nx.in
                 add_c.nx.in add_d.nx.in bind_a.nx.in
                 catenate_a.nx.in catenate_b.nx.in catenate_c.nx.in
                 catenate_d.nx.in clear_a.nx.in cleartomark_a.nx.in copy_a.nx.in
                 count_a.nx.in counttomark_a.nx.in counttomark_b.nx.in
                 counttomark_c.nx.in div_a.nx.in div_b.nx.in div_c.nx.in
                 div_d.nx.in div_e.nx.in dup_a.nx.in dup_b.nx.in eval.nx.in
                 exch_a.nx.in exch_b.nx.in exch_c.nx.in exp_a.nx.in exp_b.nx.in
                 exp_c.nx.in exp_d.nx.in exp_e.nx.in fino_a.nx.in
                 gc_roll.nx.in hello.nx.in immediate_a.nx.in immediate_b.nx.in
                 index_a.nx.in index_b.nx.in index_c.nx.in index_d.nx.in
                 index_e.nx.in index_f.nx.in integer.nx.in integer_a.nx.in
                 integer_b.nx.in integer_c.nx.in integer_d.nx.in mark_a.nx.in
                 mod_a.nx.in mod_b.nx.in mod_c.nx.in mod_d.nx.in mod_e.nx.in
                 monitor_a.nx.in monitor_b.nx.in monitor_c.nx.in mul_a.nx.in
                 mul_b.nx.in mul_c.nx.in mul_d.nx.in neg_a.nx.in neg_b.nx.in
                 neg_c.nx.in nsleep_a.nx.in
                 nsleep_b.nx.in nsleep_c.nx.in nsleep_d.nx.in pop_a.nx.in
                 pop_b.nx.in rand_a.nx.in roll_a.nx.in roll_b.nx.in
                 roll_c.nx.in roll_d.nx.in
                 roll_e.nx.in roll_f.nx.in roll_g.nx.in sclear_a.nx.in
                 sclear_b.nx.in sclear_c.nx.in scleartomark_a.nx.in
                 scleartomark_b.nx.in scleartomark_c.nx.in scleartomark_d.nx.in
                 scount_a.nx.in scount_b.nx.in scount_c.nx.in
                 scounttomark_a.nx.in scounttomark_b.nx.in scounttomark_c.nx.in
                 sdup_a.nx.in sdup_b.nx.in sdup_c.nx.in sdup_d.nx.in
                 search.nx.in sexch_a.nx.in sexch_b.nx.in sexch_c.nx.in
                 sexch_d.nx.in sindex_a.nx.in sindex_b.nx.in sindex_c.nx.in
                 sindex_d.nx.in sindex_e.nx.in sindex_f.nx.in sindex_g.nx.in
                 spop_a.nx.in spop_b.nx.in spop_c.nx.in spop_d.nx.in
                 sprint_a.nx.in spush_a.nx.in spush_b.nx.in spush_c.nx.in
                 spush_d.nx.in srand_a.nx.in srand_b.nx.in srand_c.nx.in
                 srand_d.nx.in sroll_a.nx.in sroll_b.nx.in sroll_c.nx.in
                 sroll_d.nx.in sroll_e.nx.in sroll_f.nx.in sroll_g.nx.in
                 sroll_h.nx.in sroll_i.nx.in stack_a.nx.in sub_a.nx.in
                 sub_b.nx.in sub_c.nx.in sub_d.nx.in sym_lp_a.nx.in
                 sym_rp_a.nx.in sym_rp_b.nx.in syntaxerror_a.nx.in
                 syntaxerror_b.nx.in syntaxerror_c.nx.in syntaxerror_d.nx.in
                 syntaxerror_e.nx.in syntaxerror_f.nx.in syntaxerror_g.nx.in
                 syntaxerror_h.nx.in syntaxerror_i.nx.in syntaxerror_j.nx.in
                 syntaxerror_k.nx.in;

  otests = [fromto % [srcdir]/bin/onyx/test/% [otests]];

  OTESTS = [OTESTS] [otests];
}

/* Perl scripts that drive other tests and use the sequence mode of verify. */
{
  local ptests = ;
  ptests = [fromto % [srcdir]/bin/onyx/test/% [ptests]];

  PTESTS = [PTESTS] [ptests];
}

/* C programs that are driven by tests. */
{
  local cbtests = ;
  cbtests = [fromto % [srcdir]/bin/onyx/test/% [cbtests]];

  CBTESTS = [CBTESTS] [cbtests];
}

/* onyx programs that are driven by tests. */
{
  local obtests = ;
  obtests = [fromto % [srcdir]/bin/onyx/test/% [obtests]];

  OBTESTS = [OBTESTS] [obtests];
}

/* LaTeX source files. */
{
  local tex_srcs = ;
  tex_srcs = [fromto % [srcdir]/bin/onyx/doc/latex/% [tex_srcs]];
  tex_srcs = [tex_srcs] [objdir]/bin/onyx/doc/latex/manual.tex;

  TEX_SRCS = [TEX_SRCS] [tex_srcs];
}

/* xfig source files. */
{
  local fig_srcs = ;
  fig_srcs = [fromto % [srcdir]/bin/onyx/doc/latex/% [fig_srcs]];

  FIG_SRCS = [FIG_SRCS] [fig_srcs];
}

/* Binaries. */
ONYX = [objdir]/bin/onyx/bin/onyx;
ONYX_D = [ONYX].dbg;
ONYX_A = [ONYX];
ONYX_P = [ONYX].prof;

D_BINS = [D_BINS] [ONYX_D];
A_BINS = [A_BINS] [ONYX_A];
P_BINS = [P_BINS] [ONYX_P];

/*
 * User cook'ables.
 */

onyx_all : onyx_d onyx_a onyx_p;
onyx_d : [ONYX_D];
onyx_a : [ONYX_A];
onyx_p : [ONYX_P];

onyx_install : onyx_install_d onyx_install_a onyx_install_p;

onyx_uninstall :
{
  [SHELL] -e;
data
#![SHELL]

echo rm -f [PREFIX]/bin/onyx.dbg
rm -f [PREFIX]/bin/onyx.dbg

echo rm -f [PREFIX]/bin/onyx
rm -f [PREFIX]/bin/onyx

echo rm -f [PREFIX]/bin/onyx.prof
rm -f [PREFIX]/bin/onyx.prof
dataend
}

onyx_clean :
{
}

onyx_distclean :
{
  rm -f [objdir]/bin/onyx/doc/latex/manual.tex;
}

/*
 * Build rules.
 */

onyx_install_common :
{
  [SHELL] -e;
data
#![SHELL]

echo [INSTALL] -d [PREFIX]/bin
[INSTALL] -d [PREFIX]/bin
dataend
}

onyx_install_d : [ONYX_D] onyx_install_common
{
	[INSTALL] -m 0755 [ONYX_D] [PREFIX]/bin;
}

onyx_install_a : [ONYX_A] onyx_install_common
#if [ENABLE_SHARED]
                 libonyx_install_s libedit_install_s libstash_install_s
#endif
{
#if [ENABLE_SHARED]
  [CC] [A_CFLAGS] -o [PREFIX]/bin/onyx
       [fromto [srcdir]/%0%.c [objdir]/%0%.o_a [ONYX_SRCS]]
       -rpath [PREFIX]/lib
       [LDFLAGS] -lonyx -ledit -lstash -ltermcap [EXTRALIBS];
  [CHMOD] 0755 [PREFIX]/bin/onyx;
#else
	[INSTALL] -m 0755 [ONYX_A] [PREFIX]/bin;
#endif
}

onyx_install_p : [ONYX_P] onyx_install_common
{
	[INSTALL] -m 0755 [ONYX_P] [PREFIX]/bin;
}

/*
 * Dependencies.
 */

[ONYX_D] : [fromto [srcdir]/%0%.c [objdir]/%0%.o_d [ONYX_SRCS]]
           [LIBONYX_D] [LIBEDIT_D] [LIBSTASH_D]
  set mkdir
{
  [CC] [D_CFLAGS] -o [target]
       [fromto [srcdir]/%0%.c [objdir]/%0%.o_d [ONYX_SRCS]]
       [LDFLAGS] -lonyx_d -ledit_d -lstash_d -ltermcap [EXTRALIBS];
}

[ONYX_A] : [fromto [srcdir]/%0%.c [objdir]/%0%.o_a [ONYX_SRCS]]
#if [ENABLE_SHARED]
           [LIBONYX_S] [LIBEDIT_S] [LIBSTASH_S]
#else
           [LIBONYX_A] [LIBEDIT_A] [LIBSTASH_A]
#endif
  set mkdir
{
  [CC] [A_CFLAGS] -o [target]
       [fromto [srcdir]/%0%.c [objdir]/%0%.o_a [ONYX_SRCS]]
#if [ENABLE_SHARED]
       -rpath [abs_objdir]/lib/c/libonyx/lib
       -rpath [abs_objdir]/lib/c/libedit/lib
       -rpath [abs_objdir]/lib/c/libstash/lib
#endif
       [LDFLAGS] -lonyx -ledit -lstash -ltermcap [EXTRALIBS];
}

[ONYX_P] : [fromto [srcdir]/%0%.c [objdir]/%0%.o_p [ONYX_SRCS]]
           [LIBONYX_P] [LIBEDIT_P] [LIBSTASH_P]
  set mkdir
{
  [CC] [P_CFLAGS] -o [target]
       [fromto [srcdir]/%0%.c [objdir]/%0%.o_p [ONYX_SRCS]]
       [LDFLAGS] -lonyx_p -ledit_p -lstash_p -ltermcap [EXTRALIBS];
}
