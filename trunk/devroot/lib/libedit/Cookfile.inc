/******************************************************************************
 *
 * <Copyright = "jasone">
 * <License>
 *
 ******************************************************************************
 *
 * Version: <Version>
 *
 * cook file for libcl.
 *
 ******************************************************************************/

/* Include generated dependency files. */
#include-cooked [glob [objdir]/lib/c/libcl/src/"*.d_*"]
#include-cooked [glob [objdir]/lib/c/libcl/bin/"*.d_*"]
#include-cooked [glob [objdir]/lib/c/libcl/test/"*.d_*"]

/*
 * File lists.
 */

/* Include files that _are_ installed. */
{
  local incs = tty.h vi.h term.h parse.h key.h chared.h hist.h histedit.h
               refresh.h search.h map.h sig.h libcl.h prompt.h libcl_incs.h
               emacs.h common.h fcns.h help.h tokenizer.h;
  incs = [fromto % [srcdir]/lib/c/libcl/include/libcl/% [incs]];
  incs = [incs] [objdir]/lib/c/libcl/include/libcl/libcl_defs.h;

  LIBCL_INCS = [incs];
  INCS = [INCS] [incs];
}

/* Include files that are _not_ installed. */
{
  local priv_incs = ;
  priv_incs = [fromto % [srcdir]/lib/c/libcl/include/libcl/% [priv_incs]];

  PRIV_INCS = [PRIV_INCS] [priv_incs];
}

/* Source files. */
LIBCL_SRCS = chared.c common.c cl.c emacs.c hist.c history.c key.c map.c
             parse.c prompt.c read.c refresh.c search.c sig.c term.c
             tokenizer.c tty.c vi.c fcns.c help.c;
LIBCL_SRCS = [fromto % [srcdir]/lib/c/libcl/src/% [LIBCL_SRCS]];
SRCS = [SRCS] [LIBCL_SRCS];

/* Simple tests.  Output is diff'ed with the expected (.exp file) output. */
{
  local ctests = ;
  ctests = [fromto % [srcdir]/lib/c/libcl/test/% [ctests]];

  CTESTS = [CTESTS] [ctests];
}

/* Perl scripts that drive the BTESTS. */
{
  local ptests = ;
  ptests = [fromto % [srcdir]/lib/c/libcl/test/% [ptests]];

  PTESTS = [PTESTS] [ptests];
}

/* Backend test programs that are driven by the PTESTS. */
{
  local btests = ;
  btests = [fromto % [srcdir]/lib/c/libcl/test/% [btests]];

  BTESTS = [BTESTS] [btests];
}

/* Man pages. */
{
  local manpages_3 = cl.3;
  manpages_3 = [fromto % [srcdir]/lib/c/libcl/man/man3/% [manpages_3]];

  LIBCL_MANPAGES_3 = [manpages_3];
  MANPAGES_3 = [MANPAGES_3] [manpages_3];
}

/* Libraries. */
LIBCL_REV = 0.0;

LIBCL_D = [objdir]/lib/c/libcl/lib/libcl_d.a;
LIBCL_A = [objdir]/lib/c/libcl/lib/libcl.a;
LIBCL_S = [objdir]/lib/c/libcl/lib/libcl.so.[LIBCL_REV];
LIBCL_P = [objdir]/lib/c/libcl/lib/libcl_p.a;

LDFLAGS = [LDFLAGS] -L[objdir]/lib/c/libcl/lib;
LIBS = [LIBS] -lcl;
D_LIBS = [D_LIBS] [LIBCL_D];
A_LIBS = [A_LIBS] [LIBCL_A];
P_LIBS = [P_LIBS] [LIBCL_P];
S_LIBS = [S_LIBS] [LIBCL_S];
S_LINKS = [S_LINKS] [objdir]/lib/c/libcl/lib/libcl.so;

/*
 * User cook'ables.
 */

libcl : [LIBCL_D] [LIBCL_A] [LIBCL_P]
#if [ENABLE_SHARED]
          [LIBCL_S]
#endif
;

libcl_install : libcl_install_d libcl_install_a libcl_install_p
#if [ENABLE_SHARED]
                   libcl_install_s
#endif
;

libcl_uninstall :
{
  [SHELL] -e;
data
#![SHELL]

echo rm -f [PREFIX]/lib/libcl*
rm -f [PREFIX]/lib/libcl*

echo rm -rf [PREFIX]/include/libcl
rm -rf [PREFIX]/include/libcl

for i in [LIBCL_MANPAGES_3]; do
	echo rm -f [PREFIX]/man/man3/`[BASENAME] $i`
	rm -f [PREFIX]/man/man3/`[BASENAME] $i`
done
dataend
}

/*
 * Various flags.
 */

CPPFLAGS = [CPPFLAGS] -I[srcdir]/lib/c/libcl/include
                      -I[objdir]/lib/c/libcl/include/libcl;
D_CPPFLAGS = [D_CPPFLAGS] -D_LIBCL_DBG;

/*
 * Build rules.
 */

libcl_install_common :
{
  [SHELL] -e;
data
#![SHELL]

echo [INSTALL] -d [PREFIX]/include/libcl
[INSTALL] -d [PREFIX]/include/libcl

for i in [LIBCL_INCS]; do
	echo [INSTALL] -m 0644 $i [PREFIX]/include/libcl/
	[INSTALL] -m 0644 $i [PREFIX]/include/libcl/
done

echo [INSTALL] -d [PREFIX]/man/man3
[INSTALL] -d [PREFIX]/man/man3

for i in [LIBCL_MANPAGES_3]; do
	echo [INSTALL] -m 0644 $i [PREFIX]/man/man3/
	[INSTALL] -m 0644 $i [PREFIX]/man/man3/
done

echo [INSTALL] -d [PREFIX]/lib
[INSTALL] -d [PREFIX]/lib
dataend
}

libcl_install_d : [LIBCL_D] libcl_install_common
{
	[INSTALL] -m 0444 [LIBCL_D] [PREFIX]/lib;
}

libcl_install_a : [LIBCL_A] libcl_install_common
{
	[INSTALL] -m 0444 [LIBCL_A] [PREFIX]/lib;
}

libcl_install_p : [LIBCL_P] libcl_install_common
{
	[INSTALL] -m 0444 [LIBCL_P] [PREFIX]/lib;
}

libcl_install_s : [LIBCL_D] libcl_install_common
{
	[INSTALL] -m 0444 [LIBCL_S] [PREFIX]/lib;
	[LN] -sf libcl.so.[LIBCL_REV] [PREFIX]/lib/libcl.so;
}

/*
 * Dependencies.
 */

[LIBCL_D] : [fromto [srcdir]/%0%.c [objdir]/%0%.o_d [LIBCL_SRCS]]
  set mkdir
{
  [AR] cru [target] [younger];
  [RANLIB] [target];
}

[LIBCL_A] : [fromto [srcdir]/%0%.c [objdir]/%0%.o_a [LIBCL_SRCS]]
  set mkdir
{
  [AR] cru [target] [younger];
  [RANLIB] [target];
}

[LIBCL_P] : [fromto [srcdir]/%0%.c [objdir]/%0%.o_p [LIBCL_SRCS]]
  set mkdir
{
  [AR] cru [target] [younger];
  [RANLIB] [target];
}

[LIBCL_S] : [fromto [srcdir]/%0%.c [objdir]/%0%.o_s [LIBCL_SRCS]]
  set mkdir
{
  [CC] -shared -o [target] [need];
  [LN] -sf libcl.so.[LIBCL_REV] [objdir]/lib/c/libcl/lib/libcl.so;
}
