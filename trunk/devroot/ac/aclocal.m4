dnl Must be defined, since each directory's aclocal.m4 looks for it, and exits
dnl if not found.  This hack is necessary since autoconf undefines 'include',
dnl leaving only 'sinclude', which doesn't cause an error if the file to be
dnl included can't be found.
AC_DEFUN(SQRL_INIT,[])

dnl Use pthreads.
AC_DEFUN(SQRL_USE_PTHREADS,
[
  AC_CHECK_HEADERS(pthread.h, , AC_MSG_ERROR(Cannot build without pthread.h))

  AC_CHECK_LIB(pthread, pthread_create, LIBS="$LIBS -lpthread", \
    AC_CHECK_LIB(c_r, pthread_create, \
      LIBS="$LIBS -pthread", AC_MSG_ERROR(Cannot find the pthreads library)))
])

dnl SQRL_USE_LIBSTASH_R(libsuffix, libvar, relpath)
dnl libsuffix : _d, _p.
dnl libvar : Name of variable to append -lstash<libsuffix> to.
dnl relpath : Relative path to libstash's parent dir if in the same build tree.
AC_DEFUN(SQRL_USE_LIBSTASH_R,
[
  AC_CHECK_HEADERS(libstash/libstash_r.h, , \
                   AC_MSG_ERROR(Cannot build without libstash/libstash_r.h))

  AC_CHECK_FILE([$3/libstash],$2="-static -lstash_r$1",
    echo Was hoping for $ac_config_dir/lib/libstash
    AC_CHECK_LIB(stash$1, main, $2="-static -lstash_r$1", \
      AC_MSG_ERROR(Cannot find the stash_r$1 library)))
])

AC_DEFUN(SQRL_SET_MAKEFILE,
[
AC_ARG_WITH(gnu-make, [  --with-gnu-make         Always generate a Makefile compatiple with gnu make],
if test "x$with_gnu_make" != "xno" ; then
  use_gmake="yes"
else
  use_gmake="maybe"
fi
,
use_gmake="maybe"
)

dnl This is kinda ugly, since it means that AC_CANONICAL_HOST may be called
dnl twice.
AC_CANONICAL_HOST
case "${host}" in
  i386-*-freebsd*)
	AC_DEFINE(_CW_OS_FREEBSD, )
	if test "x$use_gmake" = "xmaybe" ; then
	  use_gmake="no"
	fi
        ;;
  *-*-linux*)
	AC_DEFINE(_CW_OS_LINUX, )
	if test "x$use_gmake" = "xmaybe" ; then
	  use_gmake="yes"
	fi
        ;;
  *-*-solaris2*)
	AC_DEFINE(_CW_OS_SOLARIS, )
	if test "x$use_gmake" = "xmaybe" ; then
	  use_gmake="yes"
	fi
        ;;
esac

echo rm -f Makefile
rm -f Makefile
if test "x$use_gmake" = "xyes" ; then
  echo "ln -s Makefile.gnu Makefile"
  ln -s Makefile.gnu Makefile
else
  echo "ln -s Makefile.bsd Makefile"
  ln -s Makefile.bsd Makefile
fi
])