dnl Use pthreads.
AC_DEFUN(CW_USE_PTHREADS,
[
  AC_CHECK_HEADERS(pthread.h, , AC_MSG_ERROR(Cannot build without pthread.h))

  AC_CHECK_LIB(pthread, pthread_create, LIBS="$LIBS -lpthread", \
    AC_CHECK_LIB(c_r, pthread_create, \
      LIBS="$LIBS -pthread", AC_MSG_ERROR(Cannot find the pthreads library)))
])

dnl Do not compile with debugging by default.
AC_DEFUN(CW_DISABLE_DEBUG,
[
AC_ARG_ENABLE(debug, [  --enable-debug          Build debugging code],
if test "x$enable_debug" = "xno" ; then
  enable_debug="0"
else
  enable_debug="1"
fi
,
enable_debug="0"
)
if test "x$enable_debug" = "x1" ; then
  AC_DEFINE(_CW_DBG)
  AC_DEFINE(_CW_ASSERT)
fi
])

dnl Do not compile with profiling by default.
AC_DEFUN(CW_DISABLE_PROFILE,
[
AC_ARG_ENABLE(profile, [  --enable-profile        Build with profiling],
if test "x$enable_profile" = "xno" ; then
  enable_profile="0"
else
  enable_profile="1"
fi
,
enable_profile="0"
)
])

dnl Use inline functions by default.
AC_DEFUN(CW_ENABLE_INLINES,
[
AC_ARG_ENABLE(inlines, [  --disable-inlines       Do not use inline functions],
if test "x$enable_inlines" = "xno" ; then
  enable_inlines="0"
else
  enable_inlines="1"
fi
,
enable_inlines="1"
)
if test "x$enable_inlines" = "x1" ; then
  AC_DEFINE(_CW_USE_INLINES)
fi
])

dnl Compile libstash's buf class by default.
AC_DEFUN(CW_ENABLE_LIBSTASH_BUF,
[
AC_MSG_CHECKING(whether to include libstash's buf class in build)
AC_ARG_ENABLE(libstash-buf, [  --disable-libstash-buf  Do not compile in libstash's buf class],
if test -d "$srcdir/lib/c/$1" -a "x$enable_libstash_buf" != "xno" ; then
  enable_libstash_buf="1"
  AC_MSG_RESULT("yes")
else
  enable_libstash_buf="0"
  AC_MSG_RESULT("no")
fi
,
enable_libstash_buf="1"
AC_MSG_RESULT("yes")
)
if test "x$enable_libstash_buf" = "x1" ; then
  AC_DEFINE(_CW_HAVE_LIBSTASH_BUF)
fi
AC_SUBST(enable_libstash_buf)
])

dnl CW_BUILD_LIB(lib, var)
dnl lib : Name of library.
dnl var : Name of variable to substitute in configure output.
AC_DEFUN(CW_BUILD_LIB,
[
AC_MSG_CHECKING(whether to include $1 in build)
if test -d "$srcdir/lib/c/$1" ; then
  build_$1="yes"
  $2=1
  $2_manual=""
  if test -f "$srcdir/lib/c/$1/doc/latex/manual.tex.in" ; then
    cfgoutputs="$cfgoutputs lib/c/$1/doc/latex/manual.tex"
  fi
  cfghdrs="$cfghdrs $objdir/lib/c/$1/include/$1/$1_defs.h"
  libs="$libs $1"
  mkdir -p $objdir/lib/c/$1/include/$1
  mkdir -p $objdir/lib/c/$1/doc/latex
else
  build_$1="no"
  $2=0
  $2_manual="%"
fi
AC_MSG_RESULT($build_$1)
AC_SUBST($2)
AC_SUBST($2_manual)
])

dnl CW_BUILD_MOD(mod, var)
dnl mod : Name of module.
dnl var : Name of variable to substitute in configure output.
AC_DEFUN(CW_BUILD_MOD,
[
AC_MSG_CHECKING(whether to include $1 in build)
if test -d "$srcdir/mod/$1" ; then
  build_$1="yes"
  $2=1
  $2_manual=""
  if test -f "$srcdir/mod/$1/doc/latex/manual.tex.in" ; then
    cfgoutputs="$cfgoutputs mod/$1/doc/latex/manual.tex"
  fi
  cfghdrs="$cfghdrs $objdir/mod/$1/include/$1_defs.h"
  mods="$mods $1"
  mkdir -p $objdir/mod/$1/include
  mkdir -p $objdir/mod/$1/doc/latex
else
  build_$1="no"
  $2=0
  $2_manual="%"
fi
AC_MSG_RESULT($build_$1)
AC_SUBST($2)
AC_SUBST($2_manual)
])

dnl CW_BUILD_BIN(bin, var)
dnl bin : Name of binary.
dnl var : Name of variable to substitute in configure output.
AC_DEFUN(CW_BUILD_BIN,
[
AC_MSG_CHECKING(whether to include $1 in build)
if test -d "$srcdir/bin/$1" ; then
  build_$1="yes"
  $2=1
  $2_manual=""
  if test -f "$srcdir/bin/$1/doc/latex/manual.tex.in" ; then
    cfgoutputs="$cfgoutputs bin/$1/doc/latex/manual.tex"
  fi
  cfghdrs="$cfghdrs $objdir/bin/$1/include/$1_defs.h"
  bins="$bins $1"
  mkdir -p $objdir/bin/$1/include
  mkdir -p $objdir/bin/$1/doc/latex
else
  build_$1="no"
  $2=0
  $2_manual="%"
fi
AC_MSG_RESULT($build_$1)
AC_SUBST($2)
AC_SUBST($2_manual)
])
