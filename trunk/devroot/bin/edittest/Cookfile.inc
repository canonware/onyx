/******************************************************************************
 *
 * <Copyright = jasone>
 * <License>
 *
 ******************************************************************************
 *
 * Version: <Version>
 *
 * cook file for edittest.
 *
 ******************************************************************************/

/* Include generated dependency files. */
#include-cooked [glob [objdir]/bin/edittest/src/"*.d_*"]

/*
 * File lists.
 */

/* Include files that _are_ installed. */
{
  local incs = ;
  incs = [fromto % [srcdir]/bin/edittest/include/% [incs]];

  INCS = [INCS] [incs];
}

/* Include files that are _not_ installed. */
{
  local priv_incs = ;
  priv_incs = [fromto % [srcdir]/bin/edittest/include/% [priv_incs]];
  priv_incs = [priv_incs] [objdir]/bin/edittest/include/edittest_defs.h;

  PRIV_INCS = [PRIV_INCS] [priv_incs];
}

/* Source files. */
EDITTEST_SRCS = edittest.c;
EDITTEST_SRCS = [fromto % [srcdir]/bin/edittest/src/% [EDITTEST_SRCS]];
SRCS = [SRCS] [EDITTEST_SRCS];

/* Simple C tests.  Output is diff'ed with the expected (.exp file) output. */
{
  local ctests = ;
  ctests = [fromto % [srcdir]/bin/edittest/test/% [ctests]];

  CTESTS = [CTESTS] [ctests];
}

/*
 * Simple stil tests.  Output is diff'ed with the expected (.exp file)
 * output.
 */
{
  local stests = ;
  stests = [fromto % [srcdir]/bin/edittest/test/% [stests]];

  STESTS = [STESTS] [stests];
}

/* Perl scripts that drive other tests and use the sequence mode of verify. */
{
  local ptests = ;
  ptests = [fromto % [srcdir]/bin/edittest/test/% [ptests]];

  PTESTS = [PTESTS] [ptests];
}

/* C programs that are driven by tests. */
{
  local cbtests = ;
  cbtests = [fromto % [srcdir]/bin/edittest/test/% [cbtests]];

  CBTESTS = [CBTESTS] [cbtests];
}

/* stil programs that are driven by tests. */
{
  local sbtests = ;
  sbtests = [fromto % [srcdir]/bin/edittest/test/% [sbtests]];

  SBTESTS = [SBTESTS] [sbtests];
}

/* LaTeX source files. */
{
  local tex_srcs = ;
  tex_srcs = [fromto % [srcdir]/bin/edittest/doc/latex/% [tex_srcs]];

  TEX_SRCS = [TEX_SRCS] [tex_srcs];
}

/* xfig source files. */
{
  local fig_srcs = ;
  fig_srcs = [fromto % [srcdir]/bin/edittest/doc/latex/% [fig_srcs]];

  FIG_SRCS = [FIG_SRCS] [fig_srcs];
}

/* Binaries. */
EDITTEST = [objdir]/bin/edittest/bin/edittest;
EDITTEST_D = [EDITTEST].dbg;
EDITTEST_A = [EDITTEST];
EDITTEST_P = [EDITTEST].prof;

D_BINS = [D_BINS] [EDITTEST_D];
A_BINS = [A_BINS] [EDITTEST_A];
P_BINS = [P_BINS] [EDITTEST_P];

/*
 * User cook'ables.
 */

edittest_all : edittest_d edittest_a edittest_p;
edittest_d : [EDITTEST_D];
edittest_a : [EDITTEST_A];
edittest_p : [EDITTEST_P];

edittest_install : edittest_install_d edittest_install_a edittest_install_p;

edittest_uninstall :
{
  [SHELL] -e;
data
#![SHELL]

echo rm -f [PREFIX]/bin/edittest.dbg
rm -f [PREFIX]/bin/edittest.dbg

echo rm -f [PREFIX]/bin/edittest
rm -f [PREFIX]/bin/edittest

echo rm -f [PREFIX]/bin/edittest.prof
rm -f [PREFIX]/bin/edittest.prof
dataend
}

edittest_clean :
{
}

edittest_distclean :
{
}

/*
 * Build rules.
 */

edittest_install_common :
{
  [SHELL] -e;
data
#![SHELL]

echo [INSTALL] -d [PREFIX]/bin
[INSTALL] -d [PREFIX]/bin
dataend
}

edittest_install_d : [EDITTEST_D] edittest_install_common
{
	[INSTALL] -m 0755 [EDITTEST_D] [PREFIX]/bin;
}

edittest_install_a : [EDITTEST_A] edittest_install_common
#if [ENABLE_SHARED]
                 libedittest_install_s libstash_install_s
#endif
{
#if [ENABLE_SHARED]
  [CC] [D_CFLAGS] -o [PREFIX]/bin/edittest
       [fromto [srcdir]/%0%.c [objdir]/%0%.o_a [EDITTEST_SRCS]]
       -rpath [PREFIX]/lib
       [LDFLAGS] -ledit -lstash -ltermcap [EXTRALIBS];
  [CHMOD] 0755 [PREFIX]/bin/edittest;
#else
	[INSTALL] -m 0755 [EDITTEST_A] [PREFIX]/bin;
#endif
}

edittest_install_p : [EDITTEST_P] edittest_install_common
{
	[INSTALL] -m 0755 [EDITTEST_P] [PREFIX]/bin;
}

/*
 * Dependencies.
 */

[EDITTEST_D] : [fromto [srcdir]/%0%.c [objdir]/%0%.o_d [EDITTEST_SRCS]]
           [LIBEDIT_D] [LIBSTASH_D]
  set mkdir
{
  [CC] [D_CFLAGS] -o [target]
       [fromto [srcdir]/%0%.c [objdir]/%0%.o_d [EDITTEST_SRCS]]
       [LDFLAGS] -ledit_d -lstash_d -ltermcap [EXTRALIBS];
}

[EDITTEST_A] : [fromto [srcdir]/%0%.c [objdir]/%0%.o_a [EDITTEST_SRCS]]
#if [ENABLE_SHARED]
           [LIBEDIT_S] [LIBSTASH_S]
#else
           [LIBEDIT_A] [LIBSTASH_A]
#endif
  set mkdir
{
  [CC] [A_CFLAGS] -o [target]
       [fromto [srcdir]/%0%.c [objdir]/%0%.o_a [EDITTEST_SRCS]]
#if [ENABLE_SHARED]
       -rpath [objdir]/lib/c/libedit/lib
       -rpath [objdir]/lib/c/libstash/lib
#endif
       [LDFLAGS] -ledit -lstash -ltermcap [EXTRALIBS];
}

[EDITTEST_P] : [fromto [srcdir]/%0%.c [objdir]/%0%.o_p [EDITTEST_SRCS]]
           [LIBEDIT_P] [LIBSTASH_P]
  set mkdir
{
  [CC] [P_CFLAGS] -o [target]
       [fromto [srcdir]/%0%.c [objdir]/%0%.o_p [EDITTEST_SRCS]]
       [LDFLAGS] -ledit_p -lstash_p -ltermcap [EXTRALIBS];
}
