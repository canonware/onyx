/******************************************************************************
 *
 * <Copyright = "jasone">
 * <License>
 *
 ******************************************************************************
 *
 * Version: <Version>
 *
 * cook file for cover.
 *
 ******************************************************************************/

/* Include generated dependency files. */
#include-cooked [glob [objdir]/bin/cover/src/"*.d_*"]

/*
 * File lists.
 */

/* Include files that _are_ installed. */
{
  local incs = ;
  incs = [fromto % [srcdir]/bin/cover/include/% [incs]];

  INCS = [INCS] [incs];
}

/* Include files that are _not_ installed. */
{
  local priv_incs = cover.h gene.h matrix.h pack.h;
  priv_incs = [fromto % [srcdir]/bin/cover/include/% [priv_incs]];
  priv_incs = [priv_incs] [objdir]/bin/cover/include/cover_defs.h;

  PRIV_INCS = [PRIV_INCS] [priv_incs];
}

/* Source files. */
COVER_SRCS = cover.c matrix.c;
COVER_SRCS = [fromto % [srcdir]/bin/cover/src/% [COVER_SRCS]];
SRCS = [SRCS] [COVER_SRCS];

/* Simple tests.  Output is diff'ed with the expected (.exp file) output. */
{
  local ctests = ;
  ctests = [fromto % [srcdir]/bin/cover/test/% [ctests]];

  CTESTS = [CTESTS] [ctests];
}

/* Perl scripts that drive the BTESTS. */
{
  local ptests = ;
  ptests = [fromto % [srcdir]/bin/cover/test/% [ptests]];

  PTESTS = [PTESTS] [ptests];
}

/* Backend test programs that are driven by the PTESTS. */
{
  local btests = ;
  btests = [fromto % [srcdir]/bin/cover/test/% [btests]];

  BTESTS = [BTESTS] [btests];
}

/* Man pages. */
{
  local manpages_1 = cover.1;
  manpages_1 = [fromto % [srcdir]/bin/cover/man/man1/% [manpages_1]];

  COVER_MANPAGES_1 = [manpages_1];
  MANPAGES_1 = [MANPAGES_1] [manpages_1];
}

/* Binaries. */
COVER = [objdir]/bin/cover/src/cover;
COVER_D = [COVER].dbg;
COVER_A = [COVER];
COVER_P = [COVER].prof;

D_BINS = [D_BINS] [COVER_D];
A_BINS = [A_BINS] [COVER_A];
P_BINS = [P_BINS] [COVER_P];

/*
 * User cook'ables.
 */

cover : [COVER_D] [COVER_A] [COVER_P];

cover_install : cover_install_d cover_install_a cover_install_p;

cover_uninstall :
{
  [SHELL] -e;
data
#![SHELL]

echo rm -f [PREFIX]/bin/cover.dbg
rm -f [PREFIX]/bin/cover.dbg

echo rm -f [PREFIX]/bin/cover
rm -f [PREFIX]/bin/cover

echo rm -f [PREFIX]/bin/cover.prof
rm -f [PREFIX]/bin/cover.prof

for i in [COVER_MANPAGES_1]; do
	echo rm -f [PREFIX]/man/man1/`[BASENAME] $i`
	rm -f [PREFIX]/man/man1/`[BASENAME] $i`
done
dataend
}

cover_clean :
{
}

cover_distclean :
{
}

/*
 * Various flags.
 */

CPPFLAGS = [CPPFLAGS] -I[srcdir]/bin/cover/include
                      -I[objdir]/bin/cover/include;
D_CPPFLAGS = [D_CPPFLAGS] -D_COVER_DBG;

/*
 * Build rules.
 */

cover_install_common :
{
  [SHELL] -e;
data
#![SHELL]

echo [INSTALL] -d [PREFIX]/man/man1
[INSTALL] -d [PREFIX]/man/man1

for i in [COVER_MANPAGES_1]; do
	echo [INSTALL] -m 0644 $i [PREFIX]/man/man1/
	[INSTALL] -m 0644 $i [PREFIX]/man/man1/
done

echo [INSTALL] -d [PREFIX]/bin
[INSTALL] -d [PREFIX]/bin
dataend
}

cover_install_d : [COVER_D] cover_install_common
{
	[INSTALL] -m 0755 [COVER_D] [PREFIX]/bin;
}

cover_install_a : [COVER_A] cover_install_common
#if [ENABLE_SHARED]
                  libstash_install_s
#endif
{
#if [ENABLE_SHARED]
  [CC] [D_CFLAGS] -o [PREFIX]/bin/cover
       [fromto [srcdir]/%0%.c [objdir]/%0%.o_a [COVER_SRCS]]
       -rpath [PREFIX]/lib
       [LDFLAGS] -lstash [EXTRALIBS];
  [CHMOD] 0755 [PREFIX]/bin/cover;
#else
	[INSTALL] -m 0755 [COVER_A] [PREFIX]/bin;
#endif
}

cover_install_p : [COVER_P] cover_install_common
{
	[INSTALL] -m 0755 [COVER_P] [PREFIX]/bin;
}

/*
 * Dependencies.
 */

[COVER_D] : [fromto [srcdir]/%0%.c [objdir]/%0%.o_d [COVER_SRCS]]
           [LIBSTASH_D]
  set mkdir
{
  [CC] [D_CFLAGS] -o [target]
       [fromto [srcdir]/%0%.c [objdir]/%0%.o_d [COVER_SRCS]]
       [LDFLAGS] -lstash_d [EXTRALIBS];
}

[COVER_A] : [fromto [srcdir]/%0%.c [objdir]/%0%.o_a [COVER_SRCS]]
#if [ENABLE_SHARED]
           [LIBSTASH_S]
#else
           [LIBSTASH_A]
#endif
  set mkdir
{
  [CC] [A_CFLAGS] -o [target]
       [fromto [srcdir]/%0%.c [objdir]/%0%.o_a [COVER_SRCS]]
#if [ENABLE_SHARED]
       -rpath [objdir]/lib/c/libstash/lib
#endif
       [LDFLAGS] -lstash [EXTRALIBS];
}

[COVER_P] : [fromto [srcdir]/%0%.c [objdir]/%0%.o_p [COVER_SRCS]]
           [LIBSTASH_P]
  set mkdir
{
  [CC] [P_CFLAGS] -o [target]
       [fromto [srcdir]/%0%.c [objdir]/%0%.o_p [COVER_SRCS]]
       [LDFLAGS] -lstash_p [EXTRALIBS];
}
