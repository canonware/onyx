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
 * Simple onyx tests.  Output is diff'ed with the expected (.exp file)
 * output.
 */
{
  local otests = ;
  otests = [fromto % [srcdir]/bin/cover/test/% [otests]];

  OTESTS = [OTESTS] [otests];
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

/* onyx programs that are driven by tests. */
{
  local obtests = ;
  obtests = [fromto % [srcdir]/bin/cover/test/% [obtests]];

  OBTESTS = [OBTESTS] [obtests];
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
BINS = [BINS] [COVER];

/*
 * User cook'ables.
 */

cover : [COVER];

cover_install_self : [COVER]
{
  [INSTALL] -d [PREFIX]/bin;
  [CC] [A_CFLAGS] -o [PREFIX]/bin/cover
       [fromto [srcdir]/%0%.c [objdir]/%0%.o_a [COVER_SRCS]]
       [RPATH][PREFIX]/lib
       [LDFLAGS] -lstash [EXTRALIBS];
  [CHMOD] 0755 [PREFIX]/bin/cover;
}

cover_install : cover_install_self libstash_install_s;

cover_uninstall :
{
  rm -f [PREFIX]/bin/cover;
}

cover_clean :
{
}

cover_distclean :
{
}

/*
 * Dependencies.
 */

[COVER] : [fromto [srcdir]/%0%.c [objdir]/%0%.o_a [COVER_SRCS]]
          [LIBSTASH_S]
  set mkdir
{
  [CC] [A_CFLAGS] -o [target]
       [fromto [srcdir]/%0%.c [objdir]/%0%.o_a [COVER_SRCS]]
       [RPATH][abs_objdir]/lib/c/libstash/lib
       [LDFLAGS] -lstash [EXTRALIBS];
}
