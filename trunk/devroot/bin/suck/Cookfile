/****************************************************************************
 *
 * <Copyright = "jasone">
 * <License>
 *
 ****************************************************************************
 *
 * Version: <Version>
 *
 * cook file for suck.
 *
 ****************************************************************************/

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

/*
 * Various flags.
 */

CPPFLAGS = [CPPFLAGS] -I[srcdir]/bin/suck/include
                      -I[objdir]/bin/suck/include;
D_CPPFLAGS = [D_CPPFLAGS] -D_SUCK_DBG;

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
           [LIBSOCK_A] [LIBSTASH_A]
  set mkdir
{
  [CC] [D_CFLAGS] -o [target]
       [fromto [srcdir]/%0%.c [objdir]/%0%.o_a [SUCK_SRCS]]
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
