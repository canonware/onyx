/******************************************************************************
 *
 * <Copyright = "jasone">
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
#include-cooked [glob [objdir]/lib/c/libficl/src/"*.d_*"]
#include-cooked [glob [objdir]/lib/c/libficl/bin/"*.d_*"]
#include-cooked [glob [objdir]/lib/c/libficl/test/"*.d_*"]

/*
 * File lists.
 */

/* Include files that _are_ installed. */
{
  local incs = libficl.h;
  incs = [fromto % [srcdir]/lib/c/libficl/include/libficl/% [incs]];
  incs = [incs] [objdir]/lib/c/libficl/include/libficl/libficl_defs.h;

  LIBFICL_INCS = [incs];
  INCS = [INCS] [incs];
}

/* Include files that are _not_ installed. */
{
  local priv_incs = math64.h sysdep.h;
  priv_incs = [fromto % [srcdir]/lib/c/libficl/include/libficl/% [priv_incs]];

  PRIV_INCS = [PRIV_INCS] [priv_incs];
}

/* Forth source files. */
{
  local softwords = softcore.fr jhlocal.fr marker.fr;
                    /* oo.fr classes.fr ficllocal.fr ifbrack.fr */
  softwords = [fromto % [srcdir]/lib/c/libficl/softwords/% [softwords]];

  LIBFICL_SOFTWORDS = [softwords];
}

/* Source files. */
LIBFICL_SRCS = dict.c ficl.c math64.c stack.c sysdep.c vm.c words.c;
LIBFICL_SRCS = [fromto % [srcdir]/lib/c/libficl/src/% [LIBFICL_SRCS]];
LIBFICL_SRCS = [LIBFICL_SRCS] [objdir]/lib/c/libficl/src/softcore.c;
SRCS = [SRCS] [LIBFICL_SRCS];

/* Simple tests.  Output is diff'ed with the expected (.exp file) output. */
{
  local ctests = ;
  ctests = [fromto % [srcdir]/lib/c/libficl/test/% [ctests]];

  CTESTS = [CTESTS] [ctests];
}

/* Perl scripts that drive the BTESTS. */
{
  local ptests = ;
  ptests = [fromto % [srcdir]/lib/c/libficl/test/% [ptests]];

  PTESTS = [PTESTS] [ptests];
}

/* Backend test programs that are driven by the PTESTS. */
{
  local btests = ;
  btests = [fromto % [srcdir]/lib/c/libficl/test/% [btests]];

  BTESTS = [BTESTS] [btests];
}

/* Man pages. */
{
  local manpages_3 = ;
  manpages_3 = [fromto % [srcdir]/lib/c/libficl/man/man3/% [manpages_3]];

  LIBFICL_MANPAGES_3 = [manpages_3];
  MANPAGES_3 = [MANPAGES_3] [manpages_3];
}

/* Libraries. */
LIBFICL_REV = 2.04;

LIBFICL_D = [objdir]/lib/c/libficl/lib/libficl_d.a;
LIBFICL_A = [objdir]/lib/c/libficl/lib/libficl.a;
LIBFICL_S = [objdir]/lib/c/libficl/lib/libficl.so.[LIBFICL_REV];
LIBFICL_P = [objdir]/lib/c/libficl/lib/libficl_p.a;

LDFLAGS = [LDFLAGS] -L[objdir]/lib/c/libficl/lib;
LIBS = [LIBS] -lficl;
D_LIBS = [D_LIBS] [LIBFICL_D];
A_LIBS = [A_LIBS] [LIBFICL_A];
P_LIBS = [P_LIBS] [LIBFICL_P];
S_LIBS = [S_LIBS] [LIBFICL_S];
S_LINKS = [S_LINKS] [objdir]/lib/c/libficl/lib/libficl.so;

/*
 * User cook'ables.
 */

libficl : [LIBFICL_D] [LIBFICL_A] [LIBFICL_P]
#if [ENABLE_SHARED]
          [LIBFICL_S]
#endif
;

libficl_install : libficl_install_d libficl_install_a libficl_install_p
#if [ENABLE_SHARED]
                   libficl_install_s
