/******************************************************************************
 *
 * <Copyright = jasone>
 * <License>
 *
 ******************************************************************************
 *
 * Version: <Version>
 *
 * cook file for ncurses module.
 *
 ******************************************************************************/

/* Include generated dependency files. */
#include-cooked [glob [objdir]/mod/modncurses/src/"*.d_*"]

/*
 * File lists.
 */

/* Include files (not installed). */
{
  local incs = modncurses.h;
  incs = [fromto % [srcdir]/mod/modncurses/include/% [incs]];
  incs = [incs] [objdir]/mod/modncurses/include/modncurses_defs.h;

  MODNCURSES_MINCS = [incs];
  MINCS = [MINCS] [incs];
}

/* Source files. */
MODNCURSES_MSRCS = modncurses.c;

MODNCURSES_MSRCS = [fromto % [srcdir]/mod/modncurses/src/% [MODNCURSES_MSRCS]];
MSRCS = [MSRCS] [MODNCURSES_MSRCS];

/*
 * Simple onyx tests.  Output is diff'ed with the expected (.exp file)
 * output.
 */
{
  local otests = ;
  otests = [fromto % [srcdir]/mod/modncurses/test/% [otests]];

  OTESTS = [OTESTS] [otests];
}

/* Perl scripts that drive other tests and use the sequence mode of verify. */
{
  local ptests = ;
  ptests = [fromto % [srcdir]/mod/modncurses/test/% [ptests]];

  PTESTS = [PTESTS] [ptests];
}

/* onyx programs that are driven by tests. */
{
  local obtests = ;
  obtests = [fromto % [srcdir]/mod/modncurses/test/% [obtests]];

  OBTESTS = [OBTESTS] [obtests];
}

/* LaTeX source files. */
{
  local tex_srcs = ;
  tex_srcs = [fromto % [srcdir]/mod/modncurses/doc/latex/% [tex_srcs]];
  tex_srcs = [tex_srcs] [objdir]/mod/modncurses/doc/latex/manual.tex;

  TEX_SRCS = [TEX_SRCS] [tex_srcs];
}

/* xfig source files. */
{
  local fig_srcs = ;
  fig_srcs = [fromto % [srcdir]/mod/modncurses/doc/latex/% [fig_srcs]];

  FIG_SRCS = [FIG_SRCS] [fig_srcs];
}

/* Modules. */
MODNCURSES = [objdir]/mod/modncurses/nxmod/ncurses.nxm;

MODS = [MODS] [MODNCURSES];

/*
 * User cook'ables.
 */

modncurses : [MODNCURSES];

modncurses_install : [MODNCURSES]
{
  [INSTALL] -d [PREFIX]/share/canonware/onyx/nxmod;
  [INSTALL] -m 0444 [MODNCURSES] [PREFIX]/share/canonware/onyx/nxmod;
}

modncurses_uninstall :
{
  rm -f [PREFIX]/share/canonware/onyx/nxmod/modncurses.nxm;
}

modncurses_clean :
{
}

modncurses_distclean :
{
  rm -f [objdir]/mod/modncurses/doc/latex/manual.tex;
}

/*
 * Various flags.
 */

CPPFLAGS = [CPPFLAGS] -I[srcdir]/mod/modncurses/include
                      -I[objdir]/mod/modncurses/include;

/*
 * Dependencies.
 */

[MODNCURSES] : [fromto [srcdir]/%0%.c [objdir]/%0%.o_s [MODNCURSES_MSRCS]]
  set mkdir
{
  [CC] -shared -o [target] [need];
}
