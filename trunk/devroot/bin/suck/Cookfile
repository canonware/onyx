/******************************************************************************
 *
 * <Copyright = "jasone">
 * <License>
 *
 ******************************************************************************
 *
 * Version: <Version>
 *
 * cook file for suck.
 *
 ******************************************************************************/

/* Include generated dependency files. */
#include-cooked [glob [objdir]/bin/suck/src/"*.d_*"]

/*
 * File lists.
 */

/* Include files that _are_ installed. */
{
  local incs = ;
  incs = [fromto % [srcdir]/bin/suck/include/% [incs]];

  INCS = [INCS] [incs];
}

/* Include files that are _not_ installed. */
{
  local priv_incs = ;
  priv_incs = [fromto % [srcdir]/bin/suck/include/% [priv_incs]];
  priv_incs = [priv_incs] [objdir]/bin/suck/include/suck_defs.h;

  PRIV_INCS = [PRIV_INCS] [priv_incs];
}

/* Source files. */
SUCK_SRCS = suck.c;
SUCK_SRCS = [fromto % [srcdir]/bin/suck/src/% [SUCK_SRCS]];
SRCS = [SRCS] [SUCK_SRCS];

/* Simple tests.  Output is diff'ed with the expected (.exp file) output. */
{
  local ctests = ;
  ctests = [fromto % [srcdir]/bin/suck/test/% [ctests]];

  CTESTS = [CTESTS] [ctests];
}

/* Perl scripts that drive the BTESTS. */
{
  local ptests = ;
  ptests = [fromto % [srcdir]/bin/suck/test/% [ptests]];

  PTESTS = [PTESTS] [ptests];
}

/* Backend test programs that are driven by the PTESTS. */
{
  local btests = ;
  btests = [fromto % [srcdir]/bin/suck/test/% [btests]];

  BTESTS = [BTESTS] [btests];
}

/* Man pages. */
{
  local manpages_1 = ;
  manpages_1 = [fromto % [srcdir]/bin/suck/man/man1/% [manpages_1]];

  SUCK_MANPAGES_1 = [manpages_1];
  MANPAGES_1 = [MANPAGES_1] [manpages_1];
}

/* Binaries. */
SUCK = [objdir]/bin/suck/src/suck;
SUCK_D = [SUCK].dbg;
SUCK_A = [SUCK];
SUCK_P = [SUCK].prof;

D_BINS = [D_BINS] [SUCK_D];
A_BINS = [A_BINS] [SUCK_A];
P_BINS = [P_BINS] [SUCK_P];

/*
 * User cook'ables.
 */

suck : [SUCK_D] [SUCK_A] [SUCK_P];

suck_install : suck_install_d suck_install_a suck_install_p;

suck_uninstall :
{
  [SHELL] -e;
data
#![SHELL]

echo rm -f [PREFIX]/bin/suck.dbg
rm -f [PREFIX]/bin/suck.dbg

echo rm -f [PREFIX]/bin/suck
rm -f [PREFIX]/bin/suck

echo rm -f [PREFIX]/bin/suck.prof
rm -f [PREFIX]/bin/suck.prof

for i in [SUCK_MANPAGES_1]; do
	echo rm -f [PREFIX]/man/man1/`[BASENAME] $i`
	rm -f [PREFIX]/man/man1/`[BASENAME] $i`
done
dataend
}

/*
 * Various flags.
 */

CPPFLAGS = [CPPFLAGS] -I[srcdir]/bin/suck/include
                      -I[objdir]/bin/suck/include;
D_CPPFLAGS = [D_CPPFLAGS] -D_SUCK_DBG;

/*
 * Build rules.
 */

suck_install_common :
{
  [SHELL] -e;
data
#![SHELL]

echo [INSTALL] -d [PREFIX]/man/man1
[INSTALL] -d [PREFIX]/man/man1

for i in [SUCK_MANPAGES_1]; do
	echo [INSTALL] -m 0644 $i [PREFIX]/man/man1/
	[INSTALL] -m 0644 $i [PREFIX]/man/man1/
done

echo [INSTALL] -d [PREFIX]/bin
[INSTALL] -d [PREFIX]/bin
dataend
}

suck_install_d : [SUCK_D] suck_install_common
{
	[INSTALL] -m 0755 [SUCK_D] [PREFIX]/bin;
}

suck_install_a : [SUCK_A] suck_install_common
#if [ENABLE_SHARED]
                 libsock_install_s libstash_install_s
#endif
{
#if [ENABLE_SHARED]
  [CC] [D_CFLAGS] -o [PREFIX]/bin/suck
       [fromto [srcdir]/%0%.c [objdir]/%0%.o_a [SUCK_SRCS]]
       -rpath [PREFIX]/lib
       [LDFLAGS] -lsock -lstash [EXTRALIBS];
  [CHMOD] 0755 [PREFIX]/bin/suck;
#else
	[INSTALL] -m 0755 [SUCK_A] [PREFIX]/bin;
#endif
}

suck_install_p : [SUCK_P] suck_install_common
{
	[INSTALL] -m 0755 [SUCK_P] [PREFIX]/bin;
}

/*
 * Dependencies.
 */

[SUCK_D] : [fromto [srcdir]/%0%.c [objdir]/%0%.o_d [SUCK_SRCS]]
           [LIBSOCK_D] [LIBSTASH_D]
  set mkdir
{
  [CC] [D_CFLAGS] -o [target]
       [fromto [srcdir]/%0%.c [objdir]/%0%.o_d [SUCK_SRCS]]
       [LDFLAGS] -lsock_d -lstash_d [EXTRALIBS];
}

[SUCK_A] : [fromto [srcdir]/%0%.c [objdir]/%0%.o_a [SUCK_SRCS]]
#if [ENABLE_SHARED]
           [LIBSOCK_S] [LIBSTASH_S]
#else
           [LIBSOCK_A] [LIBSTASH_A]
#endif
  set mkdir
{
  [CC] [D_CFLAGS] -o [target]
       [fromto [srcdir]/%0%.c [objdir]/%0%.o_a [SUCK_SRCS]]
#if [ENABLE_SHARED]
       -rpath [objdir]/lib/c/libsock/lib
       -rpath [objdir]/lib/c/libstash/lib
#endif
       [LDFLAGS] -lsock -lstash [EXTRALIBS];
}

[SUCK_P] : [fromto [srcdir]/%0%.c [objdir]/%0%.o_p [SUCK_SRCS]]
           [LIBSOCK_P] [LIBSTASH_P]
  set mkdir
{
  [CC] [D_CFLAGS] -o [target]
       [fromto [srcdir]/%0%.c [objdir]/%0%.o_p [SUCK_SRCS]]
       [LDFLAGS] -lsock_p -lstash_p [EXTRALIBS];
}
