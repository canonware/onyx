/******************************************************************************
 *
 * <Copyright = jasone>
 * <License>
 *
 ******************************************************************************
 *
 * Version: <Version>
 *
 * cook file for slate module.
 *
 ******************************************************************************/

/* Include generated dependency files. */
#include-cooked [glob [objdir]/mod/modslate/src/"*.d_*"]

/*
 * File lists.
 */

/* Include files (not installed). */
{
  local incs = buf.h buffer.h modslate.h msgq.h;
  incs = [fromto % [srcdir]/mod/modslate/include/% [incs]];
  incs = [incs] [objdir]/mod/modslate/include/modslate_defs.h;

  MODSLATE_MINCS = [incs];
  MINCS = [MINCS] [incs];
}

/* Source files. */
MODSLATE_MSRCS = buf.c buffer.c modslate.c msgq.c;

MODSLATE_MSRCS = [fromto % [srcdir]/mod/modslate/src/% [MODSLATE_MSRCS]];
MSRCS = [MSRCS] [MODSLATE_MSRCS];

/*
 * Simple onyx tests.  Output is diff'ed with the expected (.exp file)
 * output.
 */
{
  local otests = ;
  otests = [fromto % [srcdir]/mod/modslate/test/% [otests]];

  OTESTS = [OTESTS] [otests];
}

/* Perl scripts that drive other tests and use the sequence mode of verify. */
{
  local ptests = ;
  ptests = [fromto % [srcdir]/mod/modslate/test/% [ptests]];

  PTESTS = [PTESTS] [ptests];
}

/* onyx programs that are driven by tests. */
{
  local obtests = ;
  obtests = [fromto % [srcdir]/mod/modslate/test/% [obtests]];

  OBTESTS = [OBTESTS] [obtests];
}

/* LaTeX source files. */
{
  local tex_srcs = ;
  tex_srcs = [fromto % [srcdir]/mod/modslate/doc/latex/% [tex_srcs]];

  TEX_SRCS = [TEX_SRCS] [tex_srcs];
}

/* xfig source files. */
{
  local fig_srcs = ;
  fig_srcs = [fromto % [srcdir]/mod/modslate/doc/latex/% [fig_srcs]];

  FIG_SRCS = [FIG_SRCS] [fig_srcs];
}

/* Modules. */
MODSLATE = [objdir]/mod/modslate/nxmod/slate.nxm;

MODS = [MODS] [MODSLATE];

/*
 * User cook'ables.
 */

modslate : [MODSLATE];

modslate_install : [MODSLATE]
{
  [INSTALL] -d [PREFIX]/share/onyx/nxmod;
  [INSTALL] -m 0444 [MODSLATE] [PREFIX]/share/onyx/nxmod;
}

modslate_uninstall :
{
  rm -f [PREFIX]/share/onyx/nxmod/slate.nxm;
}

modslate_clean :
{
}

modslate_distclean :
{
}

/*
 * Various flags.
 */

CPPFLAGS = [CPPFLAGS] -I[srcdir]/mod/modslate/include
                      -I[objdir]/mod/modslate/include;

/*
 * Dependencies.
 */

[MODSLATE] : [fromto [srcdir]/%0%.c [objdir]/%0%.o_s [MODSLATE_MSRCS]]
  set mkdir
{
  [CC] -shared -o [target] [need];
}
