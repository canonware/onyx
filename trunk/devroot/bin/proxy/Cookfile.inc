/******************************************************************************
 *
 * <Copyright = jasone>
 * <License>
 *
 ******************************************************************************
 *
 * Version: <Version>
 *
 * cook file for proxy.
 *
 ******************************************************************************/

/* Include generated dependency files. */
#include-cooked [glob [objdir]/bin/proxy/src/"*.d_*"]

/*
 * File lists.
 */

/* Include files that _are_ installed. */
{
  local incs = ;
  incs = [fromto % [srcdir]/bin/proxy/include/% [incs]];

  INCS = [INCS] [incs];
}

/* Include files that are _not_ installed. */
{
  local priv_incs = ;
  priv_incs = [fromto % [srcdir]/bin/proxy/include/% [priv_incs]];
  priv_incs = [priv_incs] [objdir]/bin/proxy/include/proxy_defs.h;

  PRIV_INCS = [PRIV_INCS] [priv_incs];
}

/* Source files. */
PROXY_SRCS = proxy.c;
PROXY_SRCS = [fromto % [srcdir]/bin/proxy/src/% [PROXY_SRCS]];
SRCS = [SRCS] [PROXY_SRCS];

/* Simple C tests.  Output is diff'ed with the expected (.exp file) output. */
{
  local ctests = ;
  ctests = [fromto % [srcdir]/bin/proxy/test/% [ctests]];

  CTESTS = [CTESTS] [ctests];
}

/*
 * Simple onyx tests.  Output is diff'ed with the expected (.exp file)
 * output.
 */
{
  local otests = ;
  otests = [fromto % [srcdir]/bin/proxy/test/% [otests]];

  OTESTS = [OTESTS] [otests];
}

/* Perl scripts that drive other tests and use the sequence mode of verify. */
{
  local ptests = ;
  ptests = [fromto % [srcdir]/bin/proxy/test/% [ptests]];

  PTESTS = [PTESTS] [ptests];
}

/* C programs that are driven by tests. */
{
  local cbtests = ;
  cbtests = [fromto % [srcdir]/bin/proxy/test/% [cbtests]];

  CBTESTS = [CBTESTS] [cbtests];
}

/* onyx programs that are driven by tests. */
{
  local obtests = ;
  obtests = [fromto % [srcdir]/bin/proxy/test/% [obtests]];

  OBTESTS = [OBTESTS] [obtests];
}

/* LaTeX source files. */
{
  local tex_srcs = ;
  tex_srcs = [fromto % [srcdir]/bin/proxy/doc/latex/% [tex_srcs]];
  tex_srcs = [tex_srcs] [objdir]/bin/proxy/doc/latex/manual.tex;

  TEX_SRCS = [TEX_SRCS] [tex_srcs];
}

/* xfig source files. */
{
  local fig_srcs = ;
  fig_srcs = [fromto % [srcdir]/bin/proxy/doc/latex/% [fig_srcs]];

  FIG_SRCS = [FIG_SRCS] [fig_srcs];
}

/* Binaries. */
PROXY = [objdir]/bin/proxy/bin/proxy;
BINS = [BINS] [PROXY];

/*
 * User cook'ables.
 */

proxy : [PROXY];

proxy_install_self : [PROXY]
{
  [INSTALL] -d [PREFIX]/bin;
  [CC] [A_CFLAGS] -o [PREFIX]/bin/proxy
       [fromto [srcdir]/%0%.c [objdir]/%0%.o_a [PROXY_SRCS]]
       [RPATH][PREFIX]/lib
       [LDFLAGS] -lsock -lstash [EXTRALIBS];
  [CHMOD] 0755 [PREFIX]/bin/proxy;
}

proxy_install : proxy_install_self libsock_install_s libstash_install_s;

proxy_uninstall :
{
  rm -f [PREFIX]/bin/proxy;
}

proxy_clean :
{
}

proxy_distclean :
{
  rm -f [objdir]/bin/proxy/doc/latex/manual.tex;
}

/*
 * Dependencies.
 */

[PROXY] : [fromto [srcdir]/%0%.c [objdir]/%0%.o_a [PROXY_SRCS]]
          [LIBSOCK_S] [LIBSTASH_S]
  set mkdir
{
  [CC] [A_CFLAGS] -o [target]
       [fromto [srcdir]/%0%.c [objdir]/%0%.o_a [PROXY_SRCS]]
       [RPATH][abs_objdir]/lib/c/libsock/lib
       [RPATH][abs_objdir]/lib/c/libstash/lib
       [LDFLAGS] -lsock -lstash [EXTRALIBS];
}
