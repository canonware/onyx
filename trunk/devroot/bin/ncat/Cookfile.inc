/****************************************************************************
 *
 * <Copyright = "jasone">
 * <License>
 *
 ****************************************************************************
 *
 * Version: <Version>
 *
 * cook file for ncat.
 *
 ****************************************************************************/

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

/*
 * Various flags.
 */

CPPFLAGS = [CPPFLAGS] -I[srcdir]/bin/ncat/include
                      -I[objdir]/bin/ncat/include;
D_CPPFLAGS = [D_CPPFLAGS] -D_NCAT_DBG;

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
           [LIBSOCK_A] [LIBSTASH_A]
  set mkdir
{
  [CC] [D_CFLAGS] -o [target]
       [fromto [srcdir]/%0%.c [objdir]/%0%.o_a [NCAT_SRCS]]
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
