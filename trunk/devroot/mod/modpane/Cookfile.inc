/******************************************************************************
 *
 * <Copyright = jasone>
 * <License>
 *
 ******************************************************************************
 *
 * Version: <Version>
 *
 * cook file for pane module.
 *
 ******************************************************************************/

/* Include generated dependency files. */
#include-cooked [glob [objdir]/mod/modpane/src/"*.d_*"]

/*
 * File lists.
 */

/* Include files (not installed). */
{
  local incs = modpane.h;
  incs = [fromto % [srcdir]/mod/modpane/include/% [incs]];
  incs = [incs] [objdir]/mod/modpane/include/modpane_defs.h;

  MODPANE_MINCS = [incs];
  MINCS = [MINCS] [incs];
}

/* Source files. */
MODPANE_MSRCS = modpane.c;

MODPANE_MSRCS = [fromto % [srcdir]/mod/modpane/src/% [MODPANE_MSRCS]];
MSRCS = [MSRCS] [MODPANE_MSRCS];

/* Generated source code (created by configure). */
MODPANE_NXCODE_GEN = modpane_defs.nx.in;
MODPANE_NXCODE_GEN = [fromto %.nx.in [objdir]/mod/modpane/nxcode/modpane/%.nx
                             [MODPANE_NXCODE_GEN]];

MODPANE_NXCODE = modpane.nx terminfo.nx;
MODPANE_NXCODE = [fromto % [srcdir]/mod/modpane/nxcode/modpane/%
                         [MODPANE_NXCODE]];

/* Modules. */
MODPANE = [objdir]/mod/modpane/nxmod/modpane.nxm;

MODS = [MODS] [MODPANE];

/*
 * User cook'ables.
 */

modpane : [MODPANE];

modpane_install : [MODPANE]
{
  [SHELL] -e;
data
#![SHELL]

echo [INSTALL] -d [PREFIX]/share/onyx/nxmod
[INSTALL] -d [PREFIX]/share/onyx/nxmod

echo [INSTALL] -m 0444 [MODPANE] [PREFIX]/share/onyx/nxmod
[INSTALL] -m 0444 [MODPANE] [PREFIX]/share/onyx/nxmod

echo [INSTALL] -d [PREFIX]/share/onyx/nxcode/modpane
[INSTALL] -d [PREFIX]/share/onyx/nxcode/modpane

for i in [MODPANE_NXCODE] [MODPANE_NXCODE_GEN]; do
	echo [INSTALL] -m 0644 $i [PREFIX]/share/onyx/nxcode/modpane/
	[INSTALL] -m 0644 $i [PREFIX]/share/onyx/nxcode/modpane/
done
dataend
}

modpane_uninstall :
{
  rm -f [PREFIX]/share/onyx/nxmod/modpane.nxm;
  rm -rf [PREFIX]/share/onyx/nxcode/modpane/;
}

modpane_clean :
{
}

modpane_distclean :
{
  rm -f [MODPANE_NXCODE_GEN];
}

/*
 * Various flags.
 */

CPPFLAGS = [CPPFLAGS] -I[srcdir]/mod/modpane/include
                      -I[objdir]/mod/modpane/include;

/*
 * Dependencies.
 */

[MODPANE] : [fromto [srcdir]/%0%.c [objdir]/%0%.o_s [MODPANE_MSRCS]]
  set mkdir
{
  [CC]
#if [matches elf [abi]]
       -shared
#elif [matches macho [abi]]
       -undefined suppress -flat_namespace -bundle -all_load
#endif
       -o [target] [need];
}
