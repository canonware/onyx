/******************************************************************************
 *
 * <Copyright = jasone>
 * <License>
 *
 ******************************************************************************
 *
 * Version: <Version>
 *
 * cook file for modcanonyx module.
 *
 ******************************************************************************/

/* Include generated dependency files. */
#include-cooked [glob [objdir]/mod/modcanonyx/src/"*.d_*"]

/*
 * File lists.
 */

/* Include files (not installed). */
{
  local incs = modcanonyx.h buf.h buffer.h;
  incs = [fromto % [srcdir]/mod/modcanonyx/include/% [incs]];
  incs = [incs] [objdir]/mod/modcanonyx/include/modcanonyx_defs.h;

  MODCANONYX_MINCS = [incs];
  MINCS = [MINCS] [incs];
}

/* Source files. */
MODCANONYX_MSRCS = canonyx.c buf.c buffer.c;

MSRCS = [MSRCS] [fromto % [srcdir]/mod/modcanonyx/src/% [MODCANONYX_MSRCS]];

/* Module-related Onyx sources that are installed. */
MODCANONYX_OSRCS = canonyx.nx;

/*
 * Simple onyx tests.  Output is diff'ed with the expected (.exp file)
 * output.
 */
{
  local otests = ;
  otests = [fromto % [srcdir]/mod/modcanonyx/test/% [otests]];

  OTESTS = [OTESTS] [otests];
}

/* Perl scripts that drive other tests and use the sequence mode of verify. */
{
  local ptests = ;
  ptests = [fromto % [srcdir]/mod/modcanonyx/test/% [ptests]];

  PTESTS = [PTESTS] [ptests];
}

/* onyx programs that are driven by tests. */
{
  local obtests = ;
  obtests = [fromto % [srcdir]/mod/modcanonyx/test/% [obtests]];

  OBTESTS = [OBTESTS] [obtests];
}

/* LaTeX source files. */
{
  local tex_srcs = ;
  tex_srcs = [fromto % [srcdir]/mod/modcanonyx/doc/latex/% [tex_srcs]];
  tex_srcs = [tex_srcs] [objdir]/mod/modcanonyx/doc/latex/manual.tex;

  TEX_SRCS = [TEX_SRCS] [tex_srcs];
}

/* xfig source files. */
{
  local fig_srcs = ;
  fig_srcs = [fromto % [srcdir]/mod/modcanonyx/doc/latex/% [fig_srcs]];

  FIG_SRCS = [FIG_SRCS] [fig_srcs];
}

/* Modules. */
MODCANONYX = canonyx.nxm;

MODS = [MODS] [objdir]/mod/modcanonyx/nxmod/[MODCANONYX];

/*
 * User cook'ables.
 */

modcanonyx : [objdir]/mod/modcanonyx/nxmod/[MODCANONYX];

modcanonyx_install : [objdir]/mod/modcanonyx/nxmod/[MODCANONYX]
{
  [SHELL] -e;
data
#![SHELL]

echo [INSTALL] -d [PREFIX]/share/onyx/nxmod
[INSTALL] -d [PREFIX]/share/onyx/nxmod

echo [INSTALL] -m 0444 [objdir]/mod/modcanonyx/nxmod/[MODCANONYX] \\
     [PREFIX]/share/onyx/nxmod
[INSTALL] -m 0444 [objdir]/mod/modcanonyx/nxmod/[MODCANONYX] \\
     [PREFIX]/share/onyx/nxmod

echo [INSTALL] -d [PREFIX]/share/onyx/nxcode
[INSTALL] -d [PREFIX]/share/onyx/nxcode

for i in [MODCANONYX_OSRCS]; do
	echo [INSTALL] -m 0644 [srcdir]/mod/modcanonyx/src/$i \\
	    [PREFIX]/share/onyx/nxcode
	[INSTALL] -m 0644 [srcdir]/mod/modcanonyx/src/$i \\
	    [PREFIX]/share/onyx/nxcode
done
dataend
}

modcanonyx_uninstall :
{
  rm -f [PREFIX]/share/onyx/nxmod/[MODCANONYX];
  [SHELL] -e;
data
#![SHELL]

echo rm -f [PREFIX]/share/onyx/nxmod/[MODCANONYX];
rm -f [PREFIX]/share/onyx/nxmod/[MODCANONYX];

for i in [MODCANONYX_OSRCS]; do
	echo rm -f [PREFIX]/share/onyx/nxcode/$i
	rm -f [PREFIX]/share/onyx/nxcode/$i
done
dataend
}

modcanonyx_clean :
{
}

modcanonyx_distclean :
{
  rm -f [objdir]/mod/modcanonyx/include/modcanonyx_defs.h;
  rm -f [objdir]/mod/modcanonyx/doc/latex/manual.tex;
}

/*
 * Various flags.
 */

CPPFLAGS = [CPPFLAGS] -I[srcdir]/mod/modcanonyx/include
                      -I[objdir]/mod/modcanonyx/include;

/*
 * Dependencies.
 */

[objdir]/mod/modcanonyx/nxmod/[MODCANONYX] :
  [fromto %.c [objdir]/mod/modcanonyx/src/%.o_s [MODCANONYX_MSRCS]]
  set mkdir
{
  [CC] -shared -o [target] [need];
}
