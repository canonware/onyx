/******************************************************************************
 *
 * <Copyright = jasone>
 * <License>
 *
 ******************************************************************************
 *
 * Version: <Version>
 *
 * cook file for nxe module.
 *
 ******************************************************************************/

/* Include generated dependency files. */
#include-cooked [glob [objdir]/mod/modnxe/src/"*.d_*"]

/*
 * File lists.
 */

/* Include files (not installed). */
{
  local incs = buf.h buffer.h modnxe.h msgq.h;
  incs = [fromto % [srcdir]/mod/modnxe/include/% [incs]];
  incs = [incs] [objdir]/mod/modnxe/include/modnxe_defs.h;

  MODNXE_MINCS = [incs];
  MINCS = [MINCS] [incs];
}

/* Source files. */
MODNXE_MSRCS = buf.c buffer.c modnxe.c msgq.c;

MODNXE_MSRCS = [fromto % [srcdir]/mod/modnxe/src/% [MODNXE_MSRCS]];
MSRCS = [MSRCS] [MODNXE_MSRCS];

/*
 * Simple onyx tests.  Output is diff'ed with the expected (.exp file)
 * output.
 */
{
  local otests = ;
  otests = [fromto % [srcdir]/mod/modnxe/test/% [otests]];

  OTESTS = [OTESTS] [otests];
}

/* Perl scripts that drive other tests and use the sequence mode of verify. */
{
  local ptests = ;
  ptests = [fromto % [srcdir]/mod/modnxe/test/% [ptests]];

  PTESTS = [PTESTS] [ptests];
}

/* onyx programs that are driven by tests. */
{
  local obtests = ;
  obtests = [fromto % [srcdir]/mod/modnxe/test/% [obtests]];

  OBTESTS = [OBTESTS] [obtests];
}

/* LaTeX source files. */
{
  local tex_srcs = ;
  tex_srcs = [fromto % [srcdir]/mod/modnxe/doc/latex/% [tex_srcs]];
  tex_srcs = [tex_srcs] [objdir]/mod/modnxe/doc/latex/manual.tex;

  TEX_SRCS = [TEX_SRCS] [tex_srcs];
}

/* xfig source files. */
{
  local fig_srcs = ;
  fig_srcs = [fromto % [srcdir]/mod/modnxe/doc/latex/% [fig_srcs]];

  FIG_SRCS = [FIG_SRCS] [fig_srcs];
}

/* Modules. */
MODNXE = [objdir]/mod/modnxe/nxmod/nxe.nxm;

MODS = [MODS] [MODNXE];

/*
 * User cook'ables.
 */

modnxe : [MODNXE];

modnxe_install : [MODNXE]
{
  [INSTALL] -d [PREFIX]/share/onyx/nxmod;
  [INSTALL] -m 0444 [MODNXE] [PREFIX]/share/onyx/nxmod;
}

modnxe_uninstall :
{
  rm -f [PREFIX]/share/onyx/nxmod/nxe.nxm;
}

modnxe_clean :
{
}

modnxe_distclean :
{
}

/*
 * Various flags.
 */

CPPFLAGS = [CPPFLAGS] -I[srcdir]/mod/modnxe/include
                      -I[objdir]/mod/modnxe/include;

/*
 * Dependencies.
 */

[MODNXE] : [fromto [srcdir]/%0%.c [objdir]/%0%.o_s [MODNXE_MSRCS]]
  set mkdir
{
  [CC] -shared -o [target] [need];
}
