/******************************************************************************
 *
 * <Copyright = jasone>
 * <License>
 *
 ******************************************************************************
 *
 * Version: <Version>
 *
 * cook file for slate.
 *
 ******************************************************************************/

/* Include generated dependency files. */
#include-cooked [glob [objdir]/bin/slate/src/"*.d_*"]

/*
 * File lists.
 */

/* Source files. */
SLATE_SRCS = slate.nx;
SLATE_SRCS = [fromto % [srcdir]/bin/slate/src/% [SLATE_SRCS]];

/*
 * Onyx tests.
 */
{
  local otests = ;
  otests = [fromto % [srcdir]/bin/slate/test/% [otests]];

  OTESTS = [OTESTS] [otests];
}

/* Perl scripts that drive other tests and use the sequence mode of verify. */
{
  local ptests = ;
  ptests = [fromto % [srcdir]/bin/slate/test/% [ptests]];

  PTESTS = [PTESTS] [ptests];
}

/* Onyx programs that are driven by tests. */
{
  local obtests = ;
  obtests = [fromto % [srcdir]/bin/slate/test/% [obtests]];

  OBTESTS = [OBTESTS] [obtests];
}

/* LaTeX source files. */
{
  local tex_srcs = ;
  tex_srcs = [fromto % [srcdir]/bin/slate/doc/latex/% [tex_srcs]];

  TEX_SRCS = [TEX_SRCS] [tex_srcs];
}

/* xfig source files. */
{
  local fig_srcs = ;
  fig_srcs = [fromto % [srcdir]/bin/slate/doc/latex/% [fig_srcs]];

  FIG_SRCS = [FIG_SRCS] [fig_srcs];
}

/* Binaries. */
SLATE = [objdir]/bin/slate/bin/slate;
BINS = [BINS] [SLATE];

/*
 * User cook'ables.
 */

slate : [SLATE];

slate_install_self : [ONYX]
{
  [INSTALL] -d [PREFIX]/bin;

  [SHELL] -e;
data
#![SHELL]

echo \"[srcdir]/bin/slate/src/slate.nx --> [PREFIX]/bin/slate\"

echo \"#![abs_objdir]/[ONYX]\" > [PREFIX]/bin/slate
echo \"!#\" >> [PREFIX]/bin/slate
[CAT] [srcdir]/bin/slate/src/slate.nx >> [PREFIX]/bin/slate;
chmod u+x [PREFIX]/bin/slate;
dataend

  [CHMOD] 0755 [PREFIX]/bin/slate;

/*  [INSTALL] -d [PREFIX]/man/man1;
  [INSTALL] -m 0444 [objdir]/bin/slate/man/man1/slate.1 [PREFIX]/man/man1; */
}

slate_install : slate_install_self
                onyx_install modslate_install;

slate_uninstall :
{
  rm -f [PREFIX]/bin/slate;
/*  rm -f [PREFIX]/man/man1/slate.1; */
}

slate_clean :
{
}

slate_distclean :
{
  rm -f [objdir]/bin/slate/doc/latex/manual.tex;
/*  rm -f [objdir]/bin/slate/man/man1/slate.1; */
}

/*
 * Dependencies.
 */

[objdir]/bin/slate/bin/% : [srcdir]/bin/slate/src/%.nx [ONYX] [MODSLATE]
  set mkdir
{
  [SHELL] -e;
data
#![SHELL]

echo \"[srcdir]/bin/slate/src/%.nx --> [target]\"

echo \"#![abs_objdir]/[ONYX]\" > [target]
echo \"!#\" >> [target]
[CAT] [srcdir]/bin/slate/src/%.nx >> [target];
chmod u+x [target];
dataend
}
