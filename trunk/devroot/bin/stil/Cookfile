/****************************************************************************
 *
 * <Copyright = "jasone">
 * <License>
 *
 ****************************************************************************
 *
 * Version: <Version>
 *
 * cook file for stil.
 *
 ****************************************************************************/

/* Include generated dependency files. */
#include-cooked [glob [objdir]/bin/stil/src/"*.d_*"]

/*
 * File lists.
 */

/* Include files that _are_ installed. */
{
  local incs = ;
  incs = [fromto % [srcdir]/bin/stil/include/% [incs]];

  INCS = [INCS] [incs];
}

/* Include files that are _not_ installed. */
{
  local priv_incs = ;
  priv_incs = [fromto % [srcdir]/bin/stil/include/% [priv_incs]];
  priv_incs = [priv_incs] [objdir]/bin/stil/include/stil_defs.h;

  PRIV_INCS = [PRIV_INCS] [priv_incs];
}

/* Source files. */
STIL_SRCS = stil.c;
STIL_SRCS = [fromto % [srcdir]/bin/stil/src/% [STIL_SRCS]];
SRCS = [SRCS] [STIL_SRCS];

/* Simple tests.  Output is diff'ed with the expected (.exp file) output. */
{
  local ctests = ;
  ctests = [fromto % [srcdir]/bin/stil/test/% [ctests]];

  CTESTS = [CTESTS] [ctests];
}

/* Perl scripts that drive the BTESTS. */
{
  local ptests = ;
  ptests = [fromto % [srcdir]/bin/stil/test/% [ptests]];

  PTESTS = [PTESTS] [ptests];
}

/* Backend test programs that are driven by the PTESTS. */
{
  local btests = ;
  btests = [fromto % [srcdir]/bin/stil/test/% [btests]];

  BTESTS = [BTESTS] [btests];
}

/* Man pages. */
{
  local manpages_1 = ;
  manpages_1 = [fromto % [srcdir]/bin/stil/man/man1/% [manpages_1]];

  MANPAGES_1 = [MANPAGES_1] [manpages_1];
}

/* Binaries. */
STIL = [objdir]/bin/stil/src/stil;
STIL_D = [STIL].dbg;
STIL_A = [STIL];
STIL_P = [STIL].prof;

D_BINS = [D_BINS] [STIL_D];
A_BINS = [A_BINS] [STIL_A];
P_BINS = [P_BINS] [STIL_P];

/*
 * User cook'ables.
 */

stil : [STIL_D] [STIL_A] [STIL_P];

/*
 * Various flags.
 */

CPPFLAGS = [CPPFLAGS] -I[srcdir]/bin/stil/include
                      -I[objdir]/bin/stil/include;
D_CPPFLAGS = [D_CPPFLAGS] -D_STIL_DBG;

/*
 * Dependencies.
 */

[STIL_D] : [fromto [srcdir]/%0%.c [objdir]/%0%.o_d [STIL_SRCS]] [D_LIBS]
  set mkdir
{
  [CC] [D_CFLAGS] -o [target]
       [fromto [srcdir]/%0%.c [objdir]/%0%.o_d [STIL_SRCS]]
       [D_LIBS] [LDFLAGS] [D_LIBS] [EXTRALIBS];
}

[STIL_A] : [fromto [srcdir]/%0%.c [objdir]/%0%.o_a [STIL_SRCS]] [A_LIBS]
  set mkdir
{
  [CC] [D_CFLAGS] -o [target]
       [fromto [srcdir]/%0%.c [objdir]/%0%.o_a [STIL_SRCS]]
       [D_LIBS] [LDFLAGS] [A_LIBS] [EXTRALIBS];
}

[STIL_P] : [fromto [srcdir]/%0%.c [objdir]/%0%.o_p [STIL_SRCS]] [P_LIBS]
  set mkdir
{
  [CC] [D_CFLAGS] -o [target]
       [fromto [srcdir]/%0%.c [objdir]/%0%.o_p [STIL_SRCS]]
       [D_LIBS] [LDFLAGS] [P_LIBS] [EXTRALIBS];
}
