/******************************************************************************
 *
 * <Copyright = jasone>
 * <License>
 *
 ******************************************************************************
 *
 * Version: <Version>
 *
 * cook file for libedit.
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
               refresh.h search.h map.h sig.h libedit.h prompt.h libedit_incs.h
               emacs.h common.h fcns.h help.h tokenizer.h;
  incs = [fromto % [srcdir]/lib/c/libedit/include/libedit/% [incs]];
  incs = [incs] [objdir]/lib/c/libedit/include/libedit/libedit_defs.h;

  LIBEDIT_INCS = [incs];
  INCS = [INCS] [incs];
}

/* Include files that are _not_ installed. */
{
  local priv_incs = ;
  priv_incs = [fromto % [srcdir]/lib/c/libedit/include/libedit/% [priv_incs]];

  PRIV_INCS = [PRIV_INCS] [priv_incs];
}

/* Source files. */
LIBEDIT_SRCS = chared.c common.c edit.c emacs.c hist.c history.c key.c map.c
             parse.c prompt.c read.c refresh.c search.c sig.c term.c
             tokenizer.c tty.c vi.c fcns.c help.c;
LIBEDIT_SRCS = [fromto % [srcdir]/lib/c/libedit/src/% [LIBEDIT_SRCS]];
SRCS = [SRCS] [LIBEDIT_SRCS];

/* Simple tests.  Output is diff'ed with the expected (.exp file) output. */
{
  local ctests = ;
  ctests = [fromto % [srcdir]/lib/c/libedit/test/% [ctests]];

  CTESTS = [CTESTS] [ctests];
}

/* Perl scripts that drive the BTESTS. */
{
  local ptests = ;
  ptests = [fromto % [srcdir]/lib/c/libedit/test/% [ptests]];

  PTESTS = [PTESTS] [ptests];
}

/* Backend test programs that are driven by the PTESTS. */
{
  local btests = ;
  btests = [fromto % [srcdir]/lib/c/libedit/test/% [btests]];

  BTESTS = [BTESTS] [btests];
}

/* Man pages. */
{
  local manpages_3 = /*editline.3*/;
  manpages_3 = [fromto % [srcdir]/lib/c/libedit/man/man3/% [manpages_3]];

  LIBEDIT_MANPAGES_3 = [manpages_3];
  MANPAGES_3 = [MANPAGES_3] [manpages_3];
}

/* Libraries. */
LIBEDIT_REV = 0.0;

LIBEDIT_D = [objdir]/lib/c/libedit/lib/libedit_d.a;
LIBEDIT_A = [objdir]/lib/c/libedit/lib/libedit.a;
LIBEDIT_S = [objdir]/lib/c/libedit/lib/libedit.so.[LIBEDIT_REV];
LIBEDIT_P = [objdir]/lib/c/libedit/lib/libedit_p.a;

LDFLAGS = [LDFLAGS] -L[objdir]/lib/c/libedit/lib;
LIBS = [LIBS] -ledit;
D_LIBS = [D_LIBS] [LIBEDIT_D];
A_LIBS = [A_LIBS] [LIBEDIT_A];
P_LIBS = [P_LIBS] [LIBEDIT_P];
S_LIBS = [S_LIBS] [LIBEDIT_S];
S_LINKS = [S_LINKS] [objdir]/lib/c/libedit/lib/libedit.so;

/*
 * User cook'ables.
 */

libedit : [LIBEDIT_D] [LIBEDIT_A] [LIBEDIT_P]
#if [ENABLE_SHARED]
          [LIBEDIT_S]
#endif
;

libedit_install : libedit_install_d libedit_install_a libedit_install_p
#if [ENABLE_SHARED]
                   libedit_install_s
#endif
;

libedit_uninstall :
{
  [SHELL] -e;
data
#![SHELL]

echo rm -f [PREFIX]/lib/libedit*
rm -f [PREFIX]/lib/libedit*

echo rm -rf [PREFIX]/include/libedit
rm -rf [PREFIX]/include/libedit

for i in [LIBEDIT_MANPAGES_3]; do
	echo rm -f [PREFIX]/man/man3/`[BASENAME] $i`
	rm -f [PREFIX]/man/man3/`[BASENAME] $i`
done
dataend
}

libedit_clean :
{
}

libedit_distclean :
{
}

/*
 * Various flags.
 */

CPPFLAGS = [CPPFLAGS] -I[srcdir]/lib/c/libedit/include
                      -I[objdir]/lib/c/libedit/include/libedit;
D_CPPFLAGS = [D_CPPFLAGS] -D_LIBEDIT_DBG;

/*
 * Build rules.
 */

libedit_install_common :
{
  [SHELL] -e;
data
#![SHELL]

echo [INSTALL] -d [PREFIX]/include/libedit
[INSTALL] -d [PREFIX]/include/libedit

for i in [LIBEDIT_INCS]; do
	echo [INSTALL] -m 0644 $i [PREFIX]/include/libedit/
	[INSTALL] -m 0644 $i [PREFIX]/include/libedit/
done

echo [INSTALL] -d [PREFIX]/man/man3
[INSTALL] -d [PREFIX]/man/man3

for i in [LIBEDIT_MANPAGES_3]; do
	echo [INSTALL] -m 0644 $i [PREFIX]/man/man3/
	[INSTALL] -m 0644 $i [PREFIX]/man/man3/
done

echo [INSTALL] -d [PREFIX]/lib
[INSTALL] -d [PREFIX]/lib
dataend
}

libedit_install_d : [LIBEDIT_D] libedit_install_common
{
	[INSTALL] -m 0444 [LIBEDIT_D] [PREFIX]/lib;
}

libedit_install_a : [LIBEDIT_A] libedit_install_common
{
	[INSTALL] -m 0444 [LIBEDIT_A] [PREFIX]/lib;
}

libedit_install_p : [LIBEDIT_P] libedit_install_common
{
	[INSTALL] -m 0444 [LIBEDIT_P] [PREFIX]/lib;
}

libedit_install_s : [LIBEDIT_S] libedit_install_common
{
	[INSTALL] -m 0444 [LIBEDIT_S] [PREFIX]/lib;
	[LN] -sf libedit.so.[LIBEDIT_REV] [PREFIX]/lib/libedit.so;
}

/*
 * Dependencies.
 */

[LIBEDIT_D] : [fromto [srcdir]/%0%.c [objdir]/%0%.o_d [LIBEDIT_SRCS]]
  set mkdir
{
  [AR] cru [target] [younger];
  [RANLIB] [target];
}

[LIBEDIT_A] : [fromto [srcdir]/%0%.c [objdir]/%0%.o_a [LIBEDIT_SRCS]]
  set mkdir
{
  [AR] cru [target] [younger];
  [RANLIB] [target];
}

[LIBEDIT_P] : [fromto [srcdir]/%0%.c [objdir]/%0%.o_p [LIBEDIT_SRCS]]
  set mkdir
{
  [AR] cru [target] [younger];
  [RANLIB] [target];
}

[LIBEDIT_S] : [fromto [srcdir]/%0%.c [objdir]/%0%.o_s [LIBEDIT_SRCS]]
  set mkdir
{
  [CC] -shared -o [target] [need];
  [LN] -sf libedit.so.[LIBEDIT_REV] [objdir]/lib/c/libedit/lib/libedit.so;
}
