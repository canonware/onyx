/******************************************************************************
 *
 * <Copyright = jasone>
 * <License>
 *
 ******************************************************************************
 *
 * Version: <Version>
 *
 * cook file for blow.
 *
 ******************************************************************************/

/* Include generated dependency files. */
#include-cooked [glob [objdir]/bin/blow/src/"*.d_*"]

/*
 * File lists.
 */

/* Include files that _are_ installed. */
{
  local incs = ;
  incs = [fromto % [srcdir]/bin/blow/include/% [incs]];

  INCS = [INCS] [incs];
}

/* Include files that are _not_ installed. */
{
  local priv_incs = ;
  priv_incs = [fromto % [srcdir]/bin/blow/include/% [priv_incs]];
  priv_incs = [priv_incs] [objdir]/bin/blow/include/blow_defs.h;

  PRIV_INCS = [PRIV_INCS] [priv_incs];
}

/* Source files. */
BLOW_SRCS = blow.c;
BLOW_SRCS = [fromto % [srcdir]/bin/blow/src/% [BLOW_SRCS]];
SRCS = [SRCS] [BLOW_SRCS];

/* Simple C tests.  Output is diff'ed with the expected (.exp file) output. */
{
  local ctests = ;
  ctests = [fromto % [srcdir]/bin/blow/test/% [ctests]];

  CTESTS = [CTESTS] [ctests];
}

/*
 * Simple stil tests.  Output is diff'ed with the expected (.exp file)
 * output.
 */
{
  local stests = ;
  stests = [fromto % [srcdir]/bin/blow/test/% [stests]];

  STESTS = [STESTS] [stests];
}

/* Perl scripts that drive other tests and use the sequence mode of verify. */
{
  local ptests = ;
  ptests = [fromto % [srcdir]/bin/blow/test/% [ptests]];

  PTESTS = [PTESTS] [ptests];
}

/* C programs that are driven by tests. */
{
  local cbtests = ;
  cbtests = [fromto % [srcdir]/bin/blow/test/% [cbtests]];

  CBTESTS = [CBTESTS] [cbtests];
}

/* stil programs that are driven by tests. */
{
  local sbtests = ;
  sbtests = [fromto % [srcdir]/bin/blow/test/% [sbtests]];

  SBTESTS = [SBTESTS] [sbtests];
}

/* Man pages. */
{
  local manpages_1 = ;
  manpages_1 = [fromto % [srcdir]/bin/blow/man/man1/% [manpages_1]];

  BLOW_MANPAGES_1 = [manpages_1];
  MANPAGES_1 = [MANPAGES_1] [manpages_1];
}

/* Binaries. */
BLOW = [objdir]/bin/blow/bin/blow;
BLOW_D = [BLOW].dbg;
BLOW_A = [BLOW];
BLOW_P = [BLOW].prof;

D_BINS = [D_BINS] [BLOW_D];
A_BINS = [A_BINS] [BLOW_A];
P_BINS = [P_BINS] [BLOW_P];

/*
 * User cook'ables.
 */

blow_all : blow_d blow_a blow_p;
blow_d : [BLOW_D];
blow_a : [BLOW_A];
blow_p : [BLOW_P];

blow_install : blow_install_d blow_install_a blow_install_p;

blow_uninstall :
{
  [SHELL] -e;
data
#![SHELL]

echo rm -f [PREFIX]/bin/blow.dbg
rm -f [PREFIX]/bin/blow.dbg

echo rm -f [PREFIX]/bin/blow
rm -f [PREFIX]/bin/blow

echo rm -f [PREFIX]/bin/blow.prof
rm -f [PREFIX]/bin/blow.prof

for i in [BLOW_MANPAGES_1]; do
	echo rm -f [PREFIX]/man/man1/`[BASENAME] $i`
	rm -f [PREFIX]/man/man1/`[BASENAME] $i`
done
dataend
}

blow_clean :
{
}

blow_distclean :
{
}

/*
 * Build rules.
 */

blow_install_common :
{
  [SHELL] -e;
data
#![SHELL]

echo [INSTALL] -d [PREFIX]/man/man1
[INSTALL] -d [PREFIX]/man/man1

for i in [BLOW_MANPAGES_1]; do
	echo [INSTALL] -m 0644 $i [PREFIX]/man/man1/
	[INSTALL] -m 0644 $i [PREFIX]/man/man1/
done

echo [INSTALL] -d [PREFIX]/bin
[INSTALL] -d [PREFIX]/bin
dataend
}

blow_install_d : [BLOW_D] blow_install_common
{
	[INSTALL] -m 0755 [BLOW_D] [PREFIX]/bin;
}

blow_install_a : [BLOW_A] blow_install_common
#if [ENABLE_SHARED]
                 libsock_install_s libstash_install_s
#endif
{
#if [ENABLE_SHARED]
  [CC] [D_CFLAGS] -o [PREFIX]/bin/blow
       [fromto [srcdir]/%0%.c [objdir]/%0%.o_a [BLOW_SRCS]]
       -rpath [PREFIX]/lib
       [LDFLAGS] -lsock -lstash [EXTRALIBS];
  [CHMOD] 0755 [PREFIX]/bin/blow;
#else
	[INSTALL] -m 0755 [BLOW_A] [PREFIX]/bin;
#endif
}

blow_install_p : [BLOW_P] blow_install_common
{
	[INSTALL] -m 0755 [BLOW_P] [PREFIX]/bin;
}

/*
 * Dependencies.
 */

[BLOW_D] : [fromto [srcdir]/%0%.c [objdir]/%0%.o_d [BLOW_SRCS]]
           [LIBSOCK_D] [LIBSTASH_D]
  set mkdir
{
  [CC] [D_CFLAGS] -o [target]
       [fromto [srcdir]/%0%.c [objdir]/%0%.o_d [BLOW_SRCS]]
       [LDFLAGS] -lsock_d -lstash_d [EXTRALIBS];
}

[BLOW_A] : [fromto [srcdir]/%0%.c [objdir]/%0%.o_a [BLOW_SRCS]]
#if [ENABLE_SHARED]
           [LIBSOCK_S] [LIBSTASH_S]
#else
           [LIBSOCK_A] [LIBSTASH_A]
#endif
  set mkdir
{
  [CC] [A_CFLAGS] -o [target]
       [fromto [srcdir]/%0%.c [objdir]/%0%.o_a [BLOW_SRCS]]
#if [ENABLE_SHARED]
       -rpath [objdir]/lib/c/libsock/lib
       -rpath [objdir]/lib/c/libstash/lib
#endif
       [LDFLAGS] -lsock -lstash [EXTRALIBS];
}

[BLOW_P] : [fromto [srcdir]/%0%.c [objdir]/%0%.o_p [BLOW_SRCS]]
           [LIBSOCK_P] [LIBSTASH_P]
  set mkdir
{
  [CC] [P_CFLAGS] -o [target]
       [fromto [srcdir]/%0%.c [objdir]/%0%.o_p [BLOW_SRCS]]
       [LDFLAGS] -lsock_p -lstash_p [EXTRALIBS];
}
