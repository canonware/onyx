/******************************************************************************
 *
 * <Copyright = jasone>
 * <License>
 *
 ******************************************************************************
 *
 * Version: <Version>
 *
 * cook file for libstash.
 *
 ******************************************************************************/

/* Include generated dependency files. */
#include-cooked [glob [objdir]/lib/c/libstash/src/"*.d_*"]
#include-cooked [glob [objdir]/lib/c/libstash/test/"*.d_*"]

/*
 * File lists.
 */

/* Include files that _are_ installed. */
{
  local incs = libstash.h libstash_incs.h bhp.h
#if [HAVE_LIBSTASH_BUF]
               buf.h
#endif
               ch.h cnd.h dch.h mem.h mq.h mtx.h out.h pezz.h pool.h ql.h qr.h
               qs.h rwl.h sma.h thd.h tsd.h xep.h;
  incs = [fromto % [srcdir]/lib/c/libstash/include/libstash/% [incs]];
  incs = [incs] [objdir]/lib/c/libstash/include/libstash/libstash_defs.h;

  LIBSTASH_INCS = [incs];
  INCS = [INCS] [incs];
}

/* Include files that are _not_ installed. */
{
  local priv_incs = ;
  priv_incs = [fromto % [srcdir]/lib/c/libstash/include/libstash/% [priv_incs]];

  PRIV_INCS = [PRIV_INCS] [priv_incs];
}

/* Source files. */
LIBSTASH_SRCS = libstash.c bhp.c
#if [HAVE_LIBSTASH_BUF]
                buf.c
#endif
                ch.c cnd.c dch.c mem.c mq.c mtx.c out.c pezz.c pool.c rwl.c
                sma.c thd.c tsd.c xep.c;

LIBSTASH_SRCS = [fromto % [srcdir]/lib/c/libstash/src/% [LIBSTASH_SRCS]];
SRCS = [SRCS] [LIBSTASH_SRCS];

/* Simple C tests.  Output is diff'ed with the expected (.exp file) output. */
{
  local ctests = c_bhp_a.c c_bhp_b.c
#if [HAVE_LIBSTASH_BUF]
                 c_buf_a.c c_buf_b.c c_buf_c.c c_buf_d.c c_buf_e.c
#endif
                 c_ch_a.c c_cnd_a.c c_dch_a.c c_libstash_a.c c_mq_a.c c_mtx_a.c
                 c_out_a.c c_pezz_a.c c_pool_a.c c_ql_a.c c_qr_a.c c_qs_a.c
                 c_rwl_a.c sma_a.c c_thd_a.c c_thd_b.c c_thd_c.c c_tsd_a.c
                 c_xep_a.c;
  ctests = [fromto % [srcdir]/lib/c/libstash/test/% [ctests]];

  CTESTS = [CTESTS] [ctests];
}

/*
 * Simple onyx tests.  Output is diff'ed with the expected (.exp file)
 * output.
 */
{
  local otests = ;
  otests = [fromto % [srcdir]/lib/c/libstash/test/% [otests]];

  OTESTS = [OTESTS] [otests];
}

/* Perl scripts that drive other tests and use the sequence mode of verify. */
{
  local ptests = ;
  ptests = [fromto % [srcdir]/lib/c/libstash/test/% [ptests]];

  PTESTS = [PTESTS] [ptests];
}

/* C programs that are driven by tests. */
{
  local cbtests = ;
  cbtests = [fromto % [srcdir]/lib/c/libstash/test/% [cbtests]];

  CBTESTS = [CBTESTS] [cbtests];
}

/* onyx programs that are driven by tests. */
{
  local obtests = ;
  obtests = [fromto % [srcdir]/lib/c/libstash/test/% [obtests]];

  OBTESTS = [OBTESTS] [obtests];
}

