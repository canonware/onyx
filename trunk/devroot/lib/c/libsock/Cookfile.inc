/****************************************************************************
 *
 * <Copyright = "jasone">
 * <License>
 *
 ****************************************************************************
 *
 * Version: <Version>
 *
 * cook file for libsock.
 *
 ****************************************************************************/

/* Include generated dependency files. */
#include-cooked [glob [objdir]/lib/c/libsock/src/"*.d_*"]
#include-cooked [glob [objdir]/lib/c/libsock/bin/"*.d_*"]
#include-cooked [glob [objdir]/lib/c/libsock/test/"*.d_*"]

/*
 * File lists.
 */

/* Include files that _are_ installed. */
{
  local incs = libsock.h libsock_incs.h sock.h sockb.h socks.h;
  incs = [fromto % [srcdir]/lib/c/libsock/include/libsock/% [incs]];
  incs = [incs] [objdir]/lib/c/libsock/include/libsock/libsock_defs.h;

  INCS = [INCS] [incs];
}

/* Include files that are _not_ installed. */
{
  local priv_incs = sock_p.h sockb_p.h socks_p.h;
  priv_incs = [fromto % [srcdir]/lib/c/libsock/include/libsock/% [priv_incs]];

  PRIV_INCS = [PRIV_INCS] [priv_incs];
}

/* Source files. */
LIBSOCK_SRCS = sock.c sockb.c socks.c;
LIBSOCK_SRCS = [fromto % [srcdir]/lib/c/libsock/src/% [LIBSOCK_SRCS]];
SRCS = [SRCS] [LIBSOCK_SRCS];

/* Executables. */
{
  local bins = proxy.c ncat.c blow.c suck.c echos.c;
  bins = [fromto % [srcdir]/lib/c/libsock/bin/% [bins]];

  BINS = [BINS] [bins];
}

/* Simple tests.  Output is diff'ed with the expected (.exp file) output. */
{
  local ctests = ;
  ctests = [fromto % [srcdir]/lib/c/libsock/test/% [ctests]];

  CTESTS = [CTESTS] [ctests];
}

/* Perl scripts that drive the BTESTS. */
{
  local ptests =;
  ptests = [fromto % [srcdir]/lib/c/libsock/test/% [ptests]];

  PTESTS = [PTESTS] [ptests];
}

/* Backend test programs that are driven by the PTESTS. */
{
  local btests =;
  btests = [fromto % [srcdir]/lib/c/libsock/test/% [btests]];

  BTESTS = [BTESTS] [btests];
}

/* Man pages. */
{
  local manpages_3 = libsock.3s libsock_sock.3s libsock_sockb.3s
                     libsock_socks.3s;
  manpages_3 = [fromto % [srcdir]/lib/c/libsock/man/man3/% [manpages_3]];

  MANPAGES_3 = [MANPAGES_3] [manpages_3];
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

/*
 * User cook'ables.
 */

libsock : [LIBSOCK_D] [LIBSOCK_A] [LIBSOCK_P] [LIBSOCK_S];

/*
 * Various flags.
 */

CPPFLAGS = [CPPFLAGS] -I[srcdir]/lib/c/libsock/include
                      -I[objdir]/lib/c/libsock/include/libsock;
D_CPPFLAGS = [D_CPPFLAGS] -D_LIBSOCK_DBG;

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
}
