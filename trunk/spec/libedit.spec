# Originally authored by Lars Kellogg-Stedman <lars@larsshack.org>.

Name: 		libedit
Version:	2.6.7
Release:	1
Summary:	A command line editing library.
Copyright:	BSD
Group:		Development/Libraries

Vendor:		Christos Zoulas <christos@zoulas.com>
Packager:	Jason Evans <jasone@canonware.com>
Source:		ftp://ftp.astron.com:/pub/libedit/libedit-%{version}.tar.gz

Buildroot:	%{_tmppath}/%{name}-root
Prefix:		/usr

%description
libedit implements command line editing and history features that can be used
to add emacs or vi editing to interactive programs.

%package devel
Summary: Libraries and include files for developing with libedit.
Group: Development/Libraries
Requires: %{name} = %{version}

%description devel
libedit implements command line editing and history features that can be used
to add emacs or vi editing to interactive programs.

This package includes the header files and libraries you will need to
create applications that use libedit.

%prep
%setup -q

%build
%configure --disable-readline
make

%install
[ "$RPM_BUILD_ROOT" -a "$RPM_BUILD_ROOT" != / ] && rm -rf $RPM_BUILD_ROOT
make PREFIX=$RPM_BUILD_ROOT%{prefix} install

%clean
[ "$RPM_BUILD_ROOT" -a "$RPM_BUILD_ROOT" != / ] && rm -rf $RPM_BUILD_ROOT

%files
%defattr(-,root,root)
/usr/man/man5/*
/usr/lib/libedit.so*

%files devel
%defattr(-,root,root)
/usr/man/man3/*
/usr/include/histedit.h
/usr/lib/libedit.a
