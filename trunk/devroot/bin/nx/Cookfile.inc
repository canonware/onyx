/******************************************************************************
 *
 * <Copyright = jasone>
 * <License>
 *
 ******************************************************************************
 *
 * Version: <Version>
 *
 * cook file for nx.
 *
 ******************************************************************************/

/* Include generated dependency files. */
#include-cooked [glob [objdir]/bin/nx/src/"*.d_*"]

/*
 * File lists.
 */

/* Include files that _are_ installed. */
{
  local incs = ;
  incs = [fromto % [srcdir]/bin/nx/include/% [incs]];

  INCS = [INCS] [incs];
}

/* Include files that are _not_ installed. */
{
  local priv_incs = nx.h;
  priv_incs = [fromto % [srcdir]/bin/nx/include/% [priv_incs]];
  priv_incs = [priv_incs] [objdir]/bin/nx/include/nx_defs.h;

  local priv_cincs = nx_nxcode.c;
  prix_cincs = [fromto % [srcdir]/bin/nx/src/% [priv_cincs]];

  PRIV_INCS = [PRIV_INCS] [priv_incs] [priv_cincs];
}

/* Source files. */
NX_SRCS = nx.c;
NX_SRCS = [fromto % [srcdir]/bin/nx/src/% [NX_SRCS]];
SRCS = [SRCS] [NX_SRCS];

/* Simple C tests.  Output is diff'ed with the expected (.exp file) output. */
{
  local ctests = ;
  ctests = [fromto % [srcdir]/bin/nx/test/% [ctests]];

  CTESTS = [CTESTS] [ctests];
}

/*
 * Onyx tests.
 */
{
  local otests = ;
  otests = [fromto % [srcdir]/bin/nx/test/% [otests]];

  OTESTS = [OTESTS] [otests];
}

/* Perl scripts that drive other tests and use the sequence mode of verify. */
{
  local ptests = ;
  ptests = [fromto % [srcdir]/bin/nx/test/% [ptests]];

  PTESTS = [PTESTS] [ptests];
}

/* C programs that are driven by tests. */
{
  local cbtests = ;
  cbtests = [fromto % [srcdir]/bin/nx/test/% [cbtests]];

  CBTESTS = [CBTESTS] [cbtests];
}

/* Onyx programs that are driven by tests. */
{
  local obtests = ;
  obtests = [fromto % [srcdir]/bin/nx/test/% [obtests]];

  OBTESTS = [OBTESTS] [obtests];
}

/* LaTeX source files. */
{
  local tex_srcs = ;
  tex_srcs = [fromto % [srcdir]/bin/nx/doc/latex/% [tex_srcs]];

  TEX_SRCS = [TEX_SRCS] [tex_srcs];
}

/* xfig source files. */
{
  local fig_srcs = ;
  fig_srcs = [fromto % [srcdir]/bin/nx/doc/latex/% [fig_srcs]];

  FIG_SRCS = [FIG_SRCS] [fig_srcs];
}

/* Binaries. */
NX = [objdir]/bin/nx/bin/nx;
BINS = [BINS] [NX];

/*
 * User cook'ables.
 */

BOOTSTRAP = [BOOTSTRAP] nx_bootstrap;

nx_bootstrap : [ONYX]
{
  cat [srcdir]/bin/nx/src/nx_nxcode.nx
    | [ONYX] [srcdir]/bin/nx/src/nx_nxcode_gen.nx
    > [srcdir]/bin/nx/src/nx_nxcode.c;
}

nx : [NX];

nx_install_self : [NX]
{
  [INSTALL] -d [PREFIX]/bin;
  [INSTALL] -m 0755 [NX] [PREFIX]/bin;
}

nx_install : nx_install_self;

nx_uninstall :
{
  rm -f [PREFIX]/bin/nx;
}

nx_clean :
{
}

nx_distclean :
{
  rm -f [objdir]/bin/nx/doc/latex/manual.tex;
}

/*
 * Dependencies.
 */

[NX] : [fromto [srcdir]/%0%.c [objdir]/%0%.o_a [NX_SRCS]]
         [LIBONYX_A]
         [LIBSTASH_A]
  set mkdir
{
  [CC] [A_CFLAGS] -o [target]
       [fromto [srcdir]/%0%.c [objdir]/%0%.o_a [NX_SRCS]]
       [LIBONYX_A] [LIBSTASH_A]
       [LDFLAGS] [EXTRALIBS];
}