#endif
;

libficl_uninstall :
{
  [SHELL] -e;
data
#![SHELL]

echo rm -f [PREFIX]/lib/libficl*
rm -f [PREFIX]/lib/libficl*

echo rm -rf [PREFIX]/include/libficl
rm -rf [PREFIX]/include/libficl

for i in [LIBFICL_MANPAGES_3]; do
	echo rm -f [PREFIX]/man/man3/`[BASENAME] $i`
	rm -f [PREFIX]/man/man3/`[BASENAME] $i`
done
dataend
}

libficl_clean :
{
  rm -f [objdir]/lib/c/libficl/src/softcore.c;
}

libficl_distclean :
{
  rm -f [objdir]/lib/c/libficl/softwords/softcore.pl;
}

/*
 * Various flags.
 */

CPPFLAGS = [CPPFLAGS] -I[srcdir]/lib/c/libficl/include
                      -I[objdir]/lib/c/libficl/include/libficl;
D_CPPFLAGS = [D_CPPFLAGS] -D_LIBFICL_DBG;

/*
 * Build rules.
 */

libficl_install_common :
{
  [SHELL] -e;
data
#![SHELL]

echo [INSTALL] -d [PREFIX]/include/libficl
[INSTALL] -d [PREFIX]/include/libficl

for i in [LIBFICL_INCS]; do
	echo [INSTALL] -m 0644 $i [PREFIX]/include/libficl/
	[INSTALL] -m 0644 $i [PREFIX]/include/libficl/
done

echo [INSTALL] -d [PREFIX]/man/man3
[INSTALL] -d [PREFIX]/man/man3

for i in [LIBFICL_MANPAGES_3]; do
	echo [INSTALL] -m 0644 $i [PREFIX]/man/man3/
	[INSTALL] -m 0644 $i [PREFIX]/man/man3/
done

echo [INSTALL] -d [PREFIX]/lib
[INSTALL] -d [PREFIX]/lib
dataend
}

libficl_install_d : [LIBFICL_D] libficl_install_common
{
	[INSTALL] -m 0444 [LIBFICL_D] [PREFIX]/lib;
}

libficl_install_a : [LIBFICL_A] libficl_install_common
{
	[INSTALL] -m 0444 [LIBFICL_A] [PREFIX]/lib;
}

libficl_install_p : [LIBFICL_P] libficl_install_common
{
	[INSTALL] -m 0444 [LIBFICL_P] [PREFIX]/lib;
}

libficl_install_s : [LIBFICL_D] libficl_install_common
{
	[INSTALL] -m 0444 [LIBFICL_S] [PREFIX]/lib;
	[LN] -sf libficl.so.[LIBFICL_REV] [PREFIX]/lib/libficl.so;
}

/*
 * Dependencies.
 */

[LIBFICL_D] : [fromto [srcdir]/%0%.c [objdir]/%0%.o_d [LIBFICL_SRCS]]
  set mkdir
{
  [AR] cru [target] [younger];
  [RANLIB] [target];
}

[LIBFICL_A] : [fromto [srcdir]/%0%.c [objdir]/%0%.o_a [LIBFICL_SRCS]]
  set mkdir
{
  [AR] cru [target] [younger];
  [RANLIB] [target];
}

[LIBFICL_P] : [fromto [srcdir]/%0%.c [objdir]/%0%.o_p [LIBFICL_SRCS]]
  set mkdir
{
  [AR] cru [target] [younger];
  [RANLIB] [target];
}

[LIBFICL_S] : [fromto [srcdir]/%0%.c [objdir]/%0%.o_s [LIBFICL_SRCS]]
  set mkdir
{
  [CC] -shared -o [target] [need];
  [LN] -sf libficl.so.[LIBFICL_REV] [objdir]/lib/c/libficl/lib/libficl.so;
}

[objdir]/lib/c/libficl/src/softcore.c : [LIBFICL_SOFTWORDS]
  set mkdir
{
  [objdir]/lib/c/libficl/softwords/softcore.pl [LIBFICL_SOFTWORDS] > [target];
}
