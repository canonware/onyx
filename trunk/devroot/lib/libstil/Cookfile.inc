/****************************************************************************
 *
 * <Copyright = "jasone">
 * <License>
 *
 ****************************************************************************
 *
 * Version: <Version>
 *
 * cook file for libstil.
 *
 ****************************************************************************/

/* Include generated dependency files. */
#include-cooked [glob [objdir]/lib/c/libstil/src/"*.d_*"]
#include-cooked [glob [objdir]/lib/c/libstil/bin/"*.d_*"]
#include-cooked [glob [objdir]/lib/c/libstil/test/"*.d_*"]

/*
 * File lists.
 */

/* Include files that _are_ installed. */
{
  local incs = libstil.h libstil_incs.h stil.h stilo.h stiloe.h stiloe_array.h
               stiloe_condition.h stiloe_dict.h stiloe_hook.h stiloe_lock.h
               stiloe_mstate.h stiloe_number.h stiloe_packedarray.h
               stiloe_string.h stils.h stilt.h;
  incs = [fromto % [srcdir]/lib/c/libstil/include/libstil/% [incs]];
  incs = [incs] [objdir]/lib/c/libstil/include/libstil/libstil_defs.h;

  INCS = [INCS] [incs];
}

/* Include files that are _not_ installed. */
{
  local priv_incs = ;
  priv_incs = [fromto % [srcdir]/lib/c/libstil/include/libstil/% [priv_incs]];

  PRIV_INCS = [PRIV_INCS] [priv_incs];
}

/* Source files. */
LIBSTIL_SRCS = stil.c stilo.c stiloe.c stiloe_array.c stiloe_condition.c
               stiloe_dict.c stiloe_hook.c stiloe_lock.c stiloe_mstate.c
               stiloe_number.c stiloe_packedarray.c stiloe_string.c stils.c
               stilt.c;
LIBSTIL_SRCS = [fromto % [srcdir]/lib/c/libstil/src/% [LIBSTIL_SRCS]];
SRCS = [SRCS] [LIBSTIL_SRCS];

/* Executables. */
{
  local bins = ;
  bins = [fromto % [srcdir]/lib/c/libstil/bin/% [bins]];

  BINS = [BINS] [bins];
}

/* Simple tests.  Output is diff'ed with the expected (.exp file) output. */
{
  local ctests = ;
  ctests = [fromto % [srcdir]/lib/c/libstil/test/% [ctests]];

  CTESTS = [CTESTS] [ctests];
}

/* Perl scripts that drive the BTESTS. */
{
  local ptests = ;
  ptests = [fromto % [srcdir]/lib/c/libstil/test/% [ptests]];

  PTESTS = [PTESTS] [ptests];
}

/* Backend test programs that are driven by the PTESTS. */
{
  local btests = ;
  btests = [fromto % [srcdir]/lib/c/libstil/test/% [btests]];

  BTESTS = [BTESTS] [btests];
}

/* Man pages. */
{
  local manpages_3 = ;
  manpages_3 = [fromto % [srcdir]/lib/c/libstil/man/man3/% [manpages_3]];

  MANPAGES_3 = [MANPAGES_3] [manpages_3];
}

/* Libraries. */
LIBSTIL_REV = 0.0;

LIBSTIL_D = [objdir]/lib/c/libstil/lib/libstil_d.a;
LIBSTIL_A = [objdir]/lib/c/libstil/lib/libstil.a;
LIBSTIL_S = [objdir]/lib/c/libstil/lib/libstil.so.[LIBSTIL_REV];
LIBSTIL_P = [objdir]/lib/c/libstil/lib/libstil_p.a;

LDFLAGS = [LDFLAGS] -L[objdir]/lib/c/libstil/lib;
LIBS = [LIBS] -lsock;
D_LIBS = [D_LIBS] [LIBSTIL_D];
A_LIBS = [A_LIBS] [LIBSTIL_A];
P_LIBS = [P_LIBS] [LIBSTIL_P];
S_LIBS = [S_LIBS] [LIBSTIL_S];

/*
 * User cook'ables.
 */

libstil : [LIBSTIL_D] [LIBSTIL_A] [LIBSTIL_P] [LIBSTIL_S];

/*
 * Various flags.
 */

CPPFLAGS = [CPPFLAGS] -I[srcdir]/lib/c/libstil/include
                      -I[objdir]/lib/c/libstil/include/libstil;
D_CPPFLAGS = [D_CPPFLAGS] -D_LIBSTIL_DBG;

/*
 * Dependencies.
 */

[LIBSTIL_D] : [fromto [srcdir]/%0%.c [objdir]/%0%.o_d [LIBSTIL_SRCS]]
  set mkdir
{
  [AR] cru [target] [younger];
  [RANLIB] [target];
}

[LIBSTIL_A] : [fromto [srcdir]/%0%.c [objdir]/%0%.o_a [LIBSTIL_SRCS]]
  set mkdir
{
  [AR] cru [target] [younger];
  [RANLIB] [target];
}

[LIBSTIL_P] : [fromto [srcdir]/%0%.c [objdir]/%0%.o_p [LIBSTIL_SRCS]]
  set mkdir
{
  [AR] cru [target] [younger];
  [RANLIB] [target];
}

[LIBSTIL_S] : [fromto [srcdir]/%0%.c [objdir]/%0%.o_s [LIBSTIL_SRCS]]
  set mkdir
{
  [CC] -shared -o [target] [need];
}
