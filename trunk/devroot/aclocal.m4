dnl aclocal.m4 generated automatically by aclocal 1.4-p1

dnl Copyright (C) 1994, 1995-8, 1999 Free Software Foundation, Inc.
dnl This file is free software; the Free Software Foundation
dnl gives unlimited permission to copy and/or distribute it,
dnl with or without modifications, as long as this notice is preserved.

dnl This program is distributed in the hope that it will be useful,
dnl but WITHOUT ANY WARRANTY, to the extent permitted by law; without
dnl even the implied warranty of MERCHANTABILITY or FITNESS FOR A
dnl PARTICULAR PURPOSE.

dnl Do not build documentation by default.
AC_DEFUN(CW_WITHOUT_DOCS,
[
AC_ARG_WITH(docs, [  --with-docs             Build documentation],
if test "x$with_docs" = "xno" ; then
  with_docs="0"
else
  with_docs="1"
fi
,
with_docs="0"
)
if test "x$with_docs" = "x1" ; then
  AC_PATH_PROG(LATEX, latex, , $PATH)
  AC_PATH_PROG(PDFLATEX, pdflatex, , $PATH)
  AC_PATH_PROG(DVIPS, dvips, , $PATH)
  AC_PATH_PROG(FIG2DEV, fig2dev, , $PATH)
  AC_PATH_PROG(LATEX2HTML, latex2html, , $PATH)
  AC_PATH_PROG(MAKEINDEX, makeindex, , $PATH)

  if test "x$LATEX" = "x" -o "x$PDFLATEX" = "x" -o "x$DVIPS" = "x" \
     -o "x$FIG2DEV" = "x" -o "x$LATEX2HTML" = "x" -o "x$MAKEINDEX" = "x" ; then
    AC_MSG_ERROR(Missing one or more programs necessary for documentation build)
  fi

fi
AC_SUBST(with_docs)
])

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
if test "x$enable_threads" = "x1" ; then
  dnl Look for Mach threads.
  have_mthreads="1"
  AC_CHECK_HEADERS(mach/task.h, , have_mthreads="0")
  AC_CHECK_FUNCS(mach_thread_self, , have_mthreads="0")
  if test "x$have_mthreads" = "x1" ; then
    AC_DEFINE(CW_MTHREADS)
  fi

  dnl Look for Solaris threads.
  have_sthreads="1"
  AC_CHECK_HEADERS(thread.h, , have_sthreads="0")
  AC_CHECK_LIB(thread, thr_suspend, , have_sthreads="0")
  AC_CHECK_LIB(thread, thr_continue, , have_sthreads="0")
  if test "x$have_sthreads" = "x1" ; then
    LIBS="$LIBS -lthread"
    AC_DEFINE(CW_STHREADS)
  fi

  dnl Look for pthreads.
  AC_CHECK_HEADERS(pthread.h, , enable_threads="0")
  AC_CHECK_LIB(pthread, pthread_create, LIBS="$LIBS -lpthread", \
    AC_CHECK_LIB(c_r, pthread_create, \
      LIBS="$LIBS -pthread", enable_threads="0"))
  if test "x$enable_threads" = "x1" ; then
    AC_DEFINE(CW_THREADS)
    AC_DEFINE(CW_PTHREADS)
  fi

  dnl Look for FreeBSD's non-portable suspend/resume API (libc_r).
  have_fthreads="1"
  AC_CHECK_FUNCS(pthread_suspend_np, , have_fthreads="0")
  AC_CHECK_FUNCS(pthread_resume_np, , have_fthreads="0")
  if test "x$have_fthreads" = "x1" ; then
    AC_DEFINE(CW_FTHREADS)
  fi
fi
AC_SUBST(enable_threads)
])

dnl Support the real type in Onyx by default.
AC_DEFUN(CW_ENABLE_REAL,
[
AC_ARG_ENABLE(real, [  --disable-real          Disable real number support],

if test "x$enable_real" = "xyes" ; then
  enable_real="1"
else
  enable_real="0"
fi
,
enable_real="1"
)
AC_SUBST(enable_real)
if test "x$enable_real" = "x1" ; then
  AC_DEFINE(CW_REAL)
  LIBS="$LIBS -lm"
fi
])

dnl Support POSIX file operations by default.
AC_DEFUN(CW_ENABLE_POSIX_FILE,
[
AC_ARG_ENABLE(posix-file, [  --disable-posix-file    Disable POSIX file support],
if test "x$enable_posix_file" = "xyes" ; then
  enable_posix_file="1"
else
  enable_posix_file="0"
fi
,
enable_posix_file="1"
)
AC_SUBST(enable_posix_file)
if test "x$enable_posix_file" = "x1" ; then
  AC_DEFINE(CW_POSIX_FILE)
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
dnl posix depends on posix-file.
if test "x$enable_posix_file" = "x0" ; then
  enable_posix="0"
fi

AC_SUBST(enable_posix)
if test "x$enable_posix" = "x1" ; then
  AC_DEFINE(CW_POSIX)
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
  AC_DEFINE(CW_DBG)
  AC_DEFINE(CW_ASSERT)
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
  AC_DEFINE(CW_USE_INLINES)
fi
])

