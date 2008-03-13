Summary: a file construction tool
Name: cook
Version: 2.24
Release: 1
Copyright: GPL
Group: Development/Building
Source: http://www.canb.auug.org.au/~millerp/cook/cook-2.24.tar.gz
URL: http://www.canb.auug.org.au/~millerp/cook/
BuildRoot: /tmp/cook-build-root
Icon: cook.gif
Provides: perl(host_lists.pl)

%description
Cook is a tool for constructing files. It is given a set of files to
create, and recipes of how to create them. In any non-trivial program
there will be prerequisites to performing the actions necessary to
creating any file, such as include files.  The cook program provides a
mechanism to define these.

When a program is being developed or maintained, the programmer will
typically change one file of several which comprise the program.  Cook
examines the last-modified times of the files to see when the
prerequisites of a file have changed, implying that the file needs to be
recreated as it is logically out of date.

Cook also provides a facility for implicit recipes, allowing users to
specify how to form a file with a given suffix from a file with a
different suffix.  For example, to create filename.o from filename.c

* Cook is a replacement for the traditional make(1) tool.  However, it
  is necessary to convert makefiles into cookbooks using the make2cook
  utility included in the distribution.

* Cook has a simple but powerful string-based description language with
  many built-in functions.  This allows sophisticated filename
  specification and manipulation without loss of readability or
  performance.

* Cook is able to use fingerprints to supplement file modification
  times.  This allows build optimization without contorted rules.

* Cook is able to build your project with multiple parallel threads,
  with support for rules which must be single threaded.  It is possible
  to distribute parallel builds over your LAN, allowing you to turn your
  network into a virtual parallel build engine.

If you are putting together a source-code distribution and planning to
write a makefile, consider writing a cookbook instead.  Although Cook
takes a day or two to learn, it is much more powerful and a bit more
intuitave than the traditional make(1) tool.  And Cook doesn't interpret
tab differently to 8 space characters!

%package psdocs
Summary: Cook documentation, PostScript format
Group: Development/Building

%description psdocs
Cook documentation in PostScript format.

%package dvidocs
Summary: Cook documentation, DVI format
Group: Development/Building

%description dvidocs
Cook documentation in DVI format.

%prep
%setup -q
%configure
grep datadir config.status

%build
make

%install
test -n "$RPM_BUILD_ROOT" -a "$RPM_BUILD_ROOT" != / && rm -rf "$RPM_BUILD_ROOT"
make RPM_BUILD_ROOT=$RPM_BUILD_ROOT install

%clean
test -n "$RPM_BUILD_ROOT" -a "$RPM_BUILD_ROOT" != / && rm -rf "$RPM_BUILD_ROOT"

