/****************************************************************************
 *
 * <Copyright = "jasone">
 * <License>
 *
 ****************************************************************************
 *
 * Version: <Version>
 *
 * cook file for stile.
 *
 ****************************************************************************/

/* Include generated dependency files. */
#include-cooked [glob [objdir]/bin/stile/src/"*.d_*"]

/*
 * File lists.
 */

/* Include files that _are_ installed. */
{
  local incs = ;
  incs = [fromto % [srcdir]/bin/stile/include/% [incs]];

  INCS = [INCS] [incs];
}

/* Include files that are _not_ installed. */
{
  local priv_incs = ;
  priv_incs = [fromto % [srcdir]/bin/stile/include/% [priv_incs]];
  priv_incs = [priv_incs] [objdir]/bin/stile/include/stile_defs.h;

  PRIV_INCS = [PRIV_INCS] [priv_incs];
}

/* Source files. */
STILE_SRCS = stile.c;
STILE_SRCS = [fromto % [srcdir]/bin/stile/src/% [STILE_SRCS]];
SRCS = [SRCS] [STILE_SRCS];

/* Simple tests.  Output is diff'ed with the expected (.exp file) output. */
{
  local ctests = ;
  ctests = [fromto % [srcdir]/bin/stile/test/% [ctests]];

  CTESTS = [CTESTS] [ctests];
}

/* Perl scripts that drive the BTESTS. */
{
  local ptests = ;
  ptests = [fromto % [srcdir]/bin/stile/test/% [ptests]];

  PTESTS = [PTESTS] [ptests];
}

/* Backend test programs that are driven by the PTESTS. */
{
  local btests = ;
  btests = [fromto % [srcdir]/bin/stile/test/% [btests]];

  BTESTS = [BTESTS] [btests];
}

/* Man pages. */
{
  local manpages_1 = ;
  manpages_1 = [fromto % [srcdir]/bin/stile/man/man1/% [manpages_1]];

  MANPAGES_1 = [MANPAGES_1] [manpages_1];
}

/* Binaries. */
STILE = [objdir]/bin/stile/src/stile;
STILE_D = [STILE].dbg;
STILE_A = [STILE];
STILE_P = [STILE].prof;

D_BINS = [D_BINS] [STILE_D];
A_BINS = [A_BINS] [STILE_A];
P_BINS = [P_BINS] [STILE_P];

/*
 * User cook'ables.
 */

stile : [STILE_D] [STILE_A] [STILE_P];

/*
 * Various flags.
 */

CPPFLAGS = [CPPFLAGS] -I[srcdir]/bin/stile/include
                      -I[objdir]/bin/stile/include;
D_CPPFLAGS = [D_CPPFLAGS] -D_STILE_DBG;

/*
 * Dependencies.
 */

[STILE_D] : [fromto [srcdir]/%0%.c [objdir]/%0%.o_d [STILE_SRCS]] [D_LIBS]
  set mkdir
{
  [CC] [D_CFLAGS] -o [target]
       [fromto [srcdir]/%0%.c [objdir]/%0%.o_d [STILE_SRCS]]
       [D_LIBS] [LDFLAGS] [D_LIBS] [EXTRALIBS];
}

[STILE_A] : [fromto [srcdir]/%0%.c [objdir]/%0%.o_a [STILE_SRCS]] [A_LIBS]
  set mkdir
{
  [CC] [D_CFLAGS] -o [target]
       [fromto [srcdir]/%0%.c [objdir]/%0%.o_a [STILE_SRCS]]
       [D_LIBS] [LDFLAGS] [A_LIBS] [EXTRALIBS];
}

[STILE_P] : [fromto [srcdir]/%0%.c [objdir]/%0%.o_p [STILE_SRCS]] [P_LIBS]
  set mkdir
{
  [CC] [D_CFLAGS] -o [target]
       [fromto [srcdir]/%0%.c [objdir]/%0%.o_p [STILE_SRCS]]
       [D_LIBS] [LDFLAGS] [P_LIBS] [EXTRALIBS];
}
