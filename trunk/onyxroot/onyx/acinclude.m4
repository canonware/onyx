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
  cfgoutputs="$cfgoutputs mod/$1/Cookfile.inc mod/$1/nx/$1/$1_defs.nx"
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
