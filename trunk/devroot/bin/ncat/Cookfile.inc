/******************************************************************************
 *
 * <Copyright = jasone>
 * <License>
 *
 ******************************************************************************
 *
 * Version: <Version>
 *
 * cook file for ncat.
 *
 ******************************************************************************/

/* Include generated dependency files. */
#include-cooked [glob [objdir]/bin/ncat/src/"*.d_*"]

/*
 * File lists.
 */

/* Include files that _are_ installed. */
{
  local incs = ;
  incs = [fromto % [srcdir]/bin/ncat/include/% [incs]];

  INCS = [INCS] [incs];
}

/* Include files that are _not_ installed. */
{
  local priv_incs = ;
  priv_incs = [fromto % [srcdir]/bin/ncat/include/% [priv_incs]];
  priv_incs = [priv_incs] [objdir]/bin/ncat/include/ncat_defs.h;

  PRIV_INCS = [PRIV_INCS] [priv_incs];
}

/* Source files. */
NCAT_SRCS = ncat.c;
NCAT_SRCS = [fromto % [srcdir]/bin/ncat/src/% [NCAT_SRCS]];
SRCS = [SRCS] [NCAT_SRCS];

/* Simple C tests.  Output is diff'ed with the expected (.exp file) output. */
{
  local ctests = ;
  ctests = [fromto % [srcdir]/bin/ncat/test/% [ctests]];

  CTESTS = [CTESTS] [ctests];
}

/*
 * Simple onyx tests.  Output is diff'ed with the expected (.exp file)
 * output.
 */
{
  local otests = ;
  otests = [fromto % [srcdir]/bin/ncat/test/% [otests]];

  OTESTS = [OTESTS] [otests];
}

/* Perl scripts that drive other tests and use the sequence mode of verify. */
{
  local ptests = ;
  ptests = [fromto % [srcdir]/bin/ncat/test/% [ptests]];

  PTESTS = [PTESTS] [ptests];
}

/* C programs that are driven by tests. */
{
  local cbtests = ;
  cbtests = [fromto % [srcdir]/bin/ncat/test/% [cbtests]];

  CBTESTS = [CBTESTS] [cbtests];
}

/* onyx programs that are driven by tests. */
{
  local obtests = ;
  obtests = [fromto % [srcdir]/bin/ncat/test/% [obtests]];

  OBTESTS = [OBTESTS] [obtests];
}

/* LaTeX source files. */
{
  local tex_srcs = ;
  tex_srcs = [fromto % [srcdir]/bin/ncat/doc/latex/% [tex_srcs]];
  tex_srcs = [tex_srcs] [objdir]/bin/ncat/doc/latex/manual.tex;

  TEX_SRCS = [TEX_SRCS] [tex_srcs];
}

/* xfig source files. */
{
  local fig_srcs = ;
  fig_srcs = [fromto % [srcdir]/bin/ncat/doc/latex/% [fig_srcs]];

  FIG_SRCS = [FIG_SRCS] [fig_srcs];
}

/* Binaries. */
NCAT = [objdir]/bin/ncat/bin/ncat;
BINS = [BINS] [NCAT];

/*
 * User cook'ables.
 */

ncat : [NCAT];

ncat_install_self : [NCAT]
{
  [INSTALL] -d [PREFIX]/bin;
  [CC] [A_CFLAGS] -o [PREFIX]/bin/ncat
       [fromto [srcdir]/%0%.c [objdir]/%0%.o_a [NCAT_SRCS]]
       -Wl,-rpath,[PREFIX]/lib
       [LDFLAGS] -lsock -lstash [EXTRALIBS];
  [CHMOD] 0755 [PREFIX]/bin/ncat;
}

ncat_install : ncat_install_self libsock_install_s libstash_install_s;

ncat_uninstall :
{
  rm -f [PREFIX]/bin/ncat;
}

ncat_clean :
{
}

ncat_distclean :
{
  rm -f [objdir]/bin/ncat/doc/latex/manual.tex;
}

/*
 * Dependencies.
 */

[NCAT] : [fromto [srcdir]/%0%.c [objdir]/%0%.o_a [NCAT_SRCS]]
         [LIBSOCK_S] [LIBSTASH_S]
  set mkdir
{
  [CC] [A_CFLAGS] -o [target]
       [fromto [srcdir]/%0%.c [objdir]/%0%.o_a [NCAT_SRCS]]
       -Wl,-rpath,[abs_objdir]/lib/c/libsock/lib
       -Wl,-rpath,[abs_objdir]/lib/c/libstash/lib
       [LDFLAGS] -lsock -lstash [EXTRALIBS];
}