/* LaTeX source files. */
{
  local tex_srcs = bhp.tex buf.tex ch.tex cnd.tex dch.tex mem.tex mq.tex mtx.tex
                   out.tex pezz.tex pool.tex ql.tex qr.tex qs.tex
                   rwl.tex sma.tex thd.tex tsd.tex xep.tex;
  tex_srcs = [fromto % [srcdir]/lib/c/libstash/doc/latex/% [tex_srcs]];
  tex_srcs = [tex_srcs] [objdir]/lib/c/libstash/doc/latex/manual.tex;

  TEX_SRCS = [TEX_SRCS] [tex_srcs];
}

/* xfig source files. */
{
  local fig_srcs = ;
  fig_srcs = [fromto % [srcdir]/lib/c/libstash/doc/latex/% [fig_srcs]];

  FIG_SRCS = [FIG_SRCS] [fig_srcs];
}

/* Libraries. */
LIBSTASH_REV = 1.0;

LIBSTASH_A = [objdir]/lib/c/libstash/lib/libstash.a;
LIBSTASH_S = [objdir]/lib/c/libstash/lib/libstash.so.[LIBSTASH_REV];

LDFLAGS = [LDFLAGS] -L[objdir]/lib/c/libstash/lib;
LIBS = [LIBS] -lstash;
A_LIBS = [A_LIBS] [LIBSTASH_A];
S_LIBS = [S_LIBS] [LIBSTASH_S];
S_LINKS = [S_LINKS] [objdir]/lib/c/libstash/lib/libstash.so;

/*
 * User cook'ables.
 */

libstash : [LIBSTASH_A] [LIBSTASH_S];

libstash_install : libstash_install_a libstash_install_s;

libstash_uninstall :
{
  [SHELL] -e;
data
#![SHELL]

echo rm -f [PREFIX]/lib/libstash*
rm -f [PREFIX]/lib/libstash*

echo rm -rf [PREFIX]/include/libstash
rm -rf [PREFIX]/include/libstash
dataend
}

libstash_clean :
{
}

libstash_distclean :
{
  rm -f [objdir]/lib/c/libstash/doc/latex/manual.tex;
}

/*
 * Various flags.
 */

CPPFLAGS = [CPPFLAGS] -I[srcdir]/lib/c/libstash/include
                      -I[objdir]/lib/c/libstash/include/libstash;

/*
 * Build rules.
 */

libstash_install_common :
{
  [SHELL] -e;
data
#![SHELL]

echo [INSTALL] -d [PREFIX]/include/libstash
[INSTALL] -d [PREFIX]/include/libstash

for i in [LIBSTASH_INCS]; do
	echo [INSTALL] -m 0644 $i [PREFIX]/include/libstash/
	[INSTALL] -m 0644 $i [PREFIX]/include/libstash/
done

echo [INSTALL] -d [PREFIX]/lib
[INSTALL] -d [PREFIX]/lib
dataend
}

libstash_install_a : [LIBSTASH_A] libstash_install_common
{
  [INSTALL] -m 0444 [LIBSTASH_A] [PREFIX]/lib;
}

libstash_install_s : [LIBSTASH_S] libstash_install_common
{
  [INSTALL] -m 0444 [LIBSTASH_S] [PREFIX]/lib;
  rm -f [PREFIX]/lib/libstash.so;
  [LN] -s libstash.so.[LIBSTASH_REV] [PREFIX]/lib/libstash.so;
}

/*
 * Dependencies.
 */

[LIBSTASH_A] : [fromto [srcdir]/%0%.c [objdir]/%0%.o_a [LIBSTASH_SRCS]]
  set mkdir
{
  [AR] cru [target] [younger];
  [RANLIB] [target];
}

[LIBSTASH_S] : [fromto [srcdir]/%0%.c [objdir]/%0%.o_s [LIBSTASH_SRCS]]
  set mkdir
{
  [CC] -shared -o [target] [need];
  rm -f [objdir]/lib/c/libstash/lib/libstash.so;
  [LN] -s libstash.so.[LIBSTASH_REV] [objdir]/lib/c/libstash/lib/libstash.so;
}
