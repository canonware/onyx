/******************************************************************************
 *
 * <Copyright = jasone>
 * <License>
 *
 ******************************************************************************
 *
 * Version: <Version>
 *
 * cook file for stil.
 *
 ******************************************************************************/

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

  STIL_MANPAGES_1 = [manpages_1];
  MANPAGES_1 = [MANPAGES_1] [manpages_1];
}

/* Binaries. */
STIL = [objdir]/bin/stil/bin/stil;
STIL_D = [STIL].dbg;
STIL_A = [STIL];
STIL_P = [STIL].prof;

D_BINS = [D_BINS] [STIL_D];
A_BINS = [A_BINS] [STIL_A];
P_BINS = [P_BINS] [STIL_P];

/*
 * User cook'ables.
 */

stil_all : stil_d stil_a stil_p;
stil_d : [STIL_D];
stil_a : [STIL_A];
stil_p : [STIL_P];

stil_install : stil_install_d stil_install_a stil_install_p;

stil_uninstall :
{
  [SHELL] -e;
data
#![SHELL]

echo rm -f [PREFIX]/bin/stil.dbg
rm -f [PREFIX]/bin/stil.dbg

echo rm -f [PREFIX]/bin/stil
rm -f [PREFIX]/bin/stil

echo rm -f [PREFIX]/bin/stil.prof
rm -f [PREFIX]/bin/stil.prof

for i in [STIL_MANPAGES_1]; do
	echo rm -f [PREFIX]/man/man1/`[BASENAME] $i`
	rm -f [PREFIX]/man/man1/`[BASENAME] $i`
done
dataend
}

stil_clean :
{
}

stil_distclean :
{
}

/*
 * Build rules.
 */

stil_install_common :
{
  [SHELL] -e;
data
#![SHELL]

echo [INSTALL] -d [PREFIX]/man/man1
[INSTALL] -d [PREFIX]/man/man1

for i in [STIL_MANPAGES_1]; do
	echo [INSTALL] -m 0644 $i [PREFIX]/man/man1/
	[INSTALL] -m 0644 $i [PREFIX]/man/man1/
done

echo [INSTALL] -d [PREFIX]/bin
[INSTALL] -d [PREFIX]/bin
dataend
}

stil_install_d : [STIL_D] stil_install_common
{
	[INSTALL] -m 0755 [STIL_D] [PREFIX]/bin;
}

stil_install_a : [STIL_A] stil_install_common
#if [ENABLE_SHARED]
                 libstil_install_s libedit_install_s libstash_install_s
#endif
{
#if [ENABLE_SHARED]
  [CC] [A_CFLAGS] -o [PREFIX]/bin/stil
       [fromto [srcdir]/%0%.c [objdir]/%0%.o_a [STIL_SRCS]]
       -rpath [PREFIX]/lib
       [LDFLAGS] -lstil -ledit -lstash -ltermcap [EXTRALIBS];
  [CHMOD] 0755 [PREFIX]/bin/stil;
#else
	[INSTALL] -m 0755 [STIL_A] [PREFIX]/bin;
#endif
}

stil_install_p : [STIL_P] stil_install_common
{
	[INSTALL] -m 0755 [STIL_P] [PREFIX]/bin;
}

/*
 * Dependencies.
 */

[STIL_D] : [fromto [srcdir]/%0%.c [objdir]/%0%.o_d [STIL_SRCS]]
           [LIBSTIL_D] [LIBEDIT_D] [LIBSTASH_D]
  set mkdir
{
  [CC] [D_CFLAGS] -o [target]
       [fromto [srcdir]/%0%.c [objdir]/%0%.o_d [STIL_SRCS]]
       [LDFLAGS] -lstil_d -ledit_d -lstash_d -ltermcap [EXTRALIBS];
}

[STIL_A] : [fromto [srcdir]/%0%.c [objdir]/%0%.o_a [STIL_SRCS]]
#if [ENABLE_SHARED]
           [LIBSTIL_S] [LIBEDIT_S] [LIBSTASH_S]
#else
           [LIBSTIL_A] [LIBEDIT_A] [LIBSTASH_A]
#endif
  set mkdir
{
  [CC] [A_CFLAGS] -o [target]
       [fromto [srcdir]/%0%.c [objdir]/%0%.o_a [STIL_SRCS]]
#if [ENABLE_SHARED]
       -rpath [objdir]/lib/c/libstil/lib
       -rpath [objdir]/lib/c/libedit/lib
       -rpath [objdir]/lib/c/libstash/lib
#endif
       [LDFLAGS] -lstil -ledit -lstash -ltermcap [EXTRALIBS];
}

[STIL_P] : [fromto [srcdir]/%0%.c [objdir]/%0%.o_p [STIL_SRCS]]
           [LIBSTIL_P] [LIBEDIT_P] [LIBSTASH_P]
  set mkdir
{
  [CC] [P_CFLAGS] -o [target]
       [fromto [srcdir]/%0%.c [objdir]/%0%.o_p [STIL_SRCS]]
       [LDFLAGS] -lstil_p -ledit_p -lstash_p -ltermcap [EXTRALIBS];
}