dnl Build libonyx by default.  At a minimum, find an existing installation.
AC_DEFUN(CW_ENABLE_LIBONYX,
[
AC_ARG_ENABLE(libonyx, [  --disable-libonyx       Do not build libonyx],
if test "x$enable_libonyx" = "xno" ; then
  enable_libonyx="0"
else
  enable_libonyx="1"
fi
,
enable_libonyx="1"
)
if test "x$enable_libonyx" = "x0" ; then
  AC_CHECK_HEADERS(libonyx/libonyx.h, , \
    AC_MSG_ERROR(Cannot find libonyx/libonyx.h))
  AC_CHECK_LIB(onyx, libonyx_init, LIBS="$LIBS", \
    AC_MSG_ERROR(Cannot find libonyx))
fi

AC_SUBST(enable_libonyx)
if test "x$enable_libonyx" = "x1" ; then
  AC_DEFINE(CW_USE_LIBONYX)
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
enable_libedit="1"
)
AC_CHECK_HEADERS(curses.h, , enable_libedit="0")
AC_CHECK_HEADERS(term.h, , enable_libedit="0")
AC_SEARCH_LIBS(tigetflag, ncurses curses, , enable_libedit="0")
AC_SEARCH_LIBS(tigetnum, ncursese curses, , enable_libedit="0")
AC_SEARCH_LIBS(tigetstr, ncurses curses, , enable_libedit="0")

AC_SUBST(enable_libedit)
if test "x$enable_libedit" = "x1" ; then
  AC_DEFINE(CW_USE_LIBEDIT)
fi
])

dnl Build onyx by default.
AC_DEFUN(CW_ENABLE_ONYX,
[
AC_ARG_ENABLE(onyx, [  --disable-onyx          Do not build onyx],
if test "x$enable_onyx" = "xno" ; then
  enable_onyx="0"
else
  enable_onyx="1"
fi
,
enable_onyx="1"
)
if test "x$enable_onyx" = "x0" ; then
  AC_PATH_PROG(ONYX, onyx, , $PATH)
  if test "x$ONYX" = "x" ; then
    AC_MSG_ERROR(Cannot find onyx)
  fi
else
  ONYX="$abs_objroot/bin/onyx/bin/tonyx"
  AC_SUBST(ONYX)
fi

if test `echo "<Version = onyx>" | grep Version` ; then
  onyx_version="<Version = onyx>"
else
  onyx_version=devel
fi
AC_SUBST(onyx_version)

AC_SUBST(enable_onyx)
if test "x$enable_onyx" = "x1" ; then
  AC_DEFINE(CW_USE_ONYX)
fi
])

dnl Build slate by default.
AC_DEFUN(CW_ENABLE_SLATE,
[
AC_ARG_ENABLE(slate, [  --disable-slate         Do not build slate],
if test "x$enable_slate" = "xno" ; then
  enable_slate="0"
else
  enable_slate="1"
fi
,
enable_slate="1"
)

if test `echo "<Version = slate>" | grep Version` ; then
  slate_version="<Version = slate>"
else
  slate_version=devel
fi
AC_SUBST(slate_version)

AC_SUBST(enable_slate)
if test "x$enable_slate" = "x1" ; then
  AC_DEFINE(CW_USE_SLATE)
fi
])

dnl CW_BUILD_LIB(lib, var)
dnl lib : Name of library.
dnl var : Name of variable to substitute in configure output.
AC_DEFUN(CW_BUILD_LIB,
[
AC_MSG_CHECKING(whether to include $1 in build)
if test -d "$srcroot/lib/$1" ; then
  build_$1="yes"
  $2=1
  cfgoutputs="$cfgoutputs lib/$1/Cookfile.inc"
  if test -f "$srcroot/lib/$1/doc/latex/manual.tex.in" ; then
    mkdir -p "$objroot/lib/$1/doc/latex"
    cfgoutputs="$cfgoutputs lib/$1/doc/latex/manual.tex"
  fi
  mkdir -p "$objroot/lib/$1/include/$1"
  cfghdrs="$cfghdrs $objroot/lib/$1/include/$1/$1_defs.h"
  libs="$libs $1"
else
  build_$1="no"
  $2=0
fi
AC_MSG_RESULT($build_$1)
AC_SUBST($2)
])

