/******************************************************************************
 *
 * <Copyright = "jasone">
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
#include-cooked [glob [objdir]/lib/c/libstash/bin/"*.d_*"]
#include-cooked [glob [objdir]/lib/c/libstash/test/"*.d_*"]

/*
 * File lists.
 */

/* Include files that _are_ installed. */
{
  local incs = libstash.h libstash_incs.h bhp.h buf.h ch.h cnd.h dch.h dbg.h
               mem.h mq.h mtx.h out.h pezz.h pool.h ql.h qr.h qs.h res.h rwl.h
               sema.h thd.h treen.h tsd.h;
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
LIBSTASH_SRCS = libstash.c bhp.c buf.c ch.c cnd.c dch.c dbg.c mem.c mq.c mtx.c
                out.c pezz.c pool.c res.c rwl.c sema.c thd.c treen.c tsd.c;
LIBSTASH_SRCS = [fromto % [srcdir]/lib/c/libstash/src/% [LIBSTASH_SRCS]];
SRCS = [SRCS] [LIBSTASH_SRCS];

/* Simple tests.  Output is diff'ed with the expected (.exp file) output. */
{
  local ctests = c_bhp_a.c c_bhp_b.c c_buf_a.c c_buf_b.c c_buf_c.c c_buf_d.c
                 c_buf_e.c c_ch_a.c c_cnd_a.c c_dbg_a.c c_dch_a.c
                 c_libstash_a.c c_mq_a.c c_mtx_a.c c_out_a.c c_pezz_a.c
                 c_pool_a.c c_ql_a.c c_qr_a.c c_qs_a.c c_res_a.c c_rwl_a.c
                 c_sema_a.c c_thd_a.c c_thd_b.c c_treen_a.c c_tsd_a.c;
  ctests = [fromto % [srcdir]/lib/c/libstash/test/% [ctests]];

  CTESTS = [CTESTS] [ctests];
}

/* Perl scripts that drive the BTESTS. */
{
  local ptests = p_res_a.pl.in;
  ptests = [fromto % [srcdir]/lib/c/libstash/test/% [ptests]];

  PTESTS = [PTESTS] [ptests];
}

/* Backend test programs that are driven by the PTESTS. */
{
  local btests = b_res_a.c;
  btests = [fromto % [srcdir]/lib/c/libstash/test/% [btests]];

  BTESTS = [BTESTS] [btests];
}

/* Man pages. */
{
  local manpages_3 = libstash.3s libstash_bhp.3s libstash_buf.3s libstash_ch.3s
                     libstash_cnd.3s libstash_dch.3s libstash_dbg.3s
                     libstash_mem.3s libstash_mq.3s libstash_mtx.3s
                     libstash_out.3s libstash_pezz.3s libstash_pool.3s
                     libstash_ql.3s libstash_qr.3s libstash_qs.3s
                     libstash_res.3s libstash_rwl.3s libstash_sema.3s
                     libstash_thd.3s libstash_tsd.3s libstash_treen.3s;
  manpages_3 = [fromto % [srcdir]/lib/c/libstash/man/man3/% [manpages_3]];

  LIBSTASH_MANPAGES_3 = [manpages_3];
  MANPAGES_3 = [MANPAGES_3] [manpages_3];
}

/* Libraries. */
LIBSTASH_REV = 0.0;

LIBSTASH_D = [objdir]/lib/c/libstash/lib/libstash_d.a;
LIBSTASH_A = [objdir]/lib/c/libstash/lib/libstash.a;
LIBSTASH_S = [objdir]/lib/c/libstash/lib/libstash.so.[LIBSTASH_REV];
LIBSTASH_P = [objdir]/lib/c/libstash/lib/libstash_p.a;

LDFLAGS = [LDFLAGS] -L[objdir]/lib/c/libstash/lib;
LIBS = [LIBS] -lstash;
D_LIBS = [D_LIBS] [LIBSTASH_D];
A_LIBS = [A_LIBS] [LIBSTASH_A];
P_LIBS = [P_LIBS] [LIBSTASH_P];
S_LIBS = [S_LIBS] [LIBSTASH_S];
S_LINKS = [S_LINKS] [objdir]/lib/c/libstash/lib/libstash.so;

/*
 * User cook'ables.
 */

libstash : [LIBSTASH_D] [LIBSTASH_A] [LIBSTASH_P]
#if [ENABLE_SHARED]
           [LIBSTASH_S]
#endif
;

libstash_install : libstash_install_d libstash_install_a libstash_install_p
#if [ENABLE_SHARED]
                   libstash_install_s
#endif
;

libstash_uninstall :
{
  [SHELL] -e;
data
#![SHELL]

echo rm -f [PREFIX]/lib/libstash*
rm -f [PREFIX]/lib/libstash*

echo rm -rf [PREFIX]/include/libstash
rm -rf [PREFIX]/include/libstash

for i in [LIBSTASH_MANPAGES_3]; do
	echo rm -f [PREFIX]/man/man3/`[BASENAME] $i`
	rm -f [PREFIX]/man/man3/`[BASENAME] $i`
done
dataend
}

libstash_clean :
{
}

libstash_distclean :
{
}

/*
 * Various flags.
 */

CPPFLAGS = [CPPFLAGS] -I[srcdir]/lib/c/libstash/include
                      -I[objdir]/lib/c/libstash/include/libstash;
D_CPPFLAGS = [D_CPPFLAGS] -D_LIBSTASH_DBG;

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

echo [INSTALL] -d [PREFIX]/man/man3
[INSTALL] -d [PREFIX]/man/man3

for i in [LIBSTASH_MANPAGES_3]; do
	echo [INSTALL] -m 0644 $i [PREFIX]/man/man3/
	[INSTALL] -m 0644 $i [PREFIX]/man/man3/
done

echo [INSTALL] -d [PREFIX]/lib
[INSTALL] -d [PREFIX]/lib
dataend
}

