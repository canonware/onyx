/******************************************************************************
 *
 * <Copyright = jasone>
 * <License>
 *
 ******************************************************************************
 *
 * Version: <Version>
 *
 * cook file for slate.
 *
 ******************************************************************************/

/* Include generated dependency files. */
#include-cooked [glob [objdir]/bin/slate/src/"*.d_*"]

/*
 * File lists.
 */

/* Include files that are _not_ installed. */
{
  local priv_incs = buf.h buffer.h hist.h slate.h slate_ops.h;
  priv_incs = [fromto % [srcdir]/bin/slate/include/% [priv_incs]];
  priv_incs = [priv_incs] [objdir]/bin/slate/include/slate_defs.h;

  local priv_cincs = slate_nxcode.c;
  prix_cincs = [fromto % [srcdir]/bin/slate/src/% [priv_cincs]];

  PRIV_INCS = [PRIV_INCS] [priv_incs] [priv_cincs];
}

/* Source files. */
SLATE_SRCS = buf.c buffer.c hist.c slate.c slate_ops.c;
SLATE_SRCS = [fromto % [srcdir]/bin/slate/src/% [SLATE_SRCS]];

TSLATE_SRCS = tslate.c;
TSLATE_SRCS = [fromto % [srcdir]/bin/slate/src/% [TSLATE_SRCS]];

SRCS = [SRCS] [SLATE_SRCS] [TSLATE_SRCS];

/* LaTeX source files. */
{
  local tex_srcs = ;
  tex_srcs = [fromto % [srcdir]/bin/slate/doc/latex/% [tex_srcs]];

  TEX_SRCS = [TEX_SRCS] [tex_srcs];
}

/* Binaries. */
SLATE = [objdir]/bin/slate/bin/slate;
TSLATE = [objdir]/bin/slate/bin/tslate;
BINS = [BINS] [SLATE] [TSLATE];

[TSLATE] : [SLATE];

/*
 * User cook'ables.
 */

BOOTSTRAP = [BOOTSTRAP] slate_bootstrap;

slate_bootstrap : [ONYX]
{
  cat [srcdir]/bin/slate/src/slate_nxcode.nx
    | [ONYX] [srcdir]/bin/slate/src/slate_nxcode_gen.nx
    > [srcdir]/bin/slate/src/slate_nxcode.c;
}

slate : [SLATE];
tslate : [TSLATE];

slate_install : [SLATE]
#if [matches macho [abi]]
/* Make sure install_name is set correctly before relinking. */
                install_libs
#endif
{
  [INSTALL] -d [PREFIX]/bin;

  [CC] [A_CFLAGS] -o [PREFIX]/bin/slate
       [fromto [srcdir]/%0%.c [objdir]/%0%.o_a [SLATE_SRCS]]
#if [matches elf [abi]]
       [RPATH][PREFIX]/lib
#endif
       [LDFLAGS] -lonyx
       [EXTRALIBS];
  [CHMOD] 0755 [PREFIX]/bin/slate;

/*  [INSTALL] -d [PREFIX]/man/man1;
  [INSTALL] -m 0444 [objdir]/bin/slate/man/man1/slate.1 [PREFIX]/man/man1; */
}

slate_uninstall :
{
  rm -f [PREFIX]/bin/slate;
/*  rm -f [PREFIX]/man/man1/slate.1; */
}

slate_clean :
{
}

slate_distclean :
{
  rm -f [objdir]/bin/slate/doc/latex/manual.tex;
/*  rm -f [objdir]/bin/slate/man/man1/slate.1; */
}

/*
 * Dependencies.
 */

[TSLATE] : [fromto [srcdir]/%0%.c [objdir]/%0%.o_a [TSLATE_SRCS]]
  set mkdir
{
  [CC] [A_CFLAGS] -o [target]
       [fromto [srcdir]/%0%.c [objdir]/%0%.o_a [TSLATE_SRCS]];
}

[SLATE] : [fromto [srcdir]/%0%.c [objdir]/%0%.o_a [SLATE_SRCS]]
#if [enable_libonyx]
          [LIBONYX_S]
#endif
#if [enable_libedit]
          [LIBEDIT_A]
#endif
  set mkdir
{
  [CC] [A_CFLAGS] -o [target]
       [fromto [srcdir]/%0%.c [objdir]/%0%.o_a [SLATE_SRCS]]
#if [enable_libedit]
       [LIBEDIT_A]
#endif
#if [enable_libonyx]
#if [matches elf [abi]]
       [RPATH][abs_objdir]/lib/libonyx/lib
#endif
#endif
       [LDFLAGS] -lonyx
       [EXTRALIBS];
}
