/******************************************************************************
 *
 * <Copyright = jasone>
 * <License>
 *
 ******************************************************************************
 *
 * Version: <Version>
 *
 * cook file for stil.
 *
 ******************************************************************************/

/* Include generated dependency files. */
#include-cooked [glob [objdir]/bin/stil/src/"*.d_*"]

/*
 * File lists.
 */

/* Include files that _are_ installed. */
{
  local incs = ;
  incs = [fromto % [srcdir]/bin/stil/include/% [incs]];

  INCS = [INCS] [incs];
}

/* Include files that are _not_ installed. */
{
  local priv_incs = ;
  priv_incs = [fromto % [srcdir]/bin/stil/include/% [priv_incs]];
  priv_incs = [priv_incs] [objdir]/bin/stil/include/stil_defs.h;

  PRIV_INCS = [PRIV_INCS] [priv_incs];
}

/* Source files. */
STIL_SRCS = stil.c;
STIL_SRCS = [fromto % [srcdir]/bin/stil/src/% [STIL_SRCS]];
SRCS = [SRCS] [STIL_SRCS];

/* Simple C tests.  Output is diff'ed with the expected (.exp file) output. */
{
  local ctests = ;
  ctests = [fromto % [srcdir]/bin/stil/test/% [ctests]];

  CTESTS = [CTESTS] [ctests];
}

/*
 * Simple stil tests.  Output is diff'ed with the expected (.exp file)
 * output.
 */
{
  local stests = b64scan.st.in copy_a.st.in eval.st.in gc_roll.st.in hello.st.in
                 integer.st.in sclear_a.st.in sclear_b.st.in sclear_c.st.in
                 scleartomark_a.st.in scleartomark_b.st.in scleartomark_c.st.in
                 scleartomark_d.st.in scount_a.st.in scount_b.st.in
                 scount_c.st.in scounttomark_a.st.in scounttomark_b.st.in
                 scounttomark_c.st.in sdup_a.st.in sdup_b.st.in sdup_c.st.in
                 sdup_d.st.in search.st.in sindex_a.st.in sindex_b.st.in
                 sindex_c.st.in sindex_d.st.in sindex_e.st.in sindex_f.st.in
                 sindex_g.st.in spop_a.st.in spop_b.st.in spop_c.st.in
                 spop_d.st.in sprint_a.st.in spush_a.st.in spush_b.st.in
                 spush_c.st.in spush_d.st.in sroll_a.st.in sroll_b.st.in
                 sroll_c.st.in sroll_d.st.in sroll_e.st.in sroll_f.st.in
                 sroll_g.st.in sroll_h.st.in sroll_i.st.in syntaxerror_a.st.in
                 syntaxerror_b.st.in syntaxerror_c.st.in syntaxerror_d.st.in
                 syntaxerror_e.st.in syntaxerror_f.st.in syntaxerror_g.st.in
                 syntaxerror_h.st.in syntaxerror_i.st.in syntaxerror_j.st.in
                 syntaxerror_k.st.in syntaxerror_l.st.in syntaxerror_m.st.in
                 syntaxerror_n.st.in syntaxerror_o.st.in syntaxerror_p.st.in
                 syntaxerror_q.st.in syntaxerror_r.st.in syntaxerror_s.st.in
                 syntaxerror_t.st.in;
  stests = [fromto % [srcdir]/bin/stil/test/% [stests]];

  STESTS = [STESTS] [stests];
}

/* Perl scripts that drive other tests and use the sequence mode of verify. */
{
  local ptests = ;
  ptests = [fromto % [srcdir]/bin/stil/test/% [ptests]];

  PTESTS = [PTESTS] [ptests];
}

/* C programs that are driven by tests. */
{
  local cbtests = ;
  cbtests = [fromto % [srcdir]/bin/stil/test/% [cbtests]];

  CBTESTS = [CBTESTS] [cbtests];
}

/* stil programs that are driven by tests. */
{
  local sbtests = ;
  sbtests = [fromto % [srcdir]/bin/stil/test/% [sbtests]];

  SBTESTS = [SBTESTS] [sbtests];
}

/* LaTeX source files. */
{
  local tex_srcs = ;
  tex_srcs = [fromto % [srcdir]/bin/stil/doc/latex/% [tex_srcs]];
  tex_srcs = [tex_srcs] [objdir]/bin/stil/doc/latex/manual.tex;

  TEX_SRCS = [TEX_SRCS] [tex_srcs];
}

