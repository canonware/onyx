/******************************************************************************
 *
 * <Copyright = jasone>
 * <License>
 *
 ******************************************************************************
 *
 * Version: <Version>
 *
 * cook file for libsock.
 *
 ******************************************************************************/

/* Include generated dependency files. */
#include-cooked [glob [objdir]/lib/c/libsock/src/"*.d_*"]
#include-cooked [glob [objdir]/lib/c/libsock/bin/"*.d_*"]
#include-cooked [glob [objdir]/lib/c/libsock/test/"*.d_*"]

/*
 * File lists.
 */

/* Include files that _are_ installed. */
{
  local incs = libsock.h libsock_incs.h sock.h socks.h;
  incs = [fromto % [srcdir]/lib/c/libsock/include/libsock/% [incs]];
  incs = [incs] [objdir]/lib/c/libsock/include/libsock/libsock_defs.h;

  LIBSOCK_INCS = [incs];
  INCS = [INCS] [incs];
}

/* Include files that are _not_ installed. */
{
  local priv_incs = libsock_l.h sock_l.h;
  priv_incs = [fromto % [srcdir]/lib/c/libsock/include/libsock/% [priv_incs]];

  PRIV_INCS = [PRIV_INCS] [priv_incs];
}

/* Source files. */
LIBSOCK_SRCS = libsock.c sock.c socks.c;
LIBSOCK_SRCS = [fromto % [srcdir]/lib/c/libsock/src/% [LIBSOCK_SRCS]];
SRCS = [SRCS] [LIBSOCK_SRCS];

/* Simple C tests.  Output is diff'ed with the expected (.exp file) output. */
{
  local ctests = c_sock_a.c;
  ctests = [fromto % [srcdir]/lib/c/libsock/test/% [ctests]];

  CTESTS = [CTESTS] [ctests];
}

/*
 * Simple stil tests.  Output is diff'ed with the expected (.exp file)
 * output.
 */
{
  local stests = ;
  stests = [fromto % [srcdir]/lib/c/libsock/test/% [stests]];

  STESTS = [STESTS] [stests];
}

/* Perl scripts that drive other tests and use the sequence mode of verify. */
{
  local ptests = ;
  ptests = [fromto % [srcdir]/lib/c/libsock/test/% [ptests]];

  PTESTS = [PTESTS] [ptests];
}

/* C programs that are driven by tests. */
{
  local cbtests = ;
  cbtests = [fromto % [srcdir]/lib/c/libsock/test/% [cbtests]];

  CBTESTS = [CBTESTS] [cbtests];
}

/* stil programs that are driven by tests. */
{
  local sbtests = ;
  sbtests = [fromto % [srcdir]/lib/c/libsock/test/% [sbtests]];

  SBTESTS = [SBTESTS] [sbtests];
}

/* LaTeX source files. */
{
  local tex_srcs = sock.tex socks.tex;
  tex_srcs = [fromto % [srcdir]/lib/c/libsock/doc/latex/% [tex_srcs]];
  tex_srcs = [tex_srcs] [objdir]/lib/c/libsock/doc/latex/manual.tex;

  TEX_SRCS = [TEX_SRCS] [tex_srcs];
}

/* xfig source files. */
{
  local fig_srcs = ;
  fig_srcs = [fromto % [srcdir]/lib/c/libsock/doc/latex/% [fig_srcs]];

  FIG_SRCS = [FIG_SRCS] [fig_srcs];
}

/* Libraries. */
LIBSOCK_REV = 0.0;

LIBSOCK_D = [objdir]/lib/c/libsock/lib/libsock_d.a;
LIBSOCK_A = [objdir]/lib/c/libsock/lib/libsock.a;
LIBSOCK_S = [objdir]/lib/c/libsock/lib/libsock.so.[LIBSOCK_REV];
LIBSOCK_P = [objdir]/lib/c/libsock/lib/libsock_p.a;

