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
#include-cooked [glob [objdir]/mod/modgtk/src/"*.d_*"]

/*
 * File lists.
 */

/* Include files (not installed). */
{
  local incs = modgtk.h;
  incs = [fromto % [srcdir]/mod/modgtk/include/% [incs]];
  incs = [incs] [objdir]/mod/modgtk/include/modgtk.h;

  MODGTK_MINCS = [incs];
  MINCS = [MINCS] [incs];
}

/* Source files. */
MODGTK_MSRCS = gtkbutton.c gtkcontainer.c gtkdict.c gtklabel.c \
		   gtkmain.c gtksignal.c gtkwidget.c gtkwindow.c modgtk.c;

MODGTK_MSRCS = [fromto % [srcdir]/mod/modgtk/src/% [MODGTK_MSRCS]];
MSRCS = [MSRCS] [MODGTK_MSRCS];

/*
 * Simple onyx tests.  Output is diff'ed with the expected (.exp file)
 * output.
 */
{
  local otests = ;
  otests = [fromto % [srcdir]/mod/modgtk/test/% [otests]];

  OTESTS = [OTESTS] [otests];
}

/* Perl scripts that drive other tests and use the sequence mode of verify. */
{
  local ptests = ;
  ptests = [fromto % [srcdir]/mod/modgtk/test/% [ptests]];

  PTESTS = [PTESTS] [ptests];
}

/* onyx programs that are driven by tests. */
{
  local obtests = ;
  obtests = [fromto % [srcdir]/mod/modgtk/test/% [obtests]];

  OBTESTS = [OBTESTS] [obtests];
}

/* LaTeX source files. */
/*
{
  local tex_srcs = ;
  tex_srcs = [fromto % [srcdir]/mod/modgtk/doc/latex/% [tex_srcs]];
  tex_srcs = [tex_srcs] [objdir]/mod/modgtk/doc/latex/manual.tex;

  TEX_SRCS = [TEX_SRCS] [tex_srcs];
}
*/

/* xfig source files. */
{
  local fig_srcs = ;
  fig_srcs = [fromto % [srcdir]/mod/modgtk/doc/latex/% [fig_srcs]];

  FIG_SRCS = [FIG_SRCS] [fig_srcs];
}

/* Modules. */
MODGTK = [objdir]/mod/modgtk/nxmod/gtk.nxm;

MODS = [MODS] [MODGTK];

/*
 * User cook'ables.
 */

modgtk : [MODGTK];

modgtk_install : [MODGTK]
{
  [INSTALL] -d [PREFIX]/share/canonware/onyx/nxmod;
  [INSTALL] -m 0444 [MODGTK] [PREFIX]/share/canonware/onyx/nxmod;
}

modgtk_uninstall :
{
  rm -f [PREFIX]/share/canonware/onyx/nxmod/modgtk.nxm;
}

modgtk_clean :
{
}

modgtk_distclean :
{
  rm -f [objdir]/mod/modgtk/doc/latex/manual.tex;
}

/*
 * Various flags.
 */

CPPFLAGS = [CPPFLAGS] -I[srcdir]/mod/modgtk/include
                      -I[objdir]/mod/modgtk/include
		      `[GTKCONFIG] --cflags`;

/*
 * Dependencies.
 */

[MODGTK] : [fromto [srcdir]/%0%.c [objdir]/%0%.o_s [MODGTK_MSRCS]]
  set mkdir
{
  [CC] -shared -o [target] [need] `[GTKCONFIG] --libs`;
}
