/******************************************************************************
 *
 * <Copyright = jasone>
 * <License>
 *
 ******************************************************************************
 *
 * Version: <Version>
 *
 * cook file for gtk onyx module.
 *
 ******************************************************************************/

/* Include generated dependency files. */
#include-cooked [glob [objdir]/mod/modnxgtk/src/"*.d_*"]

/*
 * File lists.
 */

/* Include files (not installed). */
{
  local incs = modnxgtk.h;
  incs = [fromto % [srcdir]/mod/modnxgtk/include/% [incs]];
  incs = [incs] [objdir]/mod/modnxgtk/include/modnxgtk.h;

  MODNXGTK_MINCS = [incs];
  MINCS = [MINCS] [incs];
}

/* Source files. */
MODNXGTK_MSRCS = gtkbutton.c gtkcontainer.c gtkdict.c gtklabel.c \
		   gtkmain.c gtksignal.c gtkwidget.c gtkwindow.c modnxgtk.c;

MODNXGTK_MSRCS = [fromto % [srcdir]/mod/modnxgtk/src/% [MODNXGTK_MSRCS]];
MSRCS = [MSRCS] [MODNXGTK_MSRCS];

/*
 * Simple onyx tests.  Output is diff'ed with the expected (.exp file)
 * output.
 */
{
  local otests = ;
  otests = [fromto % [srcdir]/mod/modnxgtk/test/% [otests]];

  OTESTS = [OTESTS] [otests];
}

/* Perl scripts that drive other tests and use the sequence mode of verify. */
{
  local ptests = ;
  ptests = [fromto % [srcdir]/mod/modnxgtk/test/% [ptests]];

  PTESTS = [PTESTS] [ptests];
}

/* onyx programs that are driven by tests. */
{
  local obtests = ;
  obtests = [fromto % [srcdir]/mod/modnxgtk/test/% [obtests]];

  OBTESTS = [OBTESTS] [obtests];
}

/* LaTeX source files. */
{
  local tex_srcs = ;
  tex_srcs = [fromto % [srcdir]/mod/modnxgtk/doc/latex/% [tex_srcs]];
  tex_srcs = [tex_srcs] [objdir]/mod/modnxgtk/doc/latex/manual.tex;

  TEX_SRCS = [TEX_SRCS] [tex_srcs];
}

/* xfig source files. */
{
  local fig_srcs = ;
  fig_srcs = [fromto % [srcdir]/mod/modnxgtk/doc/latex/% [fig_srcs]];

  FIG_SRCS = [FIG_SRCS] [fig_srcs];
}

/* Modules. */
MODNXGTK = [objdir]/mod/modnxgtk/nxmod/nxgtk.nxm;

MODS = [MODS] [MODNXGTK];

/*
 * User cook'ables.
 */

modnxgtk : [MODNXGTK];

modnxgtk : [MODNXGTK]
{
  [INSTALL] -d [PREFIX]/share/canonware/onyx/nxmod;
  [INSTALL] -m 0444 [MODNXGTK] [PREFIX]/share/canonware/onyx/nxmod;
}

modnxgtk :
{
  rm -f [PREFIX]/share/canonware/onyx/nxmod/modnxgtk.nxm;
}

modnxgtk :
{
}

modnxgtk :
{
  rm -f [objdir]/mod/modnxgtk/doc/latex/manual.tex;
}

/*
 * Various flags.
 */

CPPFLAGS = [CPPFLAGS] -I[srcdir]/mod/modnxgtk/include
                      -I[objdir]/mod/modnxgtk/include
		      `gtk-config --cflags`;

/*
 * Dependencies.
 */

[MODNXGTK] : [fromto [srcdir]/%0%.c [objdir]/%0%.o_s [MODNXGTK_MSRCS]]
  set mkdir
{
  [CC] -shared -o [target] [need] `gtk-config --libs`;
}
