/******************************************************************************
 *
 * <Copyright = jasone>
 * <License>
 *
 ******************************************************************************
 *
 * Version: <Version>
 *
 * cook file for cover.
 *
 ******************************************************************************/

/* Include generated dependency files. */
#include-cooked [glob [objdir]/bin/cover/src/"*.d_*"]

/*
 * File lists.
 */

/* Include files that _are_ installed. */
{
  local incs = ;
  incs = [fromto % [srcdir]/bin/cover/include/% [incs]];

  INCS = [INCS] [incs];
}

/* Include files that are _not_ installed. */
{
  local priv_incs = cover.h gene.h matrix.h pack.h;
  priv_incs = [fromto % [srcdir]/bin/cover/include/% [priv_incs]];
  priv_incs = [priv_incs] [objdir]/bin/cover/include/cover_defs.h;

  PRIV_INCS = [PRIV_INCS] [priv_incs];
}

/* Source files. */
COVER_SRCS = cover.c matrix.c;
COVER_SRCS = [fromto % [srcdir]/bin/cover/src/% [COVER_SRCS]];
SRCS = [SRCS] [COVER_SRCS];

/* Simple C tests.  Output is diff'ed with the expected (.exp file) output. */
{
  local ctests = ;
  ctests = [fromto % [srcdir]/bin/cover/test/% [ctests]];

  CTESTS = [CTESTS] [ctests];
}

/*
 * Simple stil tests.  Output is diff'ed with the expected (.exp file)
 * output.
 */
{
  local stests = ;
  stests = [fromto % [srcdir]/bin/cover/test/% [stests]];

  STESTS = [STESTS] [stests];
}

/* Perl scripts that drive other tests and use the sequence mode of verify. */
{
  local ptests = ;
  ptests = [fromto % [srcdir]/bin/cover/test/% [ptests]];

  PTESTS = [PTESTS] [ptests];
}

/* C programs that are driven by tests. */
{
  local cbtests = ;
  cbtests = [fromto % [srcdir]/bin/cover/test/% [cbtests]];

  CBTESTS = [CBTESTS] [cbtests];
}

/* stil programs that are driven by tests. */
{
  local sbtests = ;
  sbtests = [fromto % [srcdir]/bin/cover/test/% [sbtests]];

  SBTESTS = [SBTESTS] [sbtests];
}

/* LaTeX source files. */
{
  local tex_srcs = ;
  tex_srcs = [fromto % [srcdir]/bin/cover/doc/latex/% [tex_srcs]];

  TEX_SRCS = [TEX_SRCS] [tex_srcs];
}

/* xfig source files. */
{
  local fig_srcs = ;
  fig_srcs = [fromto % [srcdir]/bin/cover/doc/latex/% [fig_srcs]];

  FIG_SRCS = [FIG_SRCS] [fig_srcs];
}

/* Binaries. */
COVER = [objdir]/bin/cover/bin/cover;
COVER_D = [COVER].dbg;
COVER_A = [COVER];
COVER_P = [COVER].prof;

D_BINS = [D_BINS] [COVER_D];
A_BINS = [A_BINS] [COVER_A];
P_BINS = [P_BINS] [COVER_P];

/*
 * User cook'ables.
 */

cover_all : cover_d cover_a cover_p;
cover_d : [COVER_D];
cover_a : [COVER_A];
cover_p : [COVER_P];

cover_install : cover_install_d cover_install_a cover_install_p;

cover_uninstall :
{
  [SHELL] -e;
data
#![SHELL]

echo rm -f [PREFIX]/bin/cover.dbg
rm -f [PREFIX]/bin/cover.dbg

echo rm -f [PREFIX]/bin/cover
rm -f [PREFIX]/bin/cover

echo rm -f [PREFIX]/bin/cover.prof
rm -f [PREFIX]/bin/cover.prof
dataend
}

cover_clean :
{
}

cover_distclean :
{
}

/*
 * Build rules.
 */

cover_install_common :
{
  [SHELL] -e;
data
#![SHELL]

echo [INSTALL] -d [PREFIX]/bin
[INSTALL] -d [PREFIX]/bin
dataend
}

cover_install_d : [COVER_D] cover_install_common
{
	[INSTALL] -m 0755 [COVER_D] [PREFIX]/bin;
}

cover_install_a : [COVER_A] cover_install_common
#if [ENABLE_SHARED]
                  libstash_install_s
#endif
{
#if [ENABLE_SHARED]
  [CC] [D_CFLAGS] -o [PREFIX]/bin/cover
       [fromto [srcdir]/%0%.c [objdir]/%0%.o_a [COVER_SRCS]]
       -rpath [PREFIX]/lib
       [LDFLAGS] -lstash [EXTRALIBS];
  [CHMOD] 0755 [PREFIX]/bin/cover;
#else
	[INSTALL] -m 0755 [COVER_A] [PREFIX]/bin;
#endif
}

cover_install_p : [COVER_P] cover_install_common
{
	[INSTALL] -m 0755 [COVER_P] [PREFIX]/bin;
}

/*
 * Dependencies.
 */

[COVER_D] : [fromto [srcdir]/%0%.c [objdir]/%0%.o_d [COVER_SRCS]]
           [LIBSTASH_D]
  set mkdir
{
  [CC] [D_CFLAGS] -o [target]
       [fromto [srcdir]/%0%.c [objdir]/%0%.o_d [COVER_SRCS]]
       [LDFLAGS] -lstash_d [EXTRALIBS];
}

[COVER_A] : [fromto [srcdir]/%0%.c [objdir]/%0%.o_a [COVER_SRCS]]
#if [ENABLE_SHARED]
           [LIBSTASH_S]
#else
           [LIBSTASH_A]
#endif
  set mkdir
{
  [CC] [A_CFLAGS] -o [target]
       [fromto [srcdir]/%0%.c [objdir]/%0%.o_a [COVER_SRCS]]
#if [ENABLE_SHARED]
       -rpath [objdir]/lib/c/libstash/lib
#endif
       [LDFLAGS] -lstash [EXTRALIBS];
}

[COVER_P] : [fromto [srcdir]/%0%.c [objdir]/%0%.o_p [COVER_SRCS]]
           [LIBSTASH_P]
  set mkdir
{
  [CC] [P_CFLAGS] -o [target]
       [fromto [srcdir]/%0%.c [objdir]/%0%.o_p [COVER_SRCS]]
       [LDFLAGS] -lstash_p [EXTRALIBS];
}
