/******************************************************************************
 *
 * <Copyright = jasone>
 * <License>
 *
 ******************************************************************************
 *
 * Version: <Version>
 *
 * cook file for libprism.
 *
 ******************************************************************************/

/* Include generated dependency files. */
#include-cooked [glob [objdir]/lib/c/libprism/src/"*.d_*"]
#include-cooked [glob [objdir]/lib/c/libprism/bin/"*.d_*"]
#include-cooked [glob [objdir]/lib/c/libprism/test/"*.d_*"]

/*
 * File lists.
 */

/* Include files that _are_ installed. */
{
  local incs = libprism.h libprism_incs.h;
  incs = [fromto % [srcdir]/lib/c/libprism/include/libprism/% [incs]];
  incs = [incs] [objdir]/lib/c/libprism/include/libprism/libprism_defs.h;

  LIBPRISM_INCS = [incs];
  INCS = [INCS] [incs];
}

/* Include files that are _not_ installed. */
{
  local priv_incs = ;
  priv_incs = [fromto % [srcdir]/lib/c/libprism/include/libprism/% [priv_incs]];

  PRIV_INCS = [PRIV_INCS] [priv_incs];
}

/* Source files. */
LIBPRISM_SRCS = ;
LIBPRISM_SRCS = [fromto % [srcdir]/lib/c/libprism/src/% [LIBPRISM_SRCS]];
SRCS = [SRCS] [LIBPRISM_SRCS];

/* Simple C tests.  Output is diff'ed with the expected (.exp file) output. */
{
  local ctests = hook_a.c;
  ctests = [fromto % [srcdir]/lib/c/libprism/test/% [ctests]];

  CTESTS = [CTESTS] [ctests];
}

/*
 * Simple prism tests.  Output is diff'ed with the expected (.exp file)
 * output.
 */
{
  local otests = ;
  otests = [fromto % [srcdir]/lib/c/libprism/test/% [otests]];

  OTESTS = [OTESTS] [otests];
}

/* Perl scripts that drive other tests. */
{
  local ptests = ;
  ptests = [fromto % [srcdir]/lib/c/libprism/test/% [ptests]];

  PTESTS = [PTESTS] [ptests];
}

/* C programs that are driven by tests. */
{
  local cbtests = ;
  cbtests = [fromto % [srcdir]/lib/c/libprism/test/% [cbtests]];

  CBTESTS = [CBTESTS] [cbtests];
}

/* prism programs that are driven by tests. */
{
  local obtests = ;
  obtests = [fromto % [srcdir]/lib/c/libprism/test/% [obtests]];

  OBTESTS = [OBTESTS] [obtests];
}

/* LaTeX source files. */
{
  local tex_srcs = ;
  tex_srcs = [fromto % [srcdir]/lib/c/libprism/doc/latex/% [tex_srcs]];
  tex_srcs = [tex_srcs] [objdir]/lib/c/libprism/doc/latex/manual.tex;

  TEX_SRCS = [TEX_SRCS] [tex_srcs];
}

/* xfig source files. */
{
  local fig_srcs = ;
  fig_srcs = [fromto % [srcdir]/lib/c/libprism/doc/latex/% [fig_srcs]];

  FIG_SRCS = [FIG_SRCS] [fig_srcs];
}

/* Libraries. */
LIBPRISM_REV = 0.0;

LIBPRISM_D = [objdir]/lib/c/libprism/lib/libprism_d.a;
LIBPRISM_A = [objdir]/lib/c/libprism/lib/libprism.a;
LIBPRISM_S = [objdir]/lib/c/libprism/lib/libprism.so.[LIBPRISM_REV];
LIBPRISM_P = [objdir]/lib/c/libprism/lib/libprism_p.a;

LDFLAGS = [LDFLAGS] -L[objdir]/lib/c/libprism/lib;
LIBS = [LIBS] -lprism;
D_LIBS = [D_LIBS] [LIBPRISM_D];
A_LIBS = [A_LIBS] [LIBPRISM_A];
P_LIBS = [P_LIBS] [LIBPRISM_P];
S_LIBS = [S_LIBS] [LIBPRISM_S];
S_LINKS = [S_LINKS] [objdir]/lib/c/libprism/lib/libprism.so;

