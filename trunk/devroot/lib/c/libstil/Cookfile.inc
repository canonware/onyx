/******************************************************************************
 *
 * <Copyright = jasone>
 * <License>
 *
 ******************************************************************************
 *
 * Version: <Version>
 *
 * cook file for libstil.
 *
 ******************************************************************************/

/* Include generated dependency files. */
#include-cooked [glob [objdir]/lib/c/libstil/src/"*.d_*"]
#include-cooked [glob [objdir]/lib/c/libstil/bin/"*.d_*"]
#include-cooked [glob [objdir]/lib/c/libstil/test/"*.d_*"]

/*
 * File lists.
 */

/* Include files that _are_ installed. */
{
  local incs = libstil.h libstil_incs.h stil.h stila.h stiln.h stilo.h stils.h
               stilt.h systemdict.h errordict.h derror.h threaddict.h;
  incs = [fromto % [srcdir]/lib/c/libstil/include/libstil/% [incs]];
  incs = [incs] [objdir]/lib/c/libstil/include/libstil/libstil_defs.h;

  LIBSTIL_INCS = [incs];
  INCS = [INCS] [incs];
}

/* Include files that are _not_ installed. */
{
  local priv_incs = stilo_l.h stils_l.h stilt_l.h;
  priv_incs = [fromto % [srcdir]/lib/c/libstil/include/libstil/% [priv_incs]];

  PRIV_INCS = [PRIV_INCS] [priv_incs];
}

/* Source files. */
LIBSTIL_SRCS = stil.c stila.c stiln.c stilo.c stils.c stilt.c systemdict.c
               errordict.c derror.c threaddict.c;
LIBSTIL_SRCS = [fromto % [srcdir]/lib/c/libstil/src/% [LIBSTIL_SRCS]];
SRCS = [SRCS] [LIBSTIL_SRCS];

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

  LIBSTIL_MANPAGES_3 = [manpages_3];
  MANPAGES_3 = [MANPAGES_3] [manpages_3];
}

/* Libraries. */
LIBSTIL_REV = 0.0;

LIBSTIL_D = [objdir]/lib/c/libstil/lib/libstil_d.a;
LIBSTIL_A = [objdir]/lib/c/libstil/lib/libstil.a;
LIBSTIL_S = [objdir]/lib/c/libstil/lib/libstil.so.[LIBSTIL_REV];
LIBSTIL_P = [objdir]/lib/c/libstil/lib/libstil_p.a;

LDFLAGS = [LDFLAGS] -L[objdir]/lib/c/libstil/lib;
LIBS = [LIBS] -lstil;
D_LIBS = [D_LIBS] [LIBSTIL_D];
A_LIBS = [A_LIBS] [LIBSTIL_A];
P_LIBS = [P_LIBS] [LIBSTIL_P];
S_LIBS = [S_LIBS] [LIBSTIL_S];
S_LINKS = [S_LINKS] [objdir]/lib/c/libstil/lib/libstil.so;

/*
 * User cook'ables.
 */

libstil : [LIBSTIL_D] [LIBSTIL_A] [LIBSTIL_P]
#if [ENABLE_SHARED]
          [LIBSTIL_S]
#endif
;

libstil_install : libstil_install_d libstil_install_a libstil_install_p
#if [ENABLE_SHARED]
                   libstil_install_s
#endif
;

libstil_uninstall :
{
  [SHELL] -e;
data
#![SHELL]

echo rm -f [PREFIX]/lib/libstil*
rm -f [PREFIX]/lib/libstil*

echo rm -rf [PREFIX]/include/libstil
rm -rf [PREFIX]/include/libstil

for i in [LIBSTIL_MANPAGES_3]; do
	echo rm -f [PREFIX]/man/man3/`[BASENAME] $i`
	rm -f [PREFIX]/man/man3/`[BASENAME] $i`
done
dataend
}

libstil_clean :
{
}

libstil_distclean :
{
}

/*
 * Various flags.
 */

CPPFLAGS = [CPPFLAGS] -I[srcdir]/lib/c/libstil/include
                      -I[objdir]/lib/c/libstil/include/libstil;
D_CPPFLAGS = [D_CPPFLAGS] -D_LIBSTIL_DBG;

/*
 * Build rules.
 */

libstil_install_common :
{
  [SHELL] -e;
data
#![SHELL]

echo [INSTALL] -d [PREFIX]/include/libstil
[INSTALL] -d [PREFIX]/include/libstil

for i in [LIBSTIL_INCS]; do
	echo [INSTALL] -m 0644 $i [PREFIX]/include/libstil/
	[INSTALL] -m 0644 $i [PREFIX]/include/libstil/
done

echo [INSTALL] -d [PREFIX]/man/man3
[INSTALL] -d [PREFIX]/man/man3

for i in [LIBSTIL_MANPAGES_3]; do
	echo [INSTALL] -m 0644 $i [PREFIX]/man/man3/
	[INSTALL] -m 0644 $i [PREFIX]/man/man3/
done

echo [INSTALL] -d [PREFIX]/lib
[INSTALL] -d [PREFIX]/lib
dataend
}

libstil_install_d : [LIBSTIL_D] libstil_install_common
{
	[INSTALL] -m 0444 [LIBSTIL_D] [PREFIX]/lib;
}

libstil_install_a : [LIBSTIL_A] libstil_install_common
{
	[INSTALL] -m 0444 [LIBSTIL_A] [PREFIX]/lib;
}

libstil_install_p : [LIBSTIL_P] libstil_install_common
{
	[INSTALL] -m 0444 [LIBSTIL_P] [PREFIX]/lib;
}

libstil_install_s : [LIBSTIL_S] libstil_install_common
{
	[INSTALL] -m 0444 [LIBSTIL_S] [PREFIX]/lib;
	[LN] -sf libstil.so.[LIBSTIL_REV] [PREFIX]/lib/libstil.so;
}

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
  [LN] -sf libstil.so.[LIBSTIL_REV] [objdir]/lib/c/libstil/lib/libstil.so;
}
