/******************************************************************************
 *
 * <Copyright = "jasone">
 * <License>
 *
 ******************************************************************************
 *
 * Version: <Version>
 *
 * cook file for edittest.
 *
 ******************************************************************************/

/* Include generated dependency files. */
#include-cooked [glob [objdir]/bin/edittest/src/"*.d_*"]

/*
 * File lists.
 */

/* Include files that _are_ installed. */
{
  local incs = ;
  incs = [fromto % [srcdir]/bin/edittest/include/% [incs]];

  INCS = [INCS] [incs];
}

/* Include files that are _not_ installed. */
{
  local priv_incs = ;
  priv_incs = [fromto % [srcdir]/bin/edittest/include/% [priv_incs]];
  priv_incs = [priv_incs] [objdir]/bin/edittest/include/edittest_defs.h;

  PRIV_INCS = [PRIV_INCS] [priv_incs];
}

/* Source files. */
EDITTEST_SRCS = edittest.c;
EDITTEST_SRCS = [fromto % [srcdir]/bin/edittest/src/% [EDITTEST_SRCS]];
SRCS = [SRCS] [EDITTEST_SRCS];

/* Simple tests.  Output is diff'ed with the expected (.exp file) output. */
{
  local ctests = ;
  ctests = [fromto % [srcdir]/bin/edittest/test/% [ctests]];

  CTESTS = [CTESTS] [ctests];
}

/* Perl scripts that drive the BTESTS. */
{
  local ptests = ;
  ptests = [fromto % [srcdir]/bin/edittest/test/% [ptests]];

  PTESTS = [PTESTS] [ptests];
}

/* Backend test programs that are driven by the PTESTS. */
{
  local btests = ;
  btests = [fromto % [srcdir]/bin/edittest/test/% [btests]];

  BTESTS = [BTESTS] [btests];
}

/* Man pages. */
{
  local manpages_1 = ;
  manpages_1 = [fromto % [srcdir]/bin/edittest/man/man1/% [manpages_1]];

  EDITTEST_MANPAGES_1 = [manpages_1];
  MANPAGES_1 = [MANPAGES_1] [manpages_1];
}

/* Binaries. */
EDITTEST = [objdir]/bin/edittest/src/edittest;
EDITTEST_D = [EDITTEST].dbg;
EDITTEST_A = [EDITTEST];
EDITTEST_P = [EDITTEST].prof;

D_BINS = [D_BINS] [EDITTEST_D];
A_BINS = [A_BINS] [EDITTEST_A];
P_BINS = [P_BINS] [EDITTEST_P];

/*
 * User cook'ables.
 */

edittest_all : [EDITTEST_D] [EDITTEST_A] [EDITTEST_P];

edittest_install : edittest_install_d edittest_install_a edittest_install_p;

edittest_uninstall :
{
  [SHELL] -e;
data
#![SHELL]

echo rm -f [PREFIX]/bin/edittest.dbg
rm -f [PREFIX]/bin/edittest.dbg

echo rm -f [PREFIX]/bin/edittest
rm -f [PREFIX]/bin/edittest

echo rm -f [PREFIX]/bin/edittest.prof
rm -f [PREFIX]/bin/edittest.prof

for i in [EDITTEST_MANPAGES_1]; do
	echo rm -f [PREFIX]/man/man1/`[BASENAME] $i`
	rm -f [PREFIX]/man/man1/`[BASENAME] $i`
done
dataend
}

edittest_clean :
{
}

edittest_distclean :
{
}

/*
 * Various flags.
 */

CPPFLAGS = [CPPFLAGS] -I[srcdir]/bin/edittest/include
                      -I[objdir]/bin/edittest/include;
D_CPPFLAGS = [D_CPPFLAGS] -D_EDITTEST_DBG;

/*
 * Build rules.
 */

edittest_install_common :
{
  [SHELL] -e;
data
#![SHELL]

echo [INSTALL] -d [PREFIX]/man/man1
[INSTALL] -d [PREFIX]/man/man1

for i in [EDITTEST_MANPAGES_1]; do
	echo [INSTALL] -m 0644 $i [PREFIX]/man/man1/
	[INSTALL] -m 0644 $i [PREFIX]/man/man1/
done

echo [INSTALL] -d [PREFIX]/bin
[INSTALL] -d [PREFIX]/bin
dataend
}

edittest_install_d : [EDITTEST_D] edittest_install_common
{
	[INSTALL] -m 0755 [EDITTEST_D] [PREFIX]/bin;
}

edittest_install_a : [EDITTEST_A] edittest_install_common
#if [ENABLE_SHARED]
                 libedittest_install_s libstash_install_s
#endif
{
#if [ENABLE_SHARED]
  [CC] [D_CFLAGS] -o [PREFIX]/bin/edittest
       [fromto [srcdir]/%0%.c [objdir]/%0%.o_a [EDITTEST_SRCS]]
       -rpath [PREFIX]/lib
       [LDFLAGS] -ledit -lstash -ltermcap [EXTRALIBS];
  [CHMOD] 0755 [PREFIX]/bin/edittest;
#else
	[INSTALL] -m 0755 [EDITTEST_A] [PREFIX]/bin;
#endif
}

edittest_install_p : [EDITTEST_P] edittest_install_common
{
	[INSTALL] -m 0755 [EDITTEST_P] [PREFIX]/bin;
}

/*
 * Dependencies.
 */

[EDITTEST_D] : [fromto [srcdir]/%0%.c [objdir]/%0%.o_d [EDITTEST_SRCS]]
           [LIBEDIT_D] [LIBSTASH_D]
  set mkdir
{
  [CC] [D_CFLAGS] -o [target]
       [fromto [srcdir]/%0%.c [objdir]/%0%.o_d [EDITTEST_SRCS]]
       [LDFLAGS] -ledit_d -lstash_d -ltermcap [EXTRALIBS];
}

[EDITTEST_A] : [fromto [srcdir]/%0%.c [objdir]/%0%.o_a [EDITTEST_SRCS]]
#if [ENABLE_SHARED]
           [LIBEDIT_S] [LIBSTASH_S]
#else
           [LIBEDIT_A] [LIBSTASH_A]
#endif
  set mkdir
{
  [CC] [A_CFLAGS] -o [target]
       [fromto [srcdir]/%0%.c [objdir]/%0%.o_a [EDITTEST_SRCS]]
#if [ENABLE_SHARED]
       -rpath [objdir]/lib/c/libedit/lib
       -rpath [objdir]/lib/c/libstash/lib
#endif
       [LDFLAGS] -ledit -lstash -ltermcap [EXTRALIBS];
}

[EDITTEST_P] : [fromto [srcdir]/%0%.c [objdir]/%0%.o_p [EDITTEST_SRCS]]
           [LIBEDIT_P] [LIBSTASH_P]
  set mkdir
{
  [CC] [P_CFLAGS] -o [target]
       [fromto [srcdir]/%0%.c [objdir]/%0%.o_p [EDITTEST_SRCS]]
       [LDFLAGS] -ledit_p -lstash_p -ltermcap [EXTRALIBS];
}
