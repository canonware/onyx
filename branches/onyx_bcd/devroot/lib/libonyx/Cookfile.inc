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
#include-cooked [glob [objdir]/lib/libonyx/src/"*.d_*"]
#include-cooked [glob [objdir]/lib/libonyx/test/"*.d_*"]

/*
 * File lists.
 */

/* Include files that _are_ installed. */
{
  local incs = libonyx.h ch.h dch.h gcdict.h mem.h nx.h nxa.h nxn.h nxo.h
               nxo_array.h nxo_boolean.h nxo_dict.h nxo_file.h nxo_fino.h
               nxo_hook.h nxo_mark.h nxo_name.h nxo_no.h nxo_null.h
               nxo_number.h nxo_operator.h nxo_pmark.h nxo_stack.h nxo_string.h
               nxo_thread.h ql.h qr.h qs.h systemdict.h xep.h
#if [enable_threads]
               cnd.h mq.h mtx.h nxo_condition.h nxo_mutex.h thd.h tsd.h
#endif
               ;
  incs = [fromto % [srcdir]/lib/libonyx/include/libonyx/% [incs]];
  incs = [incs] [objdir]/lib/libonyx/include/libonyx/libonyx_defs.h;

  LIBONYX_INCS = [incs];
  INCS = [INCS] [incs];
}

/* Include files that are _not_ installed. */
{
  local priv_incs = gcdict_l.h nx_l.h nxa_l.h nxo_l.h nxo_array_l.h nxo_dict_l.h
                    nxo_file_l.h nxo_hook_l.h nxo_name_l.h nxo_number_l.h
                    nxo_operator_l.h nxo_stack_l.h nxo_string_l.h nxo_thread_l.h
                    systemdict_l.h
#if [enable_threads]
                    nxo_condition_l.h nxo_mutex_l.h
#endif
#if [enable_posix]
                    envdict_l.h
#endif
                    ;
  priv_incs = [fromto % [srcdir]/lib/libonyx/include/libonyx/% [priv_incs]];

  local priv_cincs = nx_nxcode.c nxo_thread_nxcode.c;
  prix_cincs = [fromto % [srcdir]/lib/libonyx/src/% [priv_cincs]];

  PRIV_INCS = [PRIV_INCS] [priv_incs] [priv_cincs];
}

/* Source files. */
LIBONYX_SRCS = libonyx.c ch.c dch.c gcdict.c mem.c nx.c nxa.c nxn.c nxo.c
               nxo_array.c nxo_boolean.c nxo_dict.c nxo_file.c nxo_fino.c
               nxo_hook.c nxo_mark.c nxo_name.c nxo_null.c nxo_number.c
               nxo_operator.c nxo_pmark.c nxo_stack.c nxo_string.c nxo_thread.c
               systemdict.c xep.c
#if [enable_threads]
               cnd.c mq.c mtx.c nxo_condition.c nxo_mutex.c thd.c tsd.c
#endif
#if [enable_posix]
               envdict.c
#endif
               ;

LIBONYX_SRCS = [fromto % [srcdir]/lib/libonyx/src/% [LIBONYX_SRCS]];
SRCS = [SRCS] [LIBONYX_SRCS];

/* Simple C tests.  Output is diff'ed with the expected (.exp file) output. */
{
  local ctests = ch_a.c dch_a.c libonyx_a.c ql_a.c qr_a.c qs_a.c xep_a.c
#if [enable_threads]
                 cnd_a.c mq_a.c mtx_a.c thd_a.c thd_b.c thd_c.c tsd_a.c
#endif
#if [enable_posix_file]
                 hook_a.c
#endif
                 ;
  ctests = [fromto % [srcdir]/lib/libonyx/test/% [ctests]];

  CTESTS = [CTESTS] [ctests];
}

/* LaTeX source files. */
{
  local tex_srcs = ch.tex cnd.tex dch.tex gcdict.tex mem.tex mq.tex mtx.tex
                   nx.tex nxa.tex nxn.tex nxo.tex nxo_array.tex nxo_boolean.tex
                   nxo_dict.tex nxo_file.tex nxo_fino.tex nxo_hook.tex
                   nxo_integer.tex nxo_mark.tex nxo_mutex.tex nxo_name.tex
                   nxo_no.tex nxo_null.tex nxo_operator.tex nxo_pmark.tex
                   nxo_stack.tex nxo_string.tex nxo_thread.tex ql.tex qr.tex
                   qs.tex systemdict.tex thd.tex tsd.tex xep.tex;
  tex_srcs = [fromto % [srcdir]/lib/libonyx/doc/latex/% [tex_srcs]];
  tex_srcs = [tex_srcs] [objdir]/lib/libonyx/doc/latex/manual.tex;

  TEX_SRCS = [TEX_SRCS] [tex_srcs];
}

