/******************************************************************************
 *
 * <Copyright = jasone>
 * <License>
 *
 ******************************************************************************
 *
 * Version: <Version>
 *
 * cook file for libonyx.
 *
 ******************************************************************************/

/* Include generated dependency files. */
#include-cooked [glob [objdir]/lib/c/libonyx/src/"*.d_*"]
#include-cooked [glob [objdir]/lib/c/libonyx/bin/"*.d_*"]
#include-cooked [glob [objdir]/lib/c/libonyx/test/"*.d_*"]

/*
 * File lists.
 */

/* Include files that _are_ installed. */
{
  local incs = libonyx.h libonyx_incs.h errordict.h gcdict.h nx.h nxa.h nxn.h
               nxo.h nxo_array.h nxo_boolean.h nxo_condition.h nxo_dict.h
               nxo_file.h nxo_fino.h nxo_hook.h nxo_integer.h nxo_mark.h
               nxo_mutex.h nxo_name.h nxo_no.h nxo_null.h nxo_operator.h
               nxo_stack.h nxo_string.h nxo_thread.h systemdict.h;
  incs = [fromto % [srcdir]/lib/c/libonyx/include/libonyx/% [incs]];
  incs = [incs] [objdir]/lib/c/libonyx/include/libonyx/libonyx_defs.h;

  LIBONYX_INCS = [incs];
  INCS = [INCS] [incs];
}

/* Include files that are _not_ installed. */
{
  local priv_incs = currenterror_l.h envdict_l.h errordict_l.h gcdict_l.h
                    nx_l.h nxaa_l.h nxo_l.h nxo_array_l.h nxo_boolean_l.h
                    nxo_condition_l.h nxo_dict_l.h nxo_file_l.h nxo_fino_l.h
                    nxo_hook_l.h nxo_integer_l.h nxo_mark_l.h nxo_mutex_l.h
                    nxo_name_l.h nxo_null_l.h nxo_operator_l.h nxo_stack_l.h
                    nxo_string_l.h nxo_thread_l.h sprintdict_l.h systemdict_l.h;
  priv_incs = [fromto % [srcdir]/lib/c/libonyx/include/libonyx/% [priv_incs]];

  PRIV_INCS = [PRIV_INCS] [priv_incs];
}

/* Source files. */
LIBONYX_SRCS = currenterror.c envdict.c errordict.c gcdict.c nx.c nxa.c nxn.c
               nxo.c nxo_array.c nxo_boolean.c nxo_condition.c nxo_dict.c
               nxo_file.c nxo_fino.c nxo_hook.c nxo_integer.c nxo_mark.c
               nxo_mutex.c nxo_name.c nxo_null.c nxo_operator.c nxo_stack.c
               nxo_string.c nxo_thread.c sprintdict.c systemdict.c;
LIBONYX_SRCS = [fromto % [srcdir]/lib/c/libonyx/src/% [LIBONYX_SRCS]];
SRCS = [SRCS] [LIBONYX_SRCS];

/* Simple C tests.  Output is diff'ed with the expected (.exp file) output. */
{
  local ctests = hook_a.c;
  ctests = [fromto % [srcdir]/lib/c/libonyx/test/% [ctests]];

  CTESTS = [CTESTS] [ctests];
}

/*
 * Simple onyx tests.  Output is diff'ed with the expected (.exp file)
 * output.
 */
{
  local otests = ;
  otests = [fromto % [srcdir]/lib/c/libonyx/test/% [otests]];

  OTESTS = [OTESTS] [otests];
}

/* Perl scripts that drive other tests. */
{
  local ptests = ;
  ptests = [fromto % [srcdir]/lib/c/libonyx/test/% [ptests]];

  PTESTS = [PTESTS] [ptests];
}

/* C programs that are driven by tests. */
{
  local cbtests = ;
  cbtests = [fromto % [srcdir]/lib/c/libonyx/test/% [cbtests]];

  CBTESTS = [CBTESTS] [cbtests];
}

/* onyx programs that are driven by tests. */
{
  local obtests = ;
  obtests = [fromto % [srcdir]/lib/c/libonyx/test/% [obtests]];

  OBTESTS = [OBTESTS] [obtests];
}

/* LaTeX source files. */
{
  local tex_srcs = gcdict.tex nx.tex nxa.tex nxn.tex nxo.tex systemdict.tex;
  tex_srcs = [fromto % [srcdir]/lib/c/libonyx/doc/latex/% [tex_srcs]];
  tex_srcs = [tex_srcs] [objdir]/lib/c/libonyx/doc/latex/manual.tex;

  TEX_SRCS = [TEX_SRCS] [tex_srcs];
}

/* xfig source files. */
{
  local fig_srcs = ;
  fig_srcs = [fromto % [srcdir]/lib/c/libonyx/doc/latex/% [fig_srcs]];

  FIG_SRCS = [FIG_SRCS] [fig_srcs];
}

