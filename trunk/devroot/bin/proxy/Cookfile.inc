/******************************************************************************
 *
 * <Copyright = jasone>
 * <License>
 *
 ******************************************************************************
 *
 * Version: <Version>
 *
 * cook file for proxy.
 *
 ******************************************************************************/

/* Include generated dependency files. */
#include-cooked [glob [objdir]/bin/proxy/src/"*.d_*"]

/*
 * File lists.
 */

/* Include files that _are_ installed. */
{
  local incs = ;
  incs = [fromto % [srcdir]/bin/proxy/include/% [incs]];

  INCS = [INCS] [incs];
}

/* Include files that are _not_ installed. */
{
  local priv_incs = ;
  priv_incs = [fromto % [srcdir]/bin/proxy/include/% [priv_incs]];
  priv_incs = [priv_incs] [objdir]/bin/proxy/include/proxy_defs.h;

  PRIV_INCS = [PRIV_INCS] [priv_incs];
}

/* Source files. */
PROXY_SRCS = proxy.c;
PROXY_SRCS = [fromto % [srcdir]/bin/proxy/src/% [PROXY_SRCS]];
SRCS = [SRCS] [PROXY_SRCS];

/* Simple C tests.  Output is diff'ed with the expected (.exp file) output. */
{
  local ctests = ;
  ctests = [fromto % [srcdir]/bin/proxy/test/% [ctests]];

  CTESTS = [CTESTS] [ctests];
}

/*
 * Simple stil tests.  Output is diff'ed with the expected (.exp file)
 * output.
 */
{
  local stests = ;
  ptests = [fromto % [srcdir]/bin/proxy/test/% [stests]];

  STESTS = [STESTS] [stests];
}

/* Perl scripts that drive other tests and use the sequence mode of verify. */
{
  local ptests = ;
  ptests = [fromto % [srcdir]/bin/proxy/test/% [ptests]];

  PTESTS = [PTESTS] [ptests];
}

/* C programs that are driven by tests. */
{
  local cbtests = ;
  cbtests = [fromto % [srcdir]/bin/proxy/test/% [cbtests]];

  CBTESTS = [CBTESTS] [cbtests];
}

/* stil programs that are driven by tests. */
{
  local sbtests = ;
  sbtests = [fromto % [srcdir]/bin/proxy/test/% [sbtests]];

  SBTESTS = [SBTESTS] [sbtests];
}

/* Man pages. */
{
  local manpages_1 = proxy.1;
  manpages_1 = [fromto % [srcdir]/bin/proxy/man/man1/% [manpages_1]];

  PROXY_MANPAGES_1 = [manpages_1];
  MANPAGES_1 = [MANPAGES_1] [manpages_1];
}

/* Binaries. */
PROXY = [objdir]/bin/proxy/bin/proxy;
PROXY_D = [PROXY].dbg;
PROXY_A = [PROXY];
PROXY_P = [PROXY].prof;

D_BINS = [D_BINS] [PROXY_D];
A_BINS = [A_BINS] [PROXY_A];
P_BINS = [P_BINS] [PROXY_P];

/*
 * User cook'ables.
 */

proxy_all : proxy_d proxy_a proxy_p;
proxy_d : [PROXY_D];
proxy_a : [PROXY_A];
proxy_p : [PROXY_P];

proxy_install : proxy_install_d proxy_install_a proxy_install_p;

proxy_uninstall :
{
  [SHELL] -e;
data
#![SHELL]

echo rm -f [PREFIX]/bin/proxy.dbg
rm -f [PREFIX]/bin/proxy.dbg

echo rm -f [PREFIX]/bin/proxy
rm -f [PREFIX]/bin/proxy

echo rm -f [PREFIX]/bin/proxy.prof
rm -f [PREFIX]/bin/proxy.prof

for i in [PROXY_MANPAGES_1]; do
	echo rm -f [PREFIX]/man/man1/`[BASENAME] $i`
	rm -f [PREFIX]/man/man1/`[BASENAME] $i`
done
dataend
}

proxy_clean :
{
}

proxy_distclean :
{
}

/*
 * Build rules.
 */

proxy_install_common :
{
  [SHELL] -e;
data
#![SHELL]

echo [INSTALL] -d [PREFIX]/man/man1
[INSTALL] -d [PREFIX]/man/man1

for i in [PROXY_MANPAGES_1]; do
	echo [INSTALL] -m 0644 $i [PREFIX]/man/man1/
	[INSTALL] -m 0644 $i [PREFIX]/man/man1/
done

echo [INSTALL] -d [PREFIX]/bin
[INSTALL] -d [PREFIX]/bin
dataend
}

proxy_install_d : [PROXY_D] proxy_install_common
{
	[INSTALL] -m 0755 [PROXY_D] [PREFIX]/bin;
}

proxy_install_a : [PROXY_A] proxy_install_common
#if [ENABLE_SHARED]
                  libsock_install_s libstash_install_s
#endif
{
#if [ENABLE_SHARED]
  [CC] [D_CFLAGS] -o [PREFIX]/bin/proxy
       [fromto [srcdir]/%0%.c [objdir]/%0%.o_a [PROXY_SRCS]]
       -rpath [PREFIX]/lib
       [LDFLAGS] -lsock -lstash [EXTRALIBS];
  [CHMOD] 0755 [PREFIX]/bin/proxy;
#else
	[INSTALL] -m 0755 [PROXY_A] [PREFIX]/bin;
#endif
}

proxy_install_p : [PROXY_P] proxy_install_common
{
	[INSTALL] -m 0755 [PROXY_P] [PREFIX]/bin;
}

/*
 * Dependencies.
 */

[PROXY_D] : [fromto [srcdir]/%0%.c [objdir]/%0%.o_d [PROXY_SRCS]]
           [LIBSOCK_D] [LIBSTASH_D]
  set mkdir
{
  [CC] [D_CFLAGS] -o [target]
       [fromto [srcdir]/%0%.c [objdir]/%0%.o_d [PROXY_SRCS]]
       [LDFLAGS] -lsock_d -lstash_d [EXTRALIBS];
}

[PROXY_A] : [fromto [srcdir]/%0%.c [objdir]/%0%.o_a [PROXY_SRCS]]
#if [ENABLE_SHARED]
           [LIBSOCK_S] [LIBSTASH_S]
#else
           [LIBSOCK_A] [LIBSTASH_A]
#endif
  set mkdir
{
  [CC] [A_CFLAGS] -o [target]
       [fromto [srcdir]/%0%.c [objdir]/%0%.o_a [PROXY_SRCS]]
#if [ENABLE_SHARED]
       -rpath [objdir]/lib/c/libsock/lib
       -rpath [objdir]/lib/c/libstash/lib
#endif
       [LDFLAGS] -lsock -lstash [EXTRALIBS];
}

[PROXY_P] : [fromto [srcdir]/%0%.c [objdir]/%0%.o_p [PROXY_SRCS]]
           [LIBSOCK_P] [LIBSTASH_P]
  set mkdir
{
  [CC] [P_CFLAGS] -o [target]
       [fromto [srcdir]/%0%.c [objdir]/%0%.o_p [PROXY_SRCS]]
       [LDFLAGS] -lsock_p -lstash_p [EXTRALIBS];
}
