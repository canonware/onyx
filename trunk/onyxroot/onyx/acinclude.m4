dnl Use pthreads.
AC_DEFUN(CW_USE_PTHREADS,
[
  AC_CHECK_HEADERS(pthread.h, , AC_MSG_ERROR(Cannot build without pthread.h))

  AC_CHECK_LIB(pthread, pthread_create, LIBS="$LIBS -lpthread", \
    AC_CHECK_LIB(c_r, pthread_create, \
      LIBS="$LIBS -pthread", AC_MSG_ERROR(Cannot find the pthreads library)))
])

AC_DEFUN(CW_DISABLE_SHARED,
[
dnl Comment out target dependencies on shared libraries if --disable-shared
dnl is defined.
AC_ARG_ENABLE(shared, [  --enable-shared         Build shared libraries],
if test "x$enable_shared" = "xno" ; then
  enable_shared="0"
else
  enable_shared="1"
fi
,
enable_shared="0"
)
AC_SUBST(enable_shared)
])

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