libstash_install_d : [LIBSTASH_D] libstash_install_common
{
	[INSTALL] -m 0444 [LIBSTASH_D] [PREFIX]/lib;
}

libstash_install_a : [LIBSTASH_A] libstash_install_common
{
	[INSTALL] -m 0444 [LIBSTASH_A] [PREFIX]/lib;
}

libstash_install_p : [LIBSTASH_P] libstash_install_common
{
	[INSTALL] -m 0444 [LIBSTASH_P] [PREFIX]/lib;
}

libstash_install_s : [LIBSTASH_D] libstash_install_common
{
	[INSTALL] -m 0444 [LIBSTASH_S] [PREFIX]/lib;
	[LN] -sf libstash.so.[LIBSTASH_REV] [PREFIX]/lib/libstash.so;
}

/*
 * Dependencies.
 */

[LIBSTASH_D] : [fromto [srcdir]/%0%.c [objdir]/%0%.o_d [LIBSTASH_SRCS]]
  set mkdir
{
  [AR] cru [target] [younger];
  [RANLIB] [target];
}

[LIBSTASH_A] : [fromto [srcdir]/%0%.c [objdir]/%0%.o_a [LIBSTASH_SRCS]]
  set mkdir
{
  [AR] cru [target] [younger];
  [RANLIB] [target];
}

[LIBSTASH_P] : [fromto [srcdir]/%0%.c [objdir]/%0%.o_p [LIBSTASH_SRCS]]
  set mkdir
{
  [AR] cru [target] [younger];
  [RANLIB] [target];
}

[LIBSTASH_S] : [fromto [srcdir]/%0%.c [objdir]/%0%.o_s [LIBSTASH_SRCS]]
  set mkdir
{
  [CC] -shared -o [target] [need];
  [LN] -sf libstash.so.[LIBSTASH_REV] [objdir]/lib/c/libstash/lib/libstash.so;
}