%files
%defattr (-,root,root)
%attr(0755,root,bin) %_prefix/bin/c_incl
%attr(0755,root,bin) %_prefix/bin/cook
%attr(0755,root,bin) %_prefix/bin/cook_bom
%attr(0755,root,bin) %_prefix/bin/cookfp
%attr(0755,root,bin) %_prefix/bin/cook_rsh
%attr(0755,root,bin) %_prefix/bin/cooktime
%attr(0755,root,bin) %_prefix/bin/find_libs
%attr(0755,root,bin) %_prefix/bin/make2cook
%attr(0755,root,bin) %_prefix/bin/roffpp
%attr(0644,root,bin) %_prefix/lib/cook/en/LC_MESSAGES/c_incl.mo
%attr(0644,root,bin) %_prefix/lib/cook/en/LC_MESSAGES/cook_bom.mo
%attr(0644,root,bin) %_prefix/lib/cook/en/LC_MESSAGES/cookfp.mo
%attr(0644,root,bin) %_prefix/lib/cook/en/LC_MESSAGES/cook.mo
%attr(0644,root,bin) %_prefix/lib/cook/en/LC_MESSAGES/cooktime.mo
%attr(0644,root,bin) %_prefix/lib/cook/en/LC_MESSAGES/file_check.mo
%attr(0644,root,bin) %_prefix/lib/cook/en/LC_MESSAGES/find_libs.mo
%attr(0644,root,bin) %_prefix/lib/cook/en/LC_MESSAGES/make2cook.mo
%attr(0644,root,bin) %_prefix/lib/cook/en/LC_MESSAGES/roffpp.mo
%attr(0644,root,bin) %_prefix/share/cook/as
%attr(0644,root,bin) %_prefix/share/cook/bison
%attr(0644,root,bin) %_prefix/share/cook/c
%attr(0644,root,bin) %_prefix/share/cook/c++
%attr(0644,root,bin) %_prefix/share/cook/en/man1/c_incl.1*
%attr(0644,root,bin) %_prefix/share/cook/en/man1/cook.1*
%attr(0644,root,bin) %_prefix/share/cook/en/man1/cook_bom.1*
%attr(0644,root,bin) %_prefix/share/cook/en/man1/cookfp.1*
%attr(0644,root,bin) %_prefix/share/cook/en/man1/cook_lic.1*
%attr(0644,root,bin) %_prefix/share/cook/en/man1/cook_rsh.1*
%attr(0644,root,bin) %_prefix/share/cook/en/man1/cooktime.1*
%attr(0644,root,bin) %_prefix/share/cook/en/man1/find_libs.1*
%attr(0644,root,bin) %_prefix/share/cook/en/man1/make2cook.1*
%attr(0644,root,bin) %_prefix/share/cook/en/man1/roffpp.1*
%attr(0644,root,bin) %_prefix/share/cook/en/refman.txt
%attr(0644,root,bin) %_prefix/share/cook/en/tutorial.txt
%attr(0644,root,bin) %_prefix/share/cook/en/user-guide.txt
%attr(0644,root,bin) %_prefix/share/cook/f77
%attr(0644,root,bin) %_prefix/share/cook/functions
%attr(0644,root,bin) %_prefix/share/cook/g77
%attr(0644,root,bin) %_prefix/share/cook/gcc
%attr(0644,root,bin) %_prefix/share/cook/home
%attr(0644,root,bin) %_prefix/share/cook/host_lists.pl
%attr(0644,root,bin) %_prefix/share/cook/lex
%attr(0644,root,bin) %_prefix/share/cook/library
%attr(0644,root,bin) %_prefix/share/cook/print
%attr(0644,root,bin) %_prefix/share/cook/program
%attr(0644,root,bin) %_prefix/share/cook/rcs
%attr(0644,root,bin) %_prefix/share/cook/recursive
%attr(0644,root,bin) %_prefix/share/cook/sccs
%attr(0644,root,bin) %_prefix/share/cook/text
%attr(0644,root,bin) %_prefix/share/cook/usr
%attr(0644,root,bin) %_prefix/share/cook/usr.local
%attr(0644,root,bin) %_prefix/share/cook/yacc
%attr(0644,root,bin) %_prefix/share/cook/yacc_many
%attr(0644,root,bin) %_prefix/share/man/man1/c_incl.1*
%attr(0644,root,bin) %_prefix/share/man/man1/cook.1*
%attr(0644,root,bin) %_prefix/share/man/man1/cook_bom.1*
%attr(0644,root,bin) %_prefix/share/man/man1/cookfp.1*
%attr(0644,root,bin) %_prefix/share/man/man1/cook_lic.1*
%attr(0644,root,bin) %_prefix/share/man/man1/cook_rsh.1*
%attr(0644,root,bin) %_prefix/share/man/man1/cooktime.1*
%attr(0644,root,bin) %_prefix/share/man/man1/find_libs.1*
%attr(0644,root,bin) %_prefix/share/man/man1/make2cook.1*
%attr(0644,root,bin) %_prefix/share/man/man1/roffpp.1*

%files psdocs
%attr(0644,root,bin) %_prefix/share/cook/en/refman.ps
%attr(0644,root,bin) %_prefix/share/cook/en/tutorial.ps
%attr(0644,root,bin) %_prefix/share/cook/en/user-guide.ps

%files dvidocs
%attr(0644,root,bin) %_prefix/share/cook/en/refman.dvi
%attr(0644,root,bin) %_prefix/share/cook/en/tutorial.dvi
%attr(0644,root,bin) %_prefix/share/cook/en/user-guide.dvi
