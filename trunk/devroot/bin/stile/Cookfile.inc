/******************************************************************************
 *
 * <Copyright = jasone>
 * <License>
 *
 ******************************************************************************
 *
 * Version: <Version>
 *
 * cook file for stile.
 *
 ******************************************************************************/

/* Include generated dependency files. */
#include-cooked [glob [objdir]/bin/stile/src/"*.d_*"]

/*
 * File lists.
 */

/* Include files that _are_ installed. */
{
  local incs = ;
  incs = [fromto % [srcdir]/bin/stile/include/% [incs]];

  INCS = [INCS] [incs];
}

/* Include files that are _not_ installed. */
{
  local priv_incs = ;
  priv_incs = [fromto % [srcdir]/bin/stile/include/% [priv_incs]];
  priv_incs = [priv_incs] [objdir]/bin/stile/include/stile_defs.h;

  PRIV_INCS = [PRIV_INCS] [priv_incs];
}

/* Source files. */
STILE_SRCS = stile.c;
STILE_SRCS = [fromto % [srcdir]/bin/stile/src/% [STILE_SRCS]];
SRCS = [SRCS] [STILE_SRCS];

/* Simple C tests.  Output is diff'ed with the expected (.exp file) output. */
{
  local ctests = ;
  ctests = [fromto % [srcdir]/bin/stile/test/% [ctests]];

  CTESTS = [CTESTS] [ctests];
}

/*
 * Simple stil tests.  Output is diff'ed with the expected (.exp file)
 * output.
 */
{
  local stests = ;
  stests = [fromto % [srcdir]/bin/stile/test/% [stests]];

  STESTS = [STESTS] [stests];
}

/* Perl scripts that drive other tests and use the sequence mode of verify. */
{
  local ptests = ;
  ptests = [fromto % [srcdir]/bin/stile/test/% [ptests]];

  PTESTS = [PTESTS] [ptests];
}

/* C programs that are driven by tests. */
{
  local cbtests = ;
  cbtests = [fromto % [srcdir]/bin/stile/test/% [cbtests]];

  CBTESTS = [CBTESTS] [cbtests];
}

/* stil programs that are driven by tests. */
{
  local sbtests = ;
  sbtests = [fromto % [srcdir]/bin/stile/test/% [sbtests]];

  SBTESTS = [SBTESTS] [sbtests];
}

/* LaTeX source files. */
{
  local tex_srcs = ;
  tex_srcs = [fromto % [srcdir]/bin/stile/doc/latex/% [tex_srcs]];
  tex_srcs = [tex_srcs] [objdir]/bin/stile/doc/latex/manual.tex;

  TEX_SRCS = [TEX_SRCS] [tex_srcs];
}

/* xfig source files. */
{
  local fig_srcs = ;
  fig_srcs = [fromto % [srcdir]/bin/stile/doc/latex/% [fig_srcs]];

  FIG_SRCS = [FIG_SRCS] [fig_srcs];
}

/* Binaries. */
STILE = [objdir]/bin/stile/bin/stile;
STILE_D = [STILE].dbg;
STILE_A = [STILE];
STILE_P = [STILE].prof;

D_BINS = [D_BINS] [STILE_D];
A_BINS = [A_BINS] [STILE_A];
P_BINS = [P_BINS] [STILE_P];

/*
 * User cook'ables.
 */

stile_all : stile_d stile_a stile_p;
stile_d : [STILE_D];
stile_a : [STILE_A];
stile_p : [STILE_P];

stile_install : stile_install_d stile_install_a stile_install_p;

stile_uninstall :
{
  [SHELL] -e;
data
#![SHELL]

echo rm -f [PREFIX]/bin/stile.dbg
rm -f [PREFIX]/bin/stile.dbg

echo rm -f [PREFIX]/bin/stile
rm -f [PREFIX]/bin/stile

echo rm -f [PREFIX]/bin/stile.prof
rm -f [PREFIX]/bin/stile.prof
dataend
}

stile_clean :
{
}

stile_distclean :
{
  rm -f [objdir]/bin/stile/doc/latex/manual.tex;
}

/*
 * Build rules.
 */

stile_install_common :
{
  [SHELL] -e;
data
#![SHELL]

echo [INSTALL] -d [PREFIX]/bin
[INSTALL] -d [PREFIX]/bin
dataend
}

stile_install_d : [STILE_D] stile_install_common
{
	[INSTALL] -m 0755 [STILE_D] [PREFIX]/bin;
}

stile_install_a : [STILE_A] stile_install_common
#if [ENABLE_SHARED]
                  libstil_install_s libstash_install_s
#endif
{
#if [ENABLE_SHARED]
  [CC] [D_CFLAGS] -o [PREFIX]/bin/stile
       [fromto [srcdir]/%0%.c [objdir]/%0%.o_a [STILE_SRCS]]
       -rpath [PREFIX]/lib
       [LDFLAGS] -lstil -lstash [EXTRALIBS];
  [CHMOD] 0755 [PREFIX]/bin/stile;
#else
	[INSTALL] -m 0755 [STILE_A] [PREFIX]/bin;
#endif
}

stile_install_p : [STILE_P] stile_install_common
{
	[INSTALL] -m 0755 [STILE_P] [PREFIX]/bin;
}

/*
 * Dependencies.
 */

[STILE_D] : [fromto [srcdir]/%0%.c [objdir]/%0%.o_d [STILE_SRCS]]
           [LIBSTIL_D] [LIBSTASH_D]
  set mkdir
{
  [CC] [D_CFLAGS] -o [target]
       [fromto [srcdir]/%0%.c [objdir]/%0%.o_d [STILE_SRCS]]
       [LDFLAGS] -lstil_d -lstash_d [EXTRALIBS];
}

[STILE_A] : [fromto [srcdir]/%0%.c [objdir]/%0%.o_a [STILE_SRCS]]
#if [ENABLE_SHARED]
           [LIBSTIL_S] [LIBSTASH_S]
#else
           [LIBSTIL_A] [LIBSTASH_A]
#endif
  set mkdir
{
  [CC] [A_CFLAGS] -o [target]
       [fromto [srcdir]/%0%.c [objdir]/%0%.o_a [STILE_SRCS]]
#if [ENABLE_SHARED]
       -rpath [objdir]/lib/c/libstil/lib
       -rpath [objdir]/lib/c/libstash/lib
#endif
       [LDFLAGS] -lstil -lstash [EXTRALIBS];
}

[STILE_P] : [fromto [srcdir]/%0%.c [objdir]/%0%.o_p [STILE_SRCS]]
           [LIBSTIL_P] [LIBSTASH_P]
  set mkdir
{
  [CC] [P_CFLAGS] -o [target]
       [fromto [srcdir]/%0%.c [objdir]/%0%.o_p [STILE_SRCS]]
       [LDFLAGS] -lstil_p -lstash_p [EXTRALIBS];
}