/* xfig source files. */
{
  local fig_srcs = ;
  fig_srcs = [fromto % [srcdir]/bin/stil/doc/latex/% [fig_srcs]];

  FIG_SRCS = [FIG_SRCS] [fig_srcs];
}

/* Binaries. */
STIL = [objdir]/bin/stil/bin/stil;
STIL_D = [STIL].dbg;
STIL_A = [STIL];
STIL_P = [STIL].prof;

D_BINS = [D_BINS] [STIL_D];
A_BINS = [A_BINS] [STIL_A];
P_BINS = [P_BINS] [STIL_P];

/*
 * User cook'ables.
 */

stil_all : stil_d stil_a stil_p;
stil_d : [STIL_D];
stil_a : [STIL_A];
stil_p : [STIL_P];

stil_install : stil_install_d stil_install_a stil_install_p;

stil_uninstall :
{
  [SHELL] -e;
data
#![SHELL]

echo rm -f [PREFIX]/bin/stil.dbg
rm -f [PREFIX]/bin/stil.dbg

echo rm -f [PREFIX]/bin/stil
rm -f [PREFIX]/bin/stil

echo rm -f [PREFIX]/bin/stil.prof
rm -f [PREFIX]/bin/stil.prof
dataend
}

stil_clean :
{
}

stil_distclean :
{
  rm -f [objdir]/bin/stil/doc/latex/manual.tex;
}

/*
 * Build rules.
 */

stil_install_common :
{
  [SHELL] -e;
data
#![SHELL]

echo [INSTALL] -d [PREFIX]/bin
[INSTALL] -d [PREFIX]/bin
dataend
}

stil_install_d : [STIL_D] stil_install_common
{
	[INSTALL] -m 0755 [STIL_D] [PREFIX]/bin;
}

stil_install_a : [STIL_A] stil_install_common
#if [ENABLE_SHARED]
                 libstil_install_s libedit_install_s libstash_install_s
#endif
{
#if [ENABLE_SHARED]
  [CC] [A_CFLAGS] -o [PREFIX]/bin/stil
       [fromto [srcdir]/%0%.c [objdir]/%0%.o_a [STIL_SRCS]]
       -rpath [PREFIX]/lib
       [LDFLAGS] -lstil -ledit -lstash -ltermcap [EXTRALIBS];
  [CHMOD] 0755 [PREFIX]/bin/stil;
#else
	[INSTALL] -m 0755 [STIL_A] [PREFIX]/bin;
#endif
}

stil_install_p : [STIL_P] stil_install_common
{
	[INSTALL] -m 0755 [STIL_P] [PREFIX]/bin;
}

/*
 * Dependencies.
 */

[STIL_D] : [fromto [srcdir]/%0%.c [objdir]/%0%.o_d [STIL_SRCS]]
           [LIBSTIL_D] [LIBEDIT_D] [LIBSTASH_D]
  set mkdir
{
  [CC] [D_CFLAGS] -o [target]
       [fromto [srcdir]/%0%.c [objdir]/%0%.o_d [STIL_SRCS]]
       [LDFLAGS] -lstil_d -ledit_d -lstash_d -ltermcap [EXTRALIBS];
}

[STIL_A] : [fromto [srcdir]/%0%.c [objdir]/%0%.o_a [STIL_SRCS]]
#if [ENABLE_SHARED]
           [LIBSTIL_S] [LIBEDIT_S] [LIBSTASH_S]
#else
           [LIBSTIL_A] [LIBEDIT_A] [LIBSTASH_A]
#endif
  set mkdir
{
  [CC] [A_CFLAGS] -o [target]
       [fromto [srcdir]/%0%.c [objdir]/%0%.o_a [STIL_SRCS]]
#if [ENABLE_SHARED]
       -rpath [abs_objdir]/lib/c/libstil/lib
       -rpath [abs_objdir]/lib/c/libedit/lib
       -rpath [abs_objdir]/lib/c/libstash/lib
#endif
       [LDFLAGS] -lstil -ledit -lstash -ltermcap [EXTRALIBS];
}

[STIL_P] : [fromto [srcdir]/%0%.c [objdir]/%0%.o_p [STIL_SRCS]]
           [LIBSTIL_P] [LIBEDIT_P] [LIBSTASH_P]
  set mkdir
{
  [CC] [P_CFLAGS] -o [target]
       [fromto [srcdir]/%0%.c [objdir]/%0%.o_p [STIL_SRCS]]
       [LDFLAGS] -lstil_p -ledit_p -lstash_p -ltermcap [EXTRALIBS];
}
