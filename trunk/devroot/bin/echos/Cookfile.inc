/****************************************************************************
 *
 * <Copyright = "jasone">
 * <License>
 *
 ****************************************************************************
 *
 * Version: <Version>
 *
 * cook file for echos.
 *
 ****************************************************************************/

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

/*
 * Various flags.
 */

CPPFLAGS = [CPPFLAGS] -I[srcdir]/bin/echos/include
                      -I[objdir]/bin/echos/include;
D_CPPFLAGS = [D_CPPFLAGS] -D_ECHOS_DBG;

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
            [LIBSOCK_A] [LIBSTASH_A]
  set mkdir
{
  [CC] [D_CFLAGS] -o [target]
       [fromto [srcdir]/%0%.c [objdir]/%0%.o_a [ECHOS_SRCS]]
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
