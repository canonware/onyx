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
AC_ARG_ENABLE(shared, [  --disable-shared        Do not build shared libraries],
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

dnl CW_BUILD_LIB(lib, var)
dnl lib : Name of library.
dnl var : Name of variable to substitute in configure output.
AC_DEFUN(CW_BUILD_LIB,
[
AC_MSG_CHECKING(include $1 in build)
if test -d "$srcdir/lib/c/$1" ; then
  build_$1="yes"
  $2=1
  cfghdrs="$cfghdrs $objdir/lib/c/$1/include/$1/$1_defs.h"
  targets="$targets $1"
  mkdir -p $objdir/lib/c/$1/include/$1
else
  build_$1="no"
  $2=0
fi
AC_MSG_RESULT($build_$1)
AC_SUBST($2)
])

dnl CW_BUILD_BIN(bin, var)
dnl bin : Name of binary.
dnl var : Name of variable to substitute in configure output.
AC_DEFUN(CW_BUILD_BIN,
[
AC_MSG_CHECKING(include $1 in build)
if test -d "$srcdir/bin/$1" ; then
  build_$1="yes"
  $2=1
  cfghdrs="$cfghdrs $objdir/bin/$1/include/$1_defs.h"
  targets="$targets $1"
  mkdir -p $objdir/bin/$1/include
else
  build_$1="no"
  $2=0
fi
AC_MSG_RESULT($build_$1)
AC_SUBST($2)
])