/*
 * User cook'ables.
 */

prism_bootstrap : [PRISM_D]
{
  cat [srcdir]/lib/c/libprism/src/softop.nx
    | [PRISM_D] [srcdir]/lib/c/libprism/src/softop_gen.nx
    > [srcdir]/lib/c/libprism/src/softop.c;
}

libprism : [LIBPRISM_D] [LIBPRISM_A] [LIBPRISM_P]
#if [ENABLE_SHARED]
          [LIBPRISM_S]
#endif
;

libprism_install : libprism_install_d libprism_install_a libprism_install_p
#if [ENABLE_SHARED]
                   libprism_install_s
#endif
;

libprism_uninstall :
{
  [SHELL] -e;
data
#![SHELL]

echo rm -f [PREFIX]/lib/libprism*
rm -f [PREFIX]/lib/libprism*

echo rm -rf [PREFIX]/include/libprism
rm -rf [PREFIX]/include/libprism
dataend
}

libprism_clean :
{
}

libprism_distclean :
{
  rm -f [objdir]/lib/c/libprism/doc/latex/manual.tex;
}

/*
 * Various flags.
 */

CPPFLAGS = [CPPFLAGS] -I[srcdir]/lib/c/libprism/include
                      -I[objdir]/lib/c/libprism/include/libprism;
D_CPPFLAGS = [D_CPPFLAGS] -D_LIBPRISM_DBG;

/*
 * Build rules.
 */

libprism_install_common :
{
  [SHELL] -e;
data
#![SHELL]

echo [INSTALL] -d [PREFIX]/include/libprism
[INSTALL] -d [PREFIX]/include/libprism

for i in [LIBPRISM_INCS]; do
	echo [INSTALL] -m 0644 $i [PREFIX]/include/libprism/
	[INSTALL] -m 0644 $i [PREFIX]/include/libprism/
done

echo [INSTALL] -d [PREFIX]/lib
[INSTALL] -d [PREFIX]/lib
dataend
}

libprism_install_d : [LIBPRISM_D] libprism_install_common
{
	[INSTALL] -m 0444 [LIBPRISM_D] [PREFIX]/lib;
}

libprism_install_a : [LIBPRISM_A] libprism_install_common
{
	[INSTALL] -m 0444 [LIBPRISM_A] [PREFIX]/lib;
}

libprism_install_p : [LIBPRISM_P] libprism_install_common
{
	[INSTALL] -m 0444 [LIBPRISM_P] [PREFIX]/lib;
}

libprism_install_s : [LIBPRISM_S] libprism_install_common
{
	[INSTALL] -m 0444 [LIBPRISM_S] [PREFIX]/lib;
	[LN] -sf libprism.so.[LIBPRISM_REV] [PREFIX]/lib/libprism.so;
}

/*
 * Dependencies.
 */

[LIBPRISM_D] : [fromto [srcdir]/%0%.c [objdir]/%0%.o_d [LIBPRISM_SRCS]]
  set mkdir
{
  [AR] cru [target] [younger];
  [RANLIB] [target];
}

[LIBPRISM_A] : [fromto [srcdir]/%0%.c [objdir]/%0%.o_a [LIBPRISM_SRCS]]
  set mkdir
{
  [AR] cru [target] [younger];
  [RANLIB] [target];
}

[LIBPRISM_P] : [fromto [srcdir]/%0%.c [objdir]/%0%.o_p [LIBPRISM_SRCS]]
  set mkdir
{
  [AR] cru [target] [younger];
  [RANLIB] [target];
}

[LIBPRISM_S] : [fromto [srcdir]/%0%.c [objdir]/%0%.o_s [LIBPRISM_SRCS]]
  set mkdir
{
  [CC] -shared -o [target] [need];
  [LN] -sf libprism.so.[LIBPRISM_REV] [objdir]/lib/c/libprism/lib/libprism.so;
}
