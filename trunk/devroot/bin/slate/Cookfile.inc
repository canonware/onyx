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

/* LaTeX source files. */
{
  local tex_srcs = ;
  tex_srcs = [fromto % [srcdir]/bin/slate/doc/latex/% [tex_srcs]];

  TEX_SRCS = [TEX_SRCS] [tex_srcs];
}

/* Binaries. */
SLATE = [objdir]/bin/slate/bin/slate;
BINS = [BINS] [SLATE];

/*
 * User cook'ables.
 */

slate : [SLATE];

slate_install_self :
{
  [INSTALL] -d [PREFIX]/bin;

  [SHELL] -e;
data
#![SHELL]

echo \"[srcdir]/bin/slate/src/slate.nx --> [PREFIX]/bin/slate\"

echo \"#![PREFIX]/bin/onyx\" > [PREFIX]/bin/slate
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

[objdir]/bin/slate/bin/% : [srcdir]/bin/slate/src/%.nx [TONYX] [MODSLATE]
  set mkdir
{
  [SHELL] -e;
data
#![SHELL]

echo \"[srcdir]/bin/slate/src/%.nx --> [target]\"

echo \"#![abs_objdir]/[TONYX]\" > [target]
echo \"!#\" >> [target]
[CAT] [srcdir]/bin/slate/src/%.nx >> [target];
chmod u+x [target];
dataend
}
