dnl CW_REQUIRES(dir, opt config_flags, opt targets)
dnl dir : Relative path of package.
dnl config_opts : Command line options to pass to configure.
dnl targets : make targets.
dnl Require a self-contained package.
dnl 
AC_DEFUN(CW_REQUIRES,
[
  if test -d $1 ; then
    pwd=`pwd`

    echo "cd $1"
    cd $1

    echo "./configure $2"
    ./configure $2

    for i in $3 ; do
      echo "make $i"
      make $i
    done

    dnl Add elements to CFLAGS, CPPFLAGS, LDFLAGS, LD_LIBRARY_PATH, and PATH.

    echo "cd $pwd"
    cd $pwd
  else
    AC_MSG_ERROR(Cannot find required package in $1)
  fi
])

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
enable_shared="1"
)
AC_SUBST(enable_shared)
])