dnl CW_BUILD_MOD(mod, var)
dnl mod : Name of module.
dnl var : Name of variable to substitute in configure output.
AC_DEFUN(CW_BUILD_MOD,
[
AC_MSG_CHECKING(whether to include $1 in build)
if test -d "$srcroot/mod/$1" ; then
  build_$1="yes"
  $2=1
  cfgoutputs="$cfgoutputs mod/$1/Cookfile.inc"
  if test -f "$srcroot/mod/$1/doc/latex/manual.tex.in" ; then
    mkdir -p "$objroot/mod/$1/doc/latex"
    cfgoutputs="$cfgoutputs mod/$1/doc/latex/manual.tex"
  fi
  mkdir -p "$objroot/mod/$1/include"
  cfghdrs="$cfghdrs $objroot/mod/$1/include/$1_defs.h"
  mods="$mods $1"
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
AC_MSG_CHECKING(whether to include $1 in build)
if test -d "$srcroot/bin/$1" ; then
  build_$1="yes"
  $2=1
  cfgoutputs="$cfgoutputs bin/$1/Cookfile.inc"
  if test -f "$srcroot/bin/$1/doc/latex/manual.tex.in" ; then
    mkdir -p "$objroot/bin/$1/doc/latex"
    cfgoutputs="$cfgoutputs bin/$1/doc/latex/manual.tex"
  fi
  if test -f "$srcroot/bin/$1/man/man1/$1.1.in" ; then
    mkdir -p "$objroot/bin/$1/man/man1"
    cfgoutputs="$cfgoutputs bin/$1/man/man1/$1.1"
  fi
  if test -f "$srcroot/bin/$1/include/$1_defs.h.in" ; then
    mkdir -p "$objroot/bin/$1/include"
    cfghdrs="$cfghdrs $objroot/bin/$1/include/$1_defs.h"
  fi
  bins="$bins $1"
else
  build_$1="no"
  $2=0
fi
AC_MSG_RESULT($build_$1)
AC_SUBST($2)
])

dnl CW_BUILD_DOC(doc, var)
dnl bin : Name of document.
dnl var : Name of variable to substitute in configure output.
AC_DEFUN(CW_BUILD_DOC,
[
AC_MSG_CHECKING(whether to include $1 document in build)
if test -d "$srcroot/doc/latex/$1" ; then
  build_$1="yes"
  $2=1
  if test -f "$srcroot/doc/latex/$1/$1.tex.in" ; then
    mkdir -p "$objroot/doc/latex/$1"
    cfgoutputs="$cfgoutputs doc/latex/$1/$1.tex doc/latex/$1/Cookfile.inc"
  fi
  docs="$docs $1"
else
  build_$1="no"
  $2=0
fi
AC_MSG_RESULT($build_$1)
AC_SUBST($2)
])


dnl PKG_CHECK_MODULES(GSTUFF, gtk+-2.0 >= 1.3 glib = 1.3.4, action-if, action-not)
dnl defines GSTUFF_LIBS, GSTUFF_CFLAGS, see pkg-config man page
dnl also defines GSTUFF_PKG_ERRORS on error
AC_DEFUN(PKG_CHECK_MODULES, [
  succeeded=no

  if test -z "$PKG_CONFIG"; then
    AC_PATH_PROG(PKG_CONFIG, pkg-config, no)
  fi

  if test "$PKG_CONFIG" = "no" ; then
     echo "*** The pkg-config script could not be found. Make sure it is"
     echo "*** in your path, or set the PKG_CONFIG environment variable"
     echo "*** to the full path to pkg-config."
     echo "*** Or see http://www.freedesktop.org/software/pkgconfig to get pkg-config."
  else
     if ! $PKG_CONFIG --atleast-pkgconfig-version 0.7.0; then
        echo "*** Your version of pkg-config is too old. You need version 0.7.0 or newer."
        echo "*** See http://www.freedesktop.org/software/pkgconfig"
     else
        AC_MSG_CHECKING(for $2)

        if $PKG_CONFIG --exists "$2" ; then
            AC_MSG_RESULT(yes)
            succeeded=yes

            AC_MSG_CHECKING($1_CFLAGS)
            $1_CFLAGS=`$PKG_CONFIG --cflags "$2"`
            AC_MSG_RESULT($$1_CFLAGS)

            AC_MSG_CHECKING($1_LIBS)
            $1_LIBS=`$PKG_CONFIG --libs "$2"`
            AC_MSG_RESULT($$1_LIBS)
        else
            $1_CFLAGS=""
            $1_LIBS=""
            ## If we have a custom action on failure, don't print errors, but 
            ## do set a variable so people can do so.
            $1_PKG_ERRORS=`$PKG_CONFIG --errors-to-stdout --print-errors "$2"`
            ifelse([$4], ,echo $$1_PKG_ERRORS,)
        fi

        AC_SUBST($1_CFLAGS)
        AC_SUBST($1_LIBS)
     fi
  fi

  if test $succeeded = yes; then
     ifelse([$3], , :, [$3])
  else
     ifelse([$4], , AC_MSG_ERROR([Library requirements ($2) not met; consider adjusting the PKG_CONFIG_PATH environment variable if your libraries are in a nonstandard prefix so pkg-config can find them.]), [$4])
  fi
])



