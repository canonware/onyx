/******************************************************************************
 *
 * <Copyright = jasone>
 * <License>
 *
 ******************************************************************************
 *
 * Version: <Version>
 *
 * cook file for libedit.  libedit is only built as a static library for the
 * benefit of onyx command line editing, so it is not installed.
 *
 ******************************************************************************/

/* Include generated dependency files. */
#include-cooked [glob [objdir]/lib/libedit/src/"*.d_*"]
#include-cooked [glob [objdir]/lib/libedit/test/"*.d_*"]

/*
 * File lists.
 */

/* Include files that _are_ installed. */
{
  local incs = tty.h vi.h libedit_term.h parse.h libedit_key.h chared.h hist.h
               histedit.h refresh.h search.h map.h libedit.h prompt.h
               libedit_incs.h emacs.h common.h fcns.h help.h tokenizer.h;
  incs = [fromto % [srcdir]/lib/libedit/include/libedit/% [incs]];
  incs = [incs] [objdir]/lib/libedit/include/libedit/libedit_defs.h;

  LIBEDIT_INCS = [incs];
  INCS = [INCS] [incs];
}

/* Source files. */
LIBEDIT_SRCS = chared.c common.c edit.c emacs.c hist.c history.c key.c map.c
             parse.c prompt.c read.c refresh.c search.c term.c
             tokenizer.c tty.c vi.c fcns.c help.c;
LIBEDIT_SRCS = [fromto % [srcdir]/lib/libedit/src/% [LIBEDIT_SRCS]];
SRCS = [SRCS] [LIBEDIT_SRCS];

/* Libraries. */
LIBEDIT_A = [objdir]/lib/libedit/lib/libedit.a;

LDFLAGS = [LDFLAGS] -L[objdir]/lib/libedit/lib;
A_LIBS = [A_LIBS] [LIBEDIT_A];

/*
 * User cook'ables.
 */

libedit : [LIBEDIT_A];

libedit_install :
{
}

libedit_uninstall :
{
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

CPPFLAGS = [CPPFLAGS] -I[srcdir]/lib/libedit/include
                      -I[objdir]/lib/libedit/include/libedit;

/*
 * Build rules.
 */

/*
 * Dependencies.
 */

[LIBEDIT_A] : [fromto [srcdir]/%0%.c [objdir]/%0%.o_a [LIBEDIT_SRCS]]
  set mkdir
{
  [AR] cru [target] [younger];
  [RANLIB] [target];
}
