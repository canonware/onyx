/******************************************************************************
 *
 * <Copyright = "jasone">
 * <License>
 *
 ******************************************************************************
 *
 * Version: <Version>
 *
 * cook file for echos.
 *
 ******************************************************************************/

/* Include generated dependency files. */
#include-cooked [glob [objdir]/bin/echos/src/"*.d_*"]

/*
 * File lists.
 */

/* Include files that _are_ installed. */
{
  local incs = ;
  incs = [fromto % [srcdir]/bin/echos/include/% [incs]];

  INCS = [INCS] [incs];
}

/* Include files that are _not_ installed. */
{
  local priv_incs = ;
  priv_incs = [fromto % [srcdir]/bin/echos/include/% [priv_incs]];
  priv_incs = [priv_incs] [objdir]/bin/echos/include/echos_defs.h;

  PRIV_INCS = [PRIV_INCS] [priv_incs];
}

/* Source files. */
ECHOS_SRCS = echos.c;
ECHOS_SRCS = [fromto % [srcdir]/bin/echos/src/% [ECHOS_SRCS]];
SRCS = [SRCS] [ECHOS_SRCS];

/* Simple tests.  Output is diff'ed with the expected (.exp file) output. */
{
  local ctests = ;
  ctests = [fromto % [srcdir]/bin/echos/test/% [ctests]];

  CTESTS = [CTESTS] [ctests];
}

/* Perl scripts that drive the BTESTS. */
{
  local ptests = ;
  ptests = [fromto % [srcdir]/bin/echos/test/% [ptests]];

  PTESTS = [PTESTS] [ptests];
}

/* Backend test programs that are driven by the PTESTS. */
{
  local btests = ;
  btests = [fromto % [srcdir]/bin/echos/test/% [btests]];

  BTESTS = [BTESTS] [btests];
}

/* Man pages. */
{
  local manpages_1 = ;
  manpages_1 = [fromto % [srcdir]/bin/echos/man/man1/% [manpages_1]];

  ECHOS_MANPAGES_1 = [manpages_1];
  MANPAGES_1 = [MANPAGES_1] [manpages_1];
}

/* Binaries. */
ECHOS = [objdir]/bin/echos/src/echos;
ECHOS_D = [ECHOS].dbg;
ECHOS_A = [ECHOS];
ECHOS_P = [ECHOS].prof;

D_BINS = [D_BINS] [ECHOS_D];
A_BINS = [A_BINS] [ECHOS_A];
P_BINS = [P_BINS] [ECHOS_P];

/*
 * User cook'ables.
 */

echos : [ECHOS_D] [ECHOS_A] [ECHOS_P];

echos_install : echos_install_d echos_install_a echos_install_p;

echos_uninstall :
{
  [SHELL] -e;
data
#![SHELL]

echo rm -f [PREFIX]/bin/echos.dbg
rm -f [PREFIX]/bin/echos.dbg

echo rm -f [PREFIX]/bin/echos
rm -f [PREFIX]/bin/echos

echo rm -f [PREFIX]/bin/echos.prof
rm -f [PREFIX]/bin/echos.prof

for i in [ECHOS_MANPAGES_1]; do
	echo rm -f [PREFIX]/man/man1/`[BASENAME] $i`
	rm -f [PREFIX]/man/man1/`[BASENAME] $i`
done
dataend
}

/*
 * Various flags.
 */

CPPFLAGS = [CPPFLAGS] -I[srcdir]/bin/echos/include
                      -I[objdir]/bin/echos/include;
D_CPPFLAGS = [D_CPPFLAGS] -D_ECHOS_DBG;

/*
 * Build rules.
 */

echos_install_common :
{
  [SHELL] -e;
data
#![SHELL]

echo [INSTALL] -d [PREFIX]/man/man1
[INSTALL] -d [PREFIX]/man/man1

for i in [ECHOS_MANPAGES_1]; do
	echo [INSTALL] -m 0644 $i [PREFIX]/man/man1/
	[INSTALL] -m 0644 $i [PREFIX]/man/man1/
done

echo [INSTALL] -d [PREFIX]/bin
[INSTALL] -d [PREFIX]/bin
dataend
}

echos_install_d : [ECHOS_D] echos_install_common
{
	[INSTALL] -m 0755 [ECHOS_D] [PREFIX]/bin;
}

echos_install_a : [ECHOS_A] echos_install_common
#if [ENABLE_SHARED]
                  libsock_install_s libstash_install_s
#endif
{
#if [ENABLE_SHARED]
  [CC] [D_CFLAGS] -o [PREFIX]/bin/echos
       [fromto [srcdir]/%0%.c [objdir]/%0%.o_a [ECHOS_SRCS]]
       -rpath [PREFIX]/lib
       [LDFLAGS] -lsock -lstash [EXTRALIBS];
  [CHMOD] 0755 [PREFIX]/bin/echos;
#else
	[INSTALL] -m 0755 [ECHOS_A] [PREFIX]/bin;
#endif
}

echos_install_p : [ECHOS_P] echos_install_common
{
	[INSTALL] -m 0755 [ECHOS_P] [PREFIX]/bin;
}

/*
 * Dependencies.
 */

[ECHOS_D] : [fromto [srcdir]/%0%.c [objdir]/%0%.o_d [ECHOS_SRCS]]
           [LIBSOCK_D] [LIBSTASH_D]
  set mkdir
{
  [CC] [D_CFLAGS] -o [target]
       [fromto [srcdir]/%0%.c [objdir]/%0%.o_d [ECHOS_SRCS]]
       [LDFLAGS] -lsock_d -lstash_d [EXTRALIBS];
}

[ECHOS_A] : [fromto [srcdir]/%0%.c [objdir]/%0%.o_a [ECHOS_SRCS]]
#if [ENABLE_SHARED]
           [LIBSOCK_S] [LIBSTASH_S]
#else
           [LIBSOCK_A] [LIBSTASH_A]
#endif
  set mkdir
{
  [CC] [D_CFLAGS] -o [target]
       [fromto [srcdir]/%0%.c [objdir]/%0%.o_a [ECHOS_SRCS]]
#if [ENABLE_SHARED]
       -rpath [objdir]/lib/c/libsock/lib
       -rpath [objdir]/lib/c/libstash/lib
#endif
       [LDFLAGS] -lsock -lstash [EXTRALIBS];
}

[ECHOS_P] : [fromto [srcdir]/%0%.c [objdir]/%0%.o_p [ECHOS_SRCS]]
           [LIBSOCK_P] [LIBSTASH_P]
  set mkdir
{
  [CC] [D_CFLAGS] -o [target]
       [fromto [srcdir]/%0%.c [objdir]/%0%.o_p [ECHOS_SRCS]]
       [LDFLAGS] -lsock_p -lstash_p [EXTRALIBS];
}