/* Libraries. */
LIBONYX_REV = 5;

LIBONYX_A = [objdir]/lib/libonyx/lib/libonyx.a;
#if [matches elf [abi]]
LIBONYX_SNAME = libonyx.so.[LIBONYX_REV];
LIBONYX_LNAME = libonyx.so;
LIBONYX_S = [objdir]/lib/libonyx/lib/[LIBONYX_SNAME];
#elif [matches macho [abi]]
LIBONYX_SNAME = libonyx.[LIBONYX_REV].dylib;
LIBONYX_LNAME = libonyx.dylib;
LIBONYX_S = [objdir]/lib/libonyx/lib/[LIBONYX_SNAME];
#else
LIBONYX_S = ;
#endif

LDFLAGS = [LDFLAGS] -L[objdir]/lib/libonyx/lib;
LIBS = [LIBS] -lonyx;
A_LIBS = [A_LIBS] [LIBONYX_A];
S_LIBS = [S_LIBS] [LIBONYX_S];
#if [matches elf [abi]]
S_LINKS = [S_LINKS] [objdir]/lib/libonyx/lib/libonyx.so;
#elif [matches macho [abi]]
S_LINKS = [S_LINKS] [objdir]/lib/libonyx/lib/libonyx.dylib;
#endif

/*
 * User cook'ables.
 */

BOOTSTRAP = [BOOTSTRAP] libonyx_bootstrap;

libonyx_bootstrap : [TONYX]
{
  cat [srcdir]/lib/libonyx/src/nx_nxcode.nx
    | [ONYX] [srcdir]/lib/libonyx/src/nx_nxcode_gen.nx
    > [srcdir]/lib/libonyx/src/nx_nxcode.c;
  cat [srcdir]/lib/libonyx/src/nxo_thread_nxcode.nx
    | [ONYX] [srcdir]/lib/libonyx/src/nxo_thread_nxcode_gen.nx
    > [srcdir]/lib/libonyx/src/nxo_thread_nxcode.c;
}

libonyx : [LIBONYX_A] [LIBONYX_S];

libonyx_install : libonyx_install_a libonyx_install_s;

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
  rm -f [objdir]/lib/libonyx/doc/latex/manual.tex;
}

/*
 * Various flags.
 */

CPPFLAGS = [CPPFLAGS] -I[srcdir]/lib/libonyx/include
                      -I[objdir]/lib/libonyx/include/libonyx;

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

libonyx_install_a : [LIBONYX_A] libonyx_install_common
{
  [INSTALL] -m 0444 [LIBONYX_A] [PREFIX]/lib;
}

libonyx_install_s : [LIBONYX_S] libonyx_install_common
{
#if [matches macho [abi]]
  /* Set install_name correctly before installing. */
  [CC] -undefined suppress -flat_namespace -dynamiclib
       -compatibility_version [LIBONYX_REV]
       -current_version [LIBONYX_REV]
       -install_name [PREFIX]/lib/libonyx.[LIBONYX_REV].dylib
       -o [LIBONYX_S] [fromto [srcdir]/%0%.c [objdir]/%0%.o_s [LIBONYX_SRCS]];
#endif

  [INSTALL] -m 0444 [LIBONYX_S] [PREFIX]/lib;

  rm -f [PREFIX]/lib/[LIBONYX_LNAME];
  [LN] -s [LIBONYX_SNAME] [PREFIX]/lib/[LIBONYX_LNAME];
}

/*
 * Dependencies.
 */

[LIBONYX_A] : [fromto [srcdir]/%0%.c [objdir]/%0%.o_a [LIBONYX_SRCS]]
  set mkdir
{
  [AR] cru [target] [younger];
  [RANLIB] [target];
}

[LIBONYX_S] : [fromto [srcdir]/%0%.c [objdir]/%0%.o_s [LIBONYX_SRCS]]
  set mkdir
{
  [CC]
#if [matches elf [abi]]
       -shared
#elif [matches macho [abi]]
       -undefined suppress -flat_namespace -dynamiclib
       -compatibility_version [LIBONYX_REV]
       -current_version [LIBONYX_REV]
       -install_name [abs_objdir]/[LIBONYX_S]
#endif
       -o [target] [need];

  rm -f [objdir]/lib/libonyx/lib/[LIBONYX_LNAME];
  [LN] -s [LIBONYX_SNAME] [objdir]/lib/libonyx/lib/[LIBONYX_LNAME];
}