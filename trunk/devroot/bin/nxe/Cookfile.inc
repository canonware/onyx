/******************************************************************************
 *
 * <Copyright = jasone>
 * <License>
 *
 ******************************************************************************
 *
 * Version: <Version>
 *
 * cook file for nxe.
 *
 ******************************************************************************/

/* Include generated dependency files. */
#include-cooked [glob [objdir]/bin/nxe/src/"*.d_*"]

/*
 * File lists.
 */

/* Source files. */
NXE_SRCS = nxe.nx;
NXE_SRCS = [fromto % [srcdir]/bin/nxe/src/% [NXE_SRCS]];

/*
 * Onyx tests.
 */
{
  local otests = ;
  otests = [fromto % [srcdir]/bin/nxe/test/% [otests]];

  OTESTS = [OTESTS] [otests];
}

/* Perl scripts that drive other tests and use the sequence mode of verify. */
{
  local ptests = ;
  ptests = [fromto % [srcdir]/bin/nxe/test/% [ptests]];

  PTESTS = [PTESTS] [ptests];
}

/* Onyx programs that are driven by tests. */
{
  local obtests = ;
  obtests = [fromto % [srcdir]/bin/nxe/test/% [obtests]];

  OBTESTS = [OBTESTS] [obtests];
}

/* LaTeX source files. */
{
  local tex_srcs = ;
  tex_srcs = [fromto % [srcdir]/bin/nxe/doc/latex/% [tex_srcs]];
  tex_srcs = [tex_srcs] [objdir]/bin/nxe/doc/latex/manual.tex;

  TEX_SRCS = [TEX_SRCS] [tex_srcs];
}

/* xfig source files. */
{
  local fig_srcs = ;
  fig_srcs = [fromto % [srcdir]/bin/nxe/doc/latex/% [fig_srcs]];

  FIG_SRCS = [FIG_SRCS] [fig_srcs];
}

/* Binaries. */
NXE = [objdir]/bin/nxe/bin/nxe;
BINS = [BINS] [NXE];

/*
 * User cook'ables.
 */

nxe : [NXE];

nxe_install_self : [ONYX]
{
  [INSTALL] -d [PREFIX]/bin;

  [SHELL] -e;
data
#![SHELL]

echo \"[srcdir]/bin/nxe/src/nxe.nx --> [PREFIX]/bin/nxe\"

echo \"#![abs_objdir]/[ONYX]\" > [PREFIX]/bin/nxe
echo \"!#\" >> [PREFIX]/bin/nxe
[CAT] [srcdir]/bin/nxe/src/nxe.nx >> [PREFIX]/bin/nxe;
chmod u+x [PREFIX]/bin/nxe;
dataend

  [CHMOD] 0755 [PREFIX]/bin/nxe;

/*  [INSTALL] -d [PREFIX]/man/man1;
  [INSTALL] -m 0444 [objdir]/bin/nxe/man/man1/nxe.1 [PREFIX]/man/man1; */
}

nxe_install : nxe_install_self
              onyx_install modnxe_install;

nxe_uninstall :
{
  rm -f [PREFIX]/bin/nxe;
/*  rm -f [PREFIX]/man/man1/nxe.1; */
}

nxe_clean :
{
}

nxe_distclean :
{
  rm -f [objdir]/bin/nxe/doc/latex/manual.tex;
/*  rm -f [objdir]/bin/nxe/man/man1/nxe.1; */
}

/*
 * Dependencies.
 */

[objdir]/bin/nxe/bin/% : [srcdir]/bin/nxe/src/%.nx [ONYX] [MODNXE]
  set mkdir
{
  [SHELL] -e;
data
#![SHELL]

echo \"[srcdir]/bin/nxe/src/%.nx --> [target]\"

echo \"#![abs_objdir]/[ONYX]\" > [target]
echo \"!#\" >> [target]
[CAT] [srcdir]/bin/nxe/src/%.nx >> [target];
chmod u+x [target];
dataend
}
