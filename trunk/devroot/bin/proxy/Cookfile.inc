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
  stests = [fromto % [srcdir]/bin/proxy/test/% [stests]];

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

/* LaTeX source files. */
{
  local tex_srcs = manual.tex;
  tex_srcs = [fromto % [srcdir]/bin/proxy/doc/latex/% [tex_srcs]];

  TEX_SRCS = [TEX_SRCS] [tex_srcs];
}

/* xfig source files. */
{
  local fig_srcs = ;
  fig_srcs = [fromto % [srcdir]/bin/proxy/doc/latex/% [fig_srcs]];

  FIG_SRCS = [FIG_SRCS] [fig_srcs];
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
dataend
}

proxy_clean :
{
}

proxy_distclean :
{
  rm -f [objdir]/bin/proxy/doc/latex/manual.tex;
}

/*
 * Build rules.
 */

proxy_install_common :
{
  [SHELL] -e;
data
#![SHELL]

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
