/******************************************************************************
 *
 * <Copyright = jasone>
 * <License>
 *
 ******************************************************************************
 *
 * Version: <Version>
 *
 * cook file for xvx.
 *
 ******************************************************************************/

/* Include generated dependency files. */
#include-cooked [glob [objdir]/bin/xvx/src/"*.d_*"]

/*
 * File lists.
 */

/* Include files that _are_ installed. */
{
  local incs = ;
  incs = [fromto % [srcdir]/bin/xvx/include/% [incs]];

  INCS = [INCS] [incs];
}

/* Include files that are _not_ installed. */
{
  local priv_incs = board.h game.h move.h xvx.h;
  priv_incs = [fromto % [srcdir]/bin/xvx/include/% [priv_incs]];
  priv_incs = [priv_incs] [objdir]/bin/xvx/include/xvx_defs.h;

  PRIV_INCS = [PRIV_INCS] [priv_incs];
}

/* Source files. */
XVX_SRCS = board.c game.c move.c xvx.c;
XVX_SRCS = [fromto % [srcdir]/bin/xvx/src/% [XVX_SRCS]];
SRCS = [SRCS] [XVX_SRCS];

/* Simple tests.  Output is diff'ed with the expected (.exp file) output. */
{
  local ctests = ;
  ctests = [fromto % [srcdir]/bin/xvx/test/% [ctests]];

  CTESTS = [CTESTS] [ctests];
}

/* Perl scripts that drive the BTESTS. */
{
  local ptests = ;
  ptests = [fromto % [srcdir]/bin/xvx/test/% [ptests]];

  PTESTS = [PTESTS] [ptests];
}

/* Backend test programs that are driven by the PTESTS. */
{
  local btests = ;
  btests = [fromto % [srcdir]/bin/xvx/test/% [btests]];

  BTESTS = [BTESTS] [btests];
}

/* Man pages. */
{
  local manpages_1 = ;
  manpages_1 = [fromto % [srcdir]/bin/xvx/man/man1/% [manpages_1]];

  XVX_MANPAGES_1 = [manpages_1];
  MANPAGES_1 = [MANPAGES_1] [manpages_1];
}

/* Binaries. */
XVX = [objdir]/bin/xvx/bin/xvx;
XVX_D = [XVX].dbg;
XVX_A = [XVX];
XVX_P = [XVX].prof;

D_BINS = [D_BINS] [XVX_D];
A_BINS = [A_BINS] [XVX_A];
P_BINS = [P_BINS] [XVX_P];

/*
 * User cook'ables.
 */

xvx_all : xvx_d xvx_a xvx_p;
xvx_d : [XVX_D];
xvx_a : [XVX_A];
xvx_p : [XVX_P];

xvx_install : xvx_install_d xvx_install_a xvx_install_p;

xvx_uninstall :
{
  [SHELL] -e;
data
#![SHELL]

echo rm -f [PREFIX]/bin/xvx.dbg
rm -f [PREFIX]/bin/xvx.dbg

echo rm -f [PREFIX]/bin/xvx
rm -f [PREFIX]/bin/xvx

echo rm -f [PREFIX]/bin/xvx.prof
rm -f [PREFIX]/bin/xvx.prof

for i in [XVX_MANPAGES_1]; do
	echo rm -f [PREFIX]/man/man1/`[BASENAME] $i`
	rm -f [PREFIX]/man/man1/`[BASENAME] $i`
done
dataend
}

xvx_clean :
{
}

xvx_distclean :
{
}

/*
 * Build rules.
 */

xvx_install_common :
{
  [SHELL] -e;
data
#![SHELL]

echo [INSTALL] -d [PREFIX]/man/man1
[INSTALL] -d [PREFIX]/man/man1

for i in [XVX_MANPAGES_1]; do
	echo [INSTALL] -m 0644 $i [PREFIX]/man/man1/
	[INSTALL] -m 0644 $i [PREFIX]/man/man1/
done

echo [INSTALL] -d [PREFIX]/bin
[INSTALL] -d [PREFIX]/bin
dataend
}

xvx_install_d : [XVX_D] xvx_install_common
{
	[INSTALL] -m 0755 [XVX_D] [PREFIX]/bin;
}

xvx_install_a : [XVX_A] xvx_install_common
#if [ENABLE_SHARED]
                  libstash_install_s
#endif
{
#if [ENABLE_SHARED]
  [CC] [D_CFLAGS] -o [PREFIX]/bin/xvx
       [fromto [srcdir]/%0%.c [objdir]/%0%.o_a [XVX_SRCS]]
       -rpath [PREFIX]/lib
       [LDFLAGS] -lstash [EXTRALIBS];
  [CHMOD] 0755 [PREFIX]/bin/xvx;
#else
	[INSTALL] -m 0755 [XVX_A] [PREFIX]/bin;
#endif
}

xvx_install_p : [XVX_P] xvx_install_common
{
	[INSTALL] -m 0755 [XVX_P] [PREFIX]/bin;
}

/*
 * Dependencies.
 */

[XVX_D] : [fromto [srcdir]/%0%.c [objdir]/%0%.o_d [XVX_SRCS]]
           [LIBSTASH_D]
  set mkdir
{
  [CC] [D_CFLAGS] -o [target]
       [fromto [srcdir]/%0%.c [objdir]/%0%.o_d [XVX_SRCS]]
       [LDFLAGS] -lstash_d [EXTRALIBS];
}

[XVX_A] : [fromto [srcdir]/%0%.c [objdir]/%0%.o_a [XVX_SRCS]]
#if [ENABLE_SHARED]
           [LIBSTASH_S]
#else
           [LIBSTASH_A]
#endif
  set mkdir
{
  [CC] [A_CFLAGS] -o [target]
       [fromto [srcdir]/%0%.c [objdir]/%0%.o_a [XVX_SRCS]]
#if [ENABLE_SHARED]
       -rpath [objdir]/lib/c/libstash/lib
#endif
       [LDFLAGS] -lstash [EXTRALIBS];
}

[XVX_P] : [fromto [srcdir]/%0%.c [objdir]/%0%.o_p [XVX_SRCS]]
           [LIBSTASH_P]
  set mkdir
{
  [CC] [P_CFLAGS] -o [target]
       [fromto [srcdir]/%0%.c [objdir]/%0%.o_p [XVX_SRCS]]
       [LDFLAGS] -lstash_p [EXTRALIBS];
}
