/****************************************************************************
 *
 * <Copyright = "jasone">
 * <License>
 *
 ****************************************************************************
 *
 * Version: <Version>
 *
 * cook file for blow.
 *
 ****************************************************************************/

/* Include generated dependency files. */
#include-cooked [glob [objdir]/bin/blow/src/"*.d_*"]

/*
 * File lists.
 */

/* Include files that _are_ installed. */
{
  local incs = ;
  incs = [fromto % [srcdir]/bin/blow/include/% [incs]];

  INCS = [INCS] [incs];
}

/* Include files that are _not_ installed. */
{
  local priv_incs = ;
  priv_incs = [fromto % [srcdir]/bin/blow/include/% [priv_incs]];
  priv_incs = [priv_incs] [objdir]/bin/blow/include/blow_defs.h;

  PRIV_INCS = [PRIV_INCS] [priv_incs];
}

/* Source files. */
BLOW_SRCS = blow.c;
BLOW_SRCS = [fromto % [srcdir]/bin/blow/src/% [BLOW_SRCS]];
SRCS = [SRCS] [BLOW_SRCS];

/* Simple tests.  Output is diff'ed with the expected (.exp file) output. */
{
  local ctests = ;
  ctests = [fromto % [srcdir]/bin/blow/test/% [ctests]];

  CTESTS = [CTESTS] [ctests];
}

/* Perl scripts that drive the BTESTS. */
{
  local ptests = ;
  ptests = [fromto % [srcdir]/bin/blow/test/% [ptests]];

  PTESTS = [PTESTS] [ptests];
}

/* Backend test programs that are driven by the PTESTS. */
{
  local btests = ;
  btests = [fromto % [srcdir]/bin/blow/test/% [btests]];

  BTESTS = [BTESTS] [btests];
}

/* Man pages. */
{
  local manpages_1 = ;
  manpages_1 = [fromto % [srcdir]/bin/blow/man/man1/% [manpages_1]];

  MANPAGES_1 = [MANPAGES_1] [manpages_1];
}

/* Binaries. */
BLOW = [objdir]/bin/blow/src/blow;
BLOW_D = [BLOW].dbg;
BLOW_A = [BLOW];
BLOW_P = [BLOW].prof;

D_BINS = [D_BINS] [BLOW_D];
A_BINS = [A_BINS] [BLOW_A];
P_BINS = [P_BINS] [BLOW_P];

/*
 * User cook'ables.
 */

blow : [BLOW_D] [BLOW_A] [BLOW_P];

/*
 * Various flags.
 */

CPPFLAGS = [CPPFLAGS] -I[srcdir]/bin/blow/include
                      -I[objdir]/bin/blow/include;
D_CPPFLAGS = [D_CPPFLAGS] -D_BLOW_DBG;

/*
 * Dependencies.
 */

[BLOW_D] : [fromto [srcdir]/%0%.c [objdir]/%0%.o_d [BLOW_SRCS]]
           [LIBSOCK_D] [LIBSTASH_D]
  set mkdir
{
  [CC] [D_CFLAGS] -o [target]
       [fromto [srcdir]/%0%.c [objdir]/%0%.o_d [BLOW_SRCS]]
       [LDFLAGS] -lsock_d -lstash_d [EXTRALIBS];
}

[BLOW_A] : [fromto [srcdir]/%0%.c [objdir]/%0%.o_a [BLOW_SRCS]]
           [LIBSOCK_A] [LIBSTASH_A]
  set mkdir
{
  [CC] [D_CFLAGS] -o [target]
       [fromto [srcdir]/%0%.c [objdir]/%0%.o_a [BLOW_SRCS]]
       [LDFLAGS] -lsock -lstash [EXTRALIBS];
}

[BLOW_P] : [fromto [srcdir]/%0%.c [objdir]/%0%.o_p [BLOW_SRCS]]
           [LIBSOCK_P] [LIBSTASH_P]
  set mkdir
{
  [CC] [D_CFLAGS] -o [target]
       [fromto [srcdir]/%0%.c [objdir]/%0%.o_p [BLOW_SRCS]]
       [LDFLAGS] -lsock_p -lstash_p [EXTRALIBS];
}