/* Libraries. */
LIBONYX_REV = 0.0;

LIBONYX_D = [objdir]/lib/c/libonyx/lib/libonyx_d.a;
LIBONYX_A = [objdir]/lib/c/libonyx/lib/libonyx.a;
LIBONYX_S = [objdir]/lib/c/libonyx/lib/libonyx.so.[LIBONYX_REV];
LIBONYX_P = [objdir]/lib/c/libonyx/lib/libonyx_p.a;

LDFLAGS = [LDFLAGS] -L[objdir]/lib/c/libonyx/lib;
LIBS = [LIBS] -lonyx;
D_LIBS = [D_LIBS] [LIBONYX_D];
A_LIBS = [A_LIBS] [LIBONYX_A];
P_LIBS = [P_LIBS] [LIBONYX_P];
S_LIBS = [S_LIBS] [LIBONYX_S];
S_LINKS = [S_LINKS] [objdir]/lib/c/libonyx/lib/libonyx.so;

/*
 * User cook'ables.
 */

onyx_bootstrap : [ONYX_D]
{
  cat [srcdir]/lib/c/libonyx/src/softop.nx
    | [ONYX_D] [srcdir]/lib/c/libonyx/src/softop_gen.nx
    > [srcdir]/lib/c/libonyx/src/softop.c;
}

libonyx : [LIBONYX_D] [LIBONYX_A] [LIBONYX_P]
#if [ENABLE_SHARED]
          [LIBONYX_S]
#endif
;

libonyx_install : libonyx_install_d libonyx_install_a libonyx_install_p
#if [ENABLE_SHARED]
                   libonyx_install_s
#endif
;

libonyx_uninstall :
{
  [SHELL] -e;
data
#![SHELL]

echo rm -f [PREFIX]/lib/libonyx*
rm -f [PREFIX]/lib/libonyx*

echo rm -rf [PREFIX]/include/libonyx
rm -rf [PREFIX]/include/libonyx
dataend
}

libonyx_clean :
{
}

libonyx_distclean :
{
  rm -f [objdir]/lib/c/libonyx/doc/latex/manual.tex;
}

/*
 * Various flags.
 */

CPPFLAGS = [CPPFLAGS] -I[srcdir]/lib/c/libonyx/include
                      -I[objdir]/lib/c/libonyx/include/libonyx;
D_CPPFLAGS = [D_CPPFLAGS] -D_LIBONYX_DBG;

/*
 * Build rules.
 */

libonyx_install_common :
{
  [SHELL] -e;
data
#![SHELL]

echo [INSTALL] -d [PREFIX]/include/libonyx
[INSTALL] -d [PREFIX]/include/libonyx

for i in [LIBONYX_INCS]; do
	echo [INSTALL] -m 0644 $i [PREFIX]/include/libonyx/
	[INSTALL] -m 0644 $i [PREFIX]/include/libonyx/
done

echo [INSTALL] -d [PREFIX]/lib
[INSTALL] -d [PREFIX]/lib
dataend
}

libonyx_install_d : [LIBONYX_D] libonyx_install_common
{
	[INSTALL] -m 0444 [LIBONYX_D] [PREFIX]/lib;
}

libonyx_install_a : [LIBONYX_A] libonyx_install_common
{
	[INSTALL] -m 0444 [LIBONYX_A] [PREFIX]/lib;
}

libonyx_install_p : [LIBONYX_P] libonyx_install_common
{
	[INSTALL] -m 0444 [LIBONYX_P] [PREFIX]/lib;
}

libonyx_install_s : [LIBONYX_S] libonyx_install_common
{
	[INSTALL] -m 0444 [LIBONYX_S] [PREFIX]/lib;
	[LN] -sf libonyx.so.[LIBONYX_REV] [PREFIX]/lib/libonyx.so;
}

/*
 * Dependencies.
 */

[LIBONYX_D] : [fromto [srcdir]/%0%.c [objdir]/%0%.o_d [LIBONYX_SRCS]]
  set mkdir
{
  [AR] cru [target] [younger];
  [RANLIB] [target];
}

[LIBONYX_A] : [fromto [srcdir]/%0%.c [objdir]/%0%.o_a [LIBONYX_SRCS]]
  set mkdir
{
  [AR] cru [target] [younger];
  [RANLIB] [target];
}

[LIBONYX_P] : [fromto [srcdir]/%0%.c [objdir]/%0%.o_p [LIBONYX_SRCS]]
  set mkdir
{
  [AR] cru [target] [younger];
  [RANLIB] [target];
}

[LIBONYX_S] : [fromto [srcdir]/%0%.c [objdir]/%0%.o_s [LIBONYX_SRCS]]
  set mkdir
{
  [CC] -shared -o [target] [need];
  [LN] -sf libonyx.so.[LIBONYX_REV] [objdir]/lib/c/libonyx/lib/libonyx.so;
}
