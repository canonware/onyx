/******************************************************************************
 *
 * <Copyright = "jasone">
 * <License>
 *
 ******************************************************************************
 *
 * Version: <Version>
 *
 * cook file for ficl.
 *
 ******************************************************************************/

/* Include generated dependency files. */
#include-cooked [glob [objdir]/bin/ficl/src/"*.d_*"]

/*
 * File lists.
 */

/* Include files that _are_ installed. */
{
  local incs = ;
  incs = [fromto % [srcdir]/bin/ficl/include/% [incs]];

  INCS = [INCS] [incs];
}

/* Include files that are _not_ installed. */
{
  local priv_incs = ;
  priv_incs = [fromto % [srcdir]/bin/ficl/include/% [priv_incs]];
  priv_incs = [priv_incs] [objdir]/bin/ficl/include/ficl_defs.h;

  PRIV_INCS = [PRIV_INCS] [priv_incs];
}

/* Source files. */
FICL_SRCS = ficl.c;
FICL_SRCS = [fromto % [srcdir]/bin/ficl/src/% [FICL_SRCS]];
SRCS = [SRCS] [FICL_SRCS];

/* Simple tests.  Output is diff'ed with the expected (.exp file) output. */
{
  local ctests = ;
  ctests = [fromto % [srcdir]/bin/ficl/test/% [ctests]];

  CTESTS = [CTESTS] [ctests];
}

/* Perl scripts that drive the BTESTS. */
{
  local ptests = ;
  ptests = [fromto % [srcdir]/bin/ficl/test/% [ptests]];

  PTESTS = [PTESTS] [ptests];
}

/* Backend test programs that are driven by the PTESTS. */
{
  local btests = ;
  btests = [fromto % [srcdir]/bin/ficl/test/% [btests]];

  BTESTS = [BTESTS] [btests];
}

/* Man pages. */
{
  local manpages_1 = ;
  manpages_1 = [fromto % [srcdir]/bin/ficl/man/man1/% [manpages_1]];

  FICL_MANPAGES_1 = [manpages_1];
  MANPAGES_1 = [MANPAGES_1] [manpages_1];
}

/* Binaries. */
FICL = [objdir]/bin/ficl/src/ficl;
FICL_D = [FICL].dbg;
FICL_A = [FICL];
FICL_P = [FICL].prof;

D_BINS = [D_BINS] [FICL_D];
A_BINS = [A_BINS] [FICL_A];
P_BINS = [P_BINS] [FICL_P];

/*
 * User cook'ables.
 */

ficl : [FICL_D] [FICL_A] [FICL_P];

ficl_install : ficl_install_d ficl_install_a ficl_install_p;

ficl_uninstall :
{
  [SHELL] -e;
data
#![SHELL]

echo rm -f [PREFIX]/bin/ficl.dbg
rm -f [PREFIX]/bin/ficl.dbg

echo rm -f [PREFIX]/bin/ficl
rm -f [PREFIX]/bin/ficl

echo rm -f [PREFIX]/bin/ficl.prof
rm -f [PREFIX]/bin/ficl.prof

for i in [FICL_MANPAGES_1]; do
	echo rm -f [PREFIX]/man/man1/`[BASENAME] $i`
	rm -f [PREFIX]/man/man1/`[BASENAME] $i`
done
dataend
}

ficl_clean :
{
}

ficl_distclean :
{
}

/*
 * Various flags.
 */

CPPFLAGS = [CPPFLAGS] -I[srcdir]/bin/ficl/include
                      -I[objdir]/bin/ficl/include;
D_CPPFLAGS = [D_CPPFLAGS] -D_FICL_DBG;

/*
 * Build rules.
 */

ficl_install_common :
{
  [SHELL] -e;
data
#![SHELL]

echo [INSTALL] -d [PREFIX]/man/man1
[INSTALL] -d [PREFIX]/man/man1

for i in [FICL_MANPAGES_1]; do
	echo [INSTALL] -m 0644 $i [PREFIX]/man/man1/
	[INSTALL] -m 0644 $i [PREFIX]/man/man1/
done

echo [INSTALL] -d [PREFIX]/bin
[INSTALL] -d [PREFIX]/bin
dataend
}

ficl_install_d : [FICL_D] ficl_install_common
{
	[INSTALL] -m 0755 [FICL_D] [PREFIX]/bin;
}

ficl_install_a : [FICL_A] ficl_install_common
#if [ENABLE_SHARED]
                 libficl_install_s libedit_install_s libstash_install_s
#endif
{
#if [ENABLE_SHARED]
  [CC] [D_CFLAGS] -o [PREFIX]/bin/ficl
       [fromto [srcdir]/%0%.c [objdir]/%0%.o_a [FICL_SRCS]]
       -rpath [PREFIX]/lib
       [LDFLAGS] -lficl -ledit -lstash -ltermcap [EXTRALIBS];
  [CHMOD] 0755 [PREFIX]/bin/ficl;
#else
	[INSTALL] -m 0755 [FICL_A] [PREFIX]/bin;
#endif
}

ficl_install_p : [FICL_P] ficl_install_common
{
	[INSTALL] -m 0755 [FICL_P] [PREFIX]/bin;
}

/*
 * Dependencies.
 */

[FICL_D] : [fromto [srcdir]/%0%.c [objdir]/%0%.o_d [FICL_SRCS]]
           [LIBFICL_D] [LIBEDIT_D] [LIBSTASH_D]
  set mkdir
{
  [CC] [D_CFLAGS] -o [target]
       [fromto [srcdir]/%0%.c [objdir]/%0%.o_d [FICL_SRCS]]
       [LDFLAGS] -lficl_d -ledit_d -lstash_d -ltermcap [EXTRALIBS];
}

[FICL_A] : [fromto [srcdir]/%0%.c [objdir]/%0%.o_a [FICL_SRCS]]
#if [ENABLE_SHARED]
           [LIBFICL_S] [LIBEDIT_S] [LIBSTASH_S]
#else
           [LIBFICL_A] [LIBEDIT_A] [LIBSTASH_A]
#endif
  set mkdir
{
  [CC] [A_CFLAGS] -o [target]
       [fromto [srcdir]/%0%.c [objdir]/%0%.o_a [FICL_SRCS]]
#if [ENABLE_SHARED]
       -rpath [objdir]/lib/c/libficl/lib
       -rpath [objdir]/lib/c/libedit/lib
       -rpath [objdir]/lib/c/libstash/lib
#endif
       [LDFLAGS] -lficl -ledit -lstash -ltermcap [EXTRALIBS];
}

[FICL_P] : [fromto [srcdir]/%0%.c [objdir]/%0%.o_p [FICL_SRCS]]
           [LIBFICL_P] [LIBEDIT_P] [LIBSTASH_P]
  set mkdir
{
  [CC] [P_CFLAGS] -o [target]
       [fromto [srcdir]/%0%.c [objdir]/%0%.o_p [FICL_SRCS]]
       [LDFLAGS] -lficl_p -ledit_p -lstash_p -ltermcap [EXTRALIBS];
}
