/****************************************************************************
 *
 * <Copyright = "jasone">
 * <License>
 *
 ****************************************************************************
 *
 * Version: <Version>
 *
 * cook file for proxy.
 *
 ****************************************************************************/

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

/* Simple tests.  Output is diff'ed with the expected (.exp file) output. */
{
  local ctests = ;
  ctests = [fromto % [srcdir]/bin/proxy/test/% [ctests]];

  CTESTS = [CTESTS] [ctests];
}

/* Perl scripts that drive the BTESTS. */
{
  local ptests = ;
  ptests = [fromto % [srcdir]/bin/proxy/test/% [ptests]];

  PTESTS = [PTESTS] [ptests];
}

/* Backend test programs that are driven by the PTESTS. */
{
  local btests = ;
  btests = [fromto % [srcdir]/bin/proxy/test/% [btests]];

  BTESTS = [BTESTS] [btests];
}

/* Man pages. */
{
  local manpages_1 = ;
  manpages_1 = [fromto % [srcdir]/bin/proxy/man/man1/% [manpages_1]];

  MANPAGES_1 = [MANPAGES_1] [manpages_1];
}

/* Binaries. */
PROXY = [objdir]/bin/proxy/src/proxy;
PROXY_D = [PROXY].dbg;
PROXY_A = [PROXY];
PROXY_P = [PROXY].prof;

D_BINS = [D_BINS] [PROXY_D];
A_BINS = [A_BINS] [PROXY_A];
P_BINS = [P_BINS] [PROXY_P];

/*
 * User cook'ables.
 */

proxy : [PROXY_D] [PROXY_A] [PROXY_P];

/*
 * Various flags.
 */

CPPFLAGS = [CPPFLAGS] -I[srcdir]/bin/proxy/include
                      -I[objdir]/bin/proxy/include;
D_CPPFLAGS = [D_CPPFLAGS] -D_PROXY_DBG;

/*
 * Dependencies.
 */

[PROXY_D] : [fromto [srcdir]/%0%.c [objdir]/%0%.o_d [PROXY_SRCS]] [D_LIBS]
  set mkdir
{
  [CC] [D_CFLAGS] -o [target]
       [fromto [srcdir]/%0%.c [objdir]/%0%.o_d [PROXY_SRCS]]
       [D_LIBS] [LDFLAGS] [D_LIBS] [EXTRALIBS];
}

[PROXY_A] : [fromto [srcdir]/%0%.c [objdir]/%0%.o_a [PROXY_SRCS]] [A_LIBS]
  set mkdir
{
  [CC] [D_CFLAGS] -o [target]
       [fromto [srcdir]/%0%.c [objdir]/%0%.o_a [PROXY_SRCS]]
       [D_LIBS] [LDFLAGS] [A_LIBS] [EXTRALIBS];
}

[PROXY_P] : [fromto [srcdir]/%0%.c [objdir]/%0%.o_p [PROXY_SRCS]] [P_LIBS]
  set mkdir
{
  [CC] [D_CFLAGS] -o [target]
       [fromto [srcdir]/%0%.c [objdir]/%0%.o_p [PROXY_SRCS]]
       [D_LIBS] [LDFLAGS] [P_LIBS] [EXTRALIBS];
}
