/******************************************************************************
 *
 * <Copyright = jasone>
 * <License>
 *
 ******************************************************************************
 *
 * Version: <Version>
 *
 * cook file for nxe.
 *
 ******************************************************************************/

/* Include generated dependency files. */
#include-cooked [glob [objdir]/bin/nxe/src/"*.d_*"]

/*
 * File lists.
 */

/* Include files that _are_ installed. */
{
  local incs = ;
  incs = [fromto % [srcdir]/bin/nxe/include/% [incs]];

  INCS = [INCS] [incs];
}

/* Include files that are _not_ installed. */
{
  local priv_incs = buf.h nxe.h;
  priv_incs = [fromto % [srcdir]/bin/nxe/include/% [priv_incs]];
  priv_incs = [priv_incs] [objdir]/bin/nxe/include/nxe_defs.h;

  local priv_cincs = ;
  prix_cincs = [fromto % [srcdir]/bin/nxe/src/% [priv_cincs]];

  PRIV_INCS = [PRIV_INCS] [priv_incs] [priv_cincs];
}

/* Source files. */
NXE_SRCS = buf.c buffer.c nxe.c nxe_ops.c;
NXE_SRCS = [fromto % [srcdir]/bin/nxe/src/% [NXE_SRCS]];
SRCS = [SRCS] [NXE_SRCS];

/* Simple C tests.  Output is diff'ed with the expected (.exp file) output. */
{
  local ctests = ;
  ctests = [fromto % [srcdir]/bin/nxe/test/% [ctests]];

  CTESTS = [CTESTS] [ctests];
}

/*
 * Onyx tests.
 */
{
  local otests = ;
  otests = [fromto % [srcdir]/bin/nxe/test/% [otests]];

  OTESTS = [OTESTS] [otests];
}

/* Perl scripts that drive other tests and use the sequence mode of verify. */
{
  local ptests = ;
  ptests = [fromto % [srcdir]/bin/nxe/test/% [ptests]];

  PTESTS = [PTESTS] [ptests];
}

/* C programs that are driven by tests. */
{
  local cbtests = ;
  cbtests = [fromto % [srcdir]/bin/nxe/test/% [cbtests]];

  CBTESTS = [CBTESTS] [cbtests];
}

/* Onyx programs that are driven by tests. */
{
  local obtests = ;
  obtests = [fromto % [srcdir]/bin/nxe/test/% [obtests]];

  OBTESTS = [OBTESTS] [obtests];
}

/* LaTeX source files. */
{
  local tex_srcs = ;
  tex_srcs = [fromto % [srcdir]/bin/nxe/doc/latex/% [tex_srcs]];
  tex_srcs = [tex_srcs] [objdir]/bin/nxe/doc/latex/manual.tex;

  TEX_SRCS = [TEX_SRCS] [tex_srcs];
}

/* xfig source files. */
{
  local fig_srcs = ;
  fig_srcs = [fromto % [srcdir]/bin/nxe/doc/latex/% [fig_srcs]];

  FIG_SRCS = [FIG_SRCS] [fig_srcs];
}

/* Binaries. */
NXE = [objdir]/bin/nxe/bin/nxe;
BINS = [BINS] [NXE];

/*
 * User cook'ables.
 */

nxe : [NXE];

nxe_install_self : [NXE]
{
  [INSTALL] -d [PREFIX]/bin;
  [CC] [A_CFLAGS] -o [PREFIX]/bin/nxe
       [fromto [srcdir]/%0%.c [objdir]/%0%.o_a [NXE_SRCS]]
#if [enable_libedit]
       [LIBEDIT_A]
#endif
       [RPATH][PREFIX]/lib
       [LDFLAGS] -lonyx -lcurses [EXTRALIBS];
  [CHMOD] 0755 [PREFIX]/bin/nxe;

  [INSTALL] -d [PREFIX]/man/man1;
  [INSTALL] -m 0444 [objdir]/bin/nxe/man/man1/nxe.1 [PREFIX]/man/man1;
}

nxe_install : nxe_install_self
               libonyx_install_s /* libedit_install_s */;

nxe_uninstall :
{
  rm -f [PREFIX]/bin/nxe;
  rm -f [PREFIX]/man/man1/nxe.1;
}

nxe_clean :
{
}

nxe_distclean :
{
  rm -f [objdir]/bin/nxe/doc/latex/manual.tex;
  rm -f [objdir]/bin/nxe/man/man1/nxe.1;
}

/*
 * Dependencies.
 */

[NXE] : [fromto [srcdir]/%0%.c [objdir]/%0%.o_a [NXE_SRCS]]
         [LIBONYX_S]
#if [enable_libedit]
         [LIBEDIT_A]
#endif
         [MODS]
  set mkdir
{
  [CC] [A_CFLAGS] -o [target]
       [fromto [srcdir]/%0%.c [objdir]/%0%.o_a [NXE_SRCS]]
#if [enable_libedit]
       [LIBEDIT_A]
#endif
/*       [RPATH][abs_objdir]/lib/c/libedit/lib */
       [RPATH][abs_objdir]/lib/c/libonyx/lib
       [LDFLAGS] -lonyx -lcurses [EXTRALIBS];
}