LDFLAGS = [LDFLAGS] -L[objdir]/lib/c/libsock/lib;
LIBS = [LIBS] -lsock;
D_LIBS = [D_LIBS] [LIBSOCK_D];
A_LIBS = [A_LIBS] [LIBSOCK_A];
P_LIBS = [P_LIBS] [LIBSOCK_P];
S_LIBS = [S_LIBS] [LIBSOCK_S];
S_LINKS = [S_LINKS] [objdir]/lib/c/libsock/lib/libsock.so;

/*
 * User cook'ables.
 */

libsock : [LIBSOCK_D] [LIBSOCK_A] [LIBSOCK_P]
#if [ENABLE_SHARED]
          [LIBSOCK_S]
#endif
;

libsock_install : libsock_install_d libsock_install_a libsock_install_p
#if [ENABLE_SHARED]
                   libsock_install_s
#endif
;

libsock_uninstall :
{
  [SHELL] -e;
data
#![SHELL]

echo rm -f [PREFIX]/lib/libsock*
rm -f [PREFIX]/lib/libsock*

echo rm -rf [PREFIX]/include/libsock
rm -rf [PREFIX]/include/libsock
dataend
}

libsock_clean :
{
}

libsock_distclean :
{
  rm -f [objdir]/lib/c/libsock/doc/latex/manual.tex;
}

/*
 * Various flags.
 */

CPPFLAGS = [CPPFLAGS] -I[srcdir]/lib/c/libsock/include
                      -I[objdir]/lib/c/libsock/include/libsock;
D_CPPFLAGS = [D_CPPFLAGS] -D_LIBSOCK_DBG;

/*
 * Build rules.
 */

libsock_install_common :
{
  [SHELL] -e;
data
#![SHELL]

echo [INSTALL] -d [PREFIX]/include/libsock
[INSTALL] -d [PREFIX]/include/libsock

for i in [LIBSOCK_INCS]; do
	echo [INSTALL] -m 0644 $i [PREFIX]/include/libsock/
	[INSTALL] -m 0644 $i [PREFIX]/include/libsock/
done

echo [INSTALL] -d [PREFIX]/lib
[INSTALL] -d [PREFIX]/lib
dataend
}

libsock_install_d : [LIBSOCK_D] libsock_install_common
{
	[INSTALL] -m 0444 [LIBSOCK_D] [PREFIX]/lib;
}

libsock_install_a : [LIBSOCK_A] libsock_install_common
{
	[INSTALL] -m 0444 [LIBSOCK_A] [PREFIX]/lib;
}

libsock_install_p : [LIBSOCK_P] libsock_install_common
{
	[INSTALL] -m 0444 [LIBSOCK_P] [PREFIX]/lib;
}

libsock_install_s : [LIBSOCK_S] libsock_install_common
{
	[INSTALL] -m 0444 [LIBSOCK_S] [PREFIX]/lib;
	[LN] -sf libsock.so.[LIBSOCK_REV] [PREFIX]/lib/libsock.so;
}

/*
 * Dependencies.
 */

[LIBSOCK_D] : [fromto [srcdir]/%0%.c [objdir]/%0%.o_d [LIBSOCK_SRCS]]
  set mkdir
{
  [AR] cru [target] [younger];
  [RANLIB] [target];
}

[LIBSOCK_A] : [fromto [srcdir]/%0%.c [objdir]/%0%.o_a [LIBSOCK_SRCS]]
  set mkdir
{
  [AR] cru [target] [younger];
  [RANLIB] [target];
}

[LIBSOCK_P] : [fromto [srcdir]/%0%.c [objdir]/%0%.o_p [LIBSOCK_SRCS]]
  set mkdir
{
  [AR] cru [target] [younger];
  [RANLIB] [target];
}

[LIBSOCK_S] : [fromto [srcdir]/%0%.c [objdir]/%0%.o_s [LIBSOCK_SRCS]]
  set mkdir
{
  [CC] -shared -o [target] [need];
  [LN] -sf libsock.so.[LIBSOCK_REV] [objdir]/lib/c/libsock/lib/libsock.so;
}
