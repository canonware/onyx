/******************************************************************************
 *
 * <Copyright = "jasone">
 * <License>
 *
 ******************************************************************************
 *
 * Version: <Version>
 *
 * cook file for cltest.
 *
 ******************************************************************************/

/* Include generated dependency files. */
#include-cooked [glob [objdir]/bin/cltest/src/"*.d_*"]

/*
 * File lists.
 */

/* Include files that _are_ installed. */
{
  local incs = ;
  incs = [fromto % [srcdir]/bin/cltest/include/% [incs]];

  INCS = [INCS] [incs];
}

/* Include files that are _not_ installed. */
{
  local priv_incs = ;
  priv_incs = [fromto % [srcdir]/bin/cltest/include/% [priv_incs]];
  priv_incs = [priv_incs] [objdir]/bin/cltest/include/cltest_defs.h;

  PRIV_INCS = [PRIV_INCS] [priv_incs];
}

/* Source files. */
CLTEST_SRCS = cltest.c;
CLTEST_SRCS = [fromto % [srcdir]/bin/cltest/src/% [CLTEST_SRCS]];
SRCS = [SRCS] [CLTEST_SRCS];

/* Simple tests.  Output is diff'ed with the expected (.exp file) output. */
{
  local ctests = ;
  ctests = [fromto % [srcdir]/bin/cltest/test/% [ctests]];

  CTESTS = [CTESTS] [ctests];
}

/* Perl scripts that drive the BTESTS. */
{
  local ptests = ;
  ptests = [fromto % [srcdir]/bin/cltest/test/% [ptests]];

  PTESTS = [PTESTS] [ptests];
}

/* Backend test programs that are driven by the PTESTS. */
{
  local btests = ;
  btests = [fromto % [srcdir]/bin/cltest/test/% [btests]];

  BTESTS = [BTESTS] [btests];
}

/* Man pages. */
{
  local manpages_1 = ;
  manpages_1 = [fromto % [srcdir]/bin/cltest/man/man1/% [manpages_1]];

  CLTEST_MANPAGES_1 = [manpages_1];
  MANPAGES_1 = [MANPAGES_1] [manpages_1];
}

/* Binaries. */
CLTEST = [objdir]/bin/cltest/src/cltest;
CLTEST_D = [CLTEST].dbg;
CLTEST_A = [CLTEST];
CLTEST_P = [CLTEST].prof;

D_BINS = [D_BINS] [CLTEST_D];
A_BINS = [A_BINS] [CLTEST_A];
P_BINS = [P_BINS] [CLTEST_P];

/*
 * User cook'ables.
 */

cltest : [CLTEST_D] [CLTEST_A] [CLTEST_P];

cltest_install : cltest_install_d cltest_install_a cltest_install_p;

cltest_uninstall :
{
  [SHELL] -e;
data
#![SHELL]

echo rm -f [PREFIX]/bin/cltest.dbg
rm -f [PREFIX]/bin/cltest.dbg

echo rm -f [PREFIX]/bin/cltest
rm -f [PREFIX]/bin/cltest

echo rm -f [PREFIX]/bin/cltest.prof
rm -f [PREFIX]/bin/cltest.prof

for i in [CLTEST_MANPAGES_1]; do
	echo rm -f [PREFIX]/man/man1/`[BASENAME] $i`
	rm -f [PREFIX]/man/man1/`[BASENAME] $i`
done
dataend
}

/*
 * Various flags.
 */

CPPFLAGS = [CPPFLAGS] -I[srcdir]/bin/cltest/include
                      -I[objdir]/bin/cltest/include;
D_CPPFLAGS = [D_CPPFLAGS] -D_CLTEST_DBG;

/*
 * Build rules.
 */

cltest_install_common :
{
  [SHELL] -e;
data
#![SHELL]

echo [INSTALL] -d [PREFIX]/man/man1
[INSTALL] -d [PREFIX]/man/man1

for i in [CLTEST_MANPAGES_1]; do
	echo [INSTALL] -m 0644 $i [PREFIX]/man/man1/
	[INSTALL] -m 0644 $i [PREFIX]/man/man1/
done

echo [INSTALL] -d [PREFIX]/bin
[INSTALL] -d [PREFIX]/bin
dataend
}

cltest_install_d : [CLTEST_D] cltest_install_common
{
	[INSTALL] -m 0755 [CLTEST_D] [PREFIX]/bin;
}

cltest_install_a : [CLTEST_A] cltest_install_common
#if [ENABLE_SHARED]
                 libcltest_install_s libstash_install_s
#endif
{
#if [ENABLE_SHARED]
  [CC] [D_CFLAGS] -o [PREFIX]/bin/cltest
       [fromto [srcdir]/%0%.c [objdir]/%0%.o_a [CLTEST_SRCS]]
       -rpath [PREFIX]/lib
       [LDFLAGS] -lcl -lstash -ltermcap [EXTRALIBS];
  [CHMOD] 0755 [PREFIX]/bin/cltest;
#else
	[INSTALL] -m 0755 [CLTEST_A] [PREFIX]/bin;
#endif
}

cltest_install_p : [CLTEST_P] cltest_install_common
{
	[INSTALL] -m 0755 [CLTEST_P] [PREFIX]/bin;
}

/*
 * Dependencies.
 */

[CLTEST_D] : [fromto [srcdir]/%0%.c [objdir]/%0%.o_d [CLTEST_SRCS]]
           [LIBSTASH_D]
  set mkdir
{
  [CC] [D_CFLAGS] -o [target]
       [fromto [srcdir]/%0%.c [objdir]/%0%.o_d [CLTEST_SRCS]]
       [LDFLAGS] -lcl_d -lstash_d -ltermcap [EXTRALIBS];
}

[CLTEST_A] : [fromto [srcdir]/%0%.c [objdir]/%0%.o_a [CLTEST_SRCS]]
#if [ENABLE_SHARED]
           [LIBCL_S] [LIBSTASH_S]
#else
           [LIBCL_A] [LIBSTASH_A]
#endif
  set mkdir
{
  [CC] [D_CFLAGS] -o [target]
       [fromto [srcdir]/%0%.c [objdir]/%0%.o_a [CLTEST_SRCS]]
#if [ENABLE_SHARED]
       -rpath [objdir]/lib/c/libcl/lib
       -rpath [objdir]/lib/c/libstash/lib
#endif
       [LDFLAGS] -lcl -lstash -ltermcap [EXTRALIBS];
}

[CLTEST_P] : [fromto [srcdir]/%0%.c [objdir]/%0%.o_p [CLTEST_SRCS]]
           [LIBCL_P] [LIBSTASH_P]
  set mkdir
{
  [CC] [D_CFLAGS] -o [target]
       [fromto [srcdir]/%0%.c [objdir]/%0%.o_p [CLTEST_SRCS]]
       [LDFLAGS] -lcl_p -lstash_p -ltermcap [EXTRALIBS];
}
