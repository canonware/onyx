/******************************************************************************
 *
 * <Copyright = "jasone">
 * <License>
 *
 ******************************************************************************
 *
 * Version: <Version>
 *
 * cook file for ncat.
 *
 ******************************************************************************/

/* Include generated dependency files. */
#include-cooked [glob [objdir]/bin/ncat/src/"*.d_*"]

/*
 * File lists.
 */

/* Include files that _are_ installed. */
{
  local incs = ;
  incs = [fromto % [srcdir]/bin/ncat/include/% [incs]];

  INCS = [INCS] [incs];
}

/* Include files that are _not_ installed. */
{
  local priv_incs = ;
  priv_incs = [fromto % [srcdir]/bin/ncat/include/% [priv_incs]];
  priv_incs = [priv_incs] [objdir]/bin/ncat/include/ncat_defs.h;

  PRIV_INCS = [PRIV_INCS] [priv_incs];
}

/* Source files. */
NCAT_SRCS = ncat.c;
NCAT_SRCS = [fromto % [srcdir]/bin/ncat/src/% [NCAT_SRCS]];
SRCS = [SRCS] [NCAT_SRCS];

/* Simple tests.  Output is diff'ed with the expected (.exp file) output. */
{
  local ctests = ;
  ctests = [fromto % [srcdir]/bin/ncat/test/% [ctests]];

  CTESTS = [CTESTS] [ctests];
}

/* Perl scripts that drive the BTESTS. */
{
  local ptests = ;
  ptests = [fromto % [srcdir]/bin/ncat/test/% [ptests]];

  PTESTS = [PTESTS] [ptests];
}

/* Backend test programs that are driven by the PTESTS. */
{
  local btests = ;
  btests = [fromto % [srcdir]/bin/ncat/test/% [btests]];

  BTESTS = [BTESTS] [btests];
}

/* Man pages. */
{
  local manpages_1 = ;
  manpages_1 = [fromto % [srcdir]/bin/ncat/man/man1/% [manpages_1]];

  NCAT_MANPAGES_1 = [manpages_1];
  MANPAGES_1 = [MANPAGES_1] [manpages_1];
}

/* Binaries. */
NCAT = [objdir]/bin/ncat/src/ncat;
NCAT_D = [NCAT].dbg;
NCAT_A = [NCAT];
NCAT_P = [NCAT].prof;

D_BINS = [D_BINS] [NCAT_D];
A_BINS = [A_BINS] [NCAT_A];
P_BINS = [P_BINS] [NCAT_P];

/*
 * User cook'ables.
 */

ncat : [NCAT_D] [NCAT_A] [NCAT_P];

ncat_install : ncat_install_d ncat_install_a ncat_install_p;

ncat_uninstall :
{
  [SHELL] -e;
data
#![SHELL]

echo rm -f [PREFIX]/bin/ncat.dbg
rm -f [PREFIX]/bin/ncat.dbg

echo rm -f [PREFIX]/bin/ncat
rm -f [PREFIX]/bin/ncat

echo rm -f [PREFIX]/bin/ncat.prof
rm -f [PREFIX]/bin/ncat.prof

for i in [NCAT_MANPAGES_1]; do
	echo rm -f [PREFIX]/man/man1/`[BASENAME] $i`
	rm -f [PREFIX]/man/man1/`[BASENAME] $i`
done
dataend
}

/*
 * Various flags.
 */

CPPFLAGS = [CPPFLAGS] -I[srcdir]/bin/ncat/include
                      -I[objdir]/bin/ncat/include;
D_CPPFLAGS = [D_CPPFLAGS] -D_NCAT_DBG;

/*
 * Build rules.
 */

ncat_install_common :
{
  [SHELL] -e;
data
#![SHELL]

echo [INSTALL] -d [PREFIX]/man/man1
[INSTALL] -d [PREFIX]/man/man1

for i in [NCAT_MANPAGES_1]; do
	echo [INSTALL] -m 0644 $i [PREFIX]/man/man1/
	[INSTALL] -m 0644 $i [PREFIX]/man/man1/
done

echo [INSTALL] -d [PREFIX]/bin
[INSTALL] -d [PREFIX]/bin
dataend
}

ncat_install_d : [NCAT_D] ncat_install_common
{
	[INSTALL] -m 0755 [NCAT_D] [PREFIX]/bin;
}

ncat_install_a : [NCAT_A] ncat_install_common
#if [ENABLE_SHARED]
                 libsock_install_s libstash_install_s
#endif
{
#if [ENABLE_SHARED]
  [CC] [D_CFLAGS] -o [PREFIX]/bin/ncat
       [fromto [srcdir]/%0%.c [objdir]/%0%.o_a [NCAT_SRCS]]
       -rpath [PREFIX]/lib
       [LDFLAGS] -lsock -lstash [EXTRALIBS];
  [CHMOD] 0755 [PREFIX]/bin/ncat;
#else
	[INSTALL] -m 0755 [NCAT_A] [PREFIX]/bin;
#endif
}

ncat_install_p : [NCAT_P] ncat_install_common
{
	[INSTALL] -m 0755 [NCAT_P] [PREFIX]/bin;
}

/*
 * Dependencies.
 */

[NCAT_D] : [fromto [srcdir]/%0%.c [objdir]/%0%.o_d [NCAT_SRCS]]
           [LIBSOCK_D] [LIBSTASH_D]
  set mkdir
{
  [CC] [D_CFLAGS] -o [target]
       [fromto [srcdir]/%0%.c [objdir]/%0%.o_d [NCAT_SRCS]]
       [LDFLAGS] -lsock_d -lstash_d [EXTRALIBS];
}

[NCAT_A] : [fromto [srcdir]/%0%.c [objdir]/%0%.o_a [NCAT_SRCS]]
#if [ENABLE_SHARED]
           [LIBSOCK_S] [LIBSTASH_S]
#else
           [LIBSOCK_A] [LIBSTASH_A]
#endif
  set mkdir
{
  [CC] [D_CFLAGS] -o [target]
       [fromto [srcdir]/%0%.c [objdir]/%0%.o_a [NCAT_SRCS]]
#if [ENABLE_SHARED]
       -rpath [objdir]/lib/c/libsock/lib
       -rpath [objdir]/lib/c/libstash/lib
#endif
       [LDFLAGS] -lsock -lstash [EXTRALIBS];
}

[NCAT_P] : [fromto [srcdir]/%0%.c [objdir]/%0%.o_p [NCAT_SRCS]]
           [LIBSOCK_P] [LIBSTASH_P]
  set mkdir
{
  [CC] [D_CFLAGS] -o [target]
       [fromto [srcdir]/%0%.c [objdir]/%0%.o_p [NCAT_SRCS]]
       [LDFLAGS] -lsock_p -lstash_p [EXTRALIBS];
}
