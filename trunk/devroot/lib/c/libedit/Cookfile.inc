/******************************************************************************
 *
 * <Copyright = jasone>
 * <License>
 *
 ******************************************************************************
 *
 * Version: <Version>
 *
 * cook file for libedit.  Although the mechanisms are in place to install
 * libedit, key sections of this file are commented out, so that only the
 * shared library gets installed, and that only happens if [ENABLE_SHARED].
 *
 ******************************************************************************/

/* Include generated dependency files. */
#include-cooked [glob [objdir]/lib/c/libedit/src/"*.d_*"]
#include-cooked [glob [objdir]/lib/c/libedit/test/"*.d_*"]

/*
 * File lists.
 */

/* Include files that _are_ installed. */
{
  local incs = tty.h vi.h libedit_term.h parse.h libedit_key.h chared.h hist.h
               histedit.h refresh.h search.h map.h libedit.h prompt.h
               libedit_incs.h emacs.h common.h fcns.h help.h tokenizer.h;
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
             parse.c prompt.c read.c refresh.c search.c term.c
             tokenizer.c tty.c vi.c fcns.c help.c;
LIBEDIT_SRCS = [fromto % [srcdir]/lib/c/libedit/src/% [LIBEDIT_SRCS]];
SRCS = [SRCS] [LIBEDIT_SRCS];

/* Simple C tests.  Output is diff'ed with the expected (.exp file) output. */
{
  local ctests = ;
  ctests = [fromto % [srcdir]/lib/c/libedit/test/% [ctests]];

  CTESTS = [CTESTS] [ctests];
}

/*
 * Simple onyx tests.  Output is diff'ed with the expected (.exp file)
 * output.
 */
{
  local otests = ;
  otests = [fromto % [srcdir]/lib/c/libedit/test/% [otests]];

  OTESTS = [OTESTS] [otests];
}

/* Perl scripts that drive other tests and use the sequence mode of verify. */
{
  local ptests = ;
  ptests = [fromto % [srcdir]/lib/c/libedit/test/% [ptests]];

  PTESTS = [PTESTS] [ptests];
}

/* C programs that are driven by tests. */
{
  local cbtests = ;
  cbtests = [fromto % [srcdir]/lib/c/libedit/test/% [cbtests]];

  CBTESTS = [CBTESTS] [cbtests];
}

/* onyx programs that are driven by tests. */
{
  local obtests = ;
  obtests = [fromto % [srcdir]/lib/c/libedit/test/% [obtests]];

  OBTESTS = [OBTESTS] [obtests];
}

/* LaTeX source files. */
{
  local tex_srcs = ;
  tex_srcs = [fromto % [srcdir]/lib/c/libedit/doc/latex/% [tex_srcs]];

  TEX_SRCS = [TEX_SRCS] [tex_srcs];
}

/* xfig source files. */
{
  local fig_srcs = ;
  fig_srcs = [fromto % [srcdir]/lib/c/libedit/doc/latex/% [fig_srcs]];

  FIG_SRCS = [FIG_SRCS] [fig_srcs];
}

/* Libraries. */
LIBEDIT_REV = 1;

LIBEDIT_A = [objdir]/lib/c/libedit/lib/libedit.a;
LIBEDIT_S = [objdir]/lib/c/libedit/lib/libedit.so.[LIBEDIT_REV];

LDFLAGS = [LDFLAGS] -L[objdir]/lib/c/libedit/lib;
/* LIBS = [LIBS] -ledit; */
A_LIBS = [A_LIBS] [LIBEDIT_A];
/* S_LIBS = [S_LIBS] [LIBEDIT_S]; */
/* S_LINKS = [S_LINKS] [objdir]/lib/c/libedit/lib/libedit.so; */

/*
 * User cook'ables.
 */

libedit : [LIBEDIT_A]  [LIBEDIT_S];

libedit_install : /* libedit_install_a libedit_install_s */;

libedit_uninstall :
{
  [SHELL] -e;
data
#![SHELL]

echo rm -f [PREFIX]/lib/libedit*
rm -f [PREFIX]/lib/libedit*

#echo rm -rf [PREFIX]/include/libedit
#rm -rf [PREFIX]/include/libedit
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

#echo [INSTALL] -d [PREFIX]/lib
#[INSTALL] -d [PREFIX]/lib
dataend
}

libedit_install_a : [LIBEDIT_A] libedit_install_common
{
  [INSTALL] -m 0444 [LIBEDIT_A] [PREFIX]/lib;
}

libedit_install_s : [LIBEDIT_S] libedit_install_common
{
  [INSTALL] -m 0444 [LIBEDIT_S] [PREFIX]/lib;

  rm -f [PREFIX]/lib/libedit.so;
  [LN] -s libedit.so.[LIBEDIT_REV] [PREFIX]/lib/libedit.so;
}

/*
 * Dependencies.
 */

[LIBEDIT_A] : [fromto [srcdir]/%0%.c [objdir]/%0%.o_a [LIBEDIT_SRCS]]
  set mkdir
{
  [AR] cru [target] [younger];
  [RANLIB] [target];
}

[LIBEDIT_S] : [fromto [srcdir]/%0%.c [objdir]/%0%.o_s [LIBEDIT_SRCS]]
  set mkdir
{
  [CC] -shared -soname libedit.so.[LIBEDIT_REV] -o [target] [need];

  rm -f [objdir]/lib/c/libedit/lib/libedit.so;
  [LN] -s libedit.so.[LIBEDIT_REV] [objdir]/lib/c/libedit/lib/libedit.so;
}
