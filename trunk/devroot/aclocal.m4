dnl Use threads by default.
AC_DEFUN(CW_ENABLE_THREADS,
[
AC_ARG_ENABLE(threads, [  --disable-threads       Disable threads],
if test "x$enable_threads" = "xyes" ; then
  enable_threads="1"
else
  enable_threads="0"
fi
,
enable_threads="1"
)
AC_SUBST(enable_threads)
if test "x$enable_threads" = "x1" ; then
  AC_DEFINE(_CW_THREADS)

  AC_CHECK_HEADERS(pthread.h, , AC_MSG_ERROR(Cannot build without pthread.h))

  AC_CHECK_LIB(pthread, pthread_create, LIBS="$LIBS -lpthread", \
    AC_CHECK_LIB(c_r, pthread_create, \
      LIBS="$LIBS -pthread", AC_MSG_ERROR(Cannot find the pthreads library)))
fi
])

dnl Support POSIX by default.
AC_DEFUN(CW_ENABLE_POSIX,
[
AC_ARG_ENABLE(posix, [  --disable-posix         Disable POSIX support],
if test "x$enable_posix" = "xyes" ; then
  enable_posix="1"
else
  enable_posix="0"
fi
,
enable_posix="1"
)
AC_SUBST(enable_posix)
if test "x$enable_posix" = "x1" ; then
  AC_DEFINE(_CW_POSIX)
fi
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

dnl Use libedit by default.
AC_DEFUN(CW_ENABLE_LIBEDIT,
[
AC_ARG_ENABLE(libedit, [  --disable-libedit       Do not use libedit],
if test "x$enable_libedit" = "xno" ; then
  enable_libedit="0"
else
  enable_libedit="1"
fi
,
if test "x$enable_threads" = "x1" -a "x$enable_posix" = "x1" ; then
  enable_libedit="1"
else
  enable_libedit="0"
fi
)
AC_SUBST(enable_libedit)
if test "x$enable_libedit" = "x1" ; then
  AC_DEFINE(_CW_USE_LIBEDIT)
fi
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
