Name: 		libedit
Summary:	A command line editing library.
Version:	2.6.7
Release:	1
License:	BSD
Group:		Development/Libraries
Source0:	ftp://ftp.astron.com:/pub/libedit/libedit-%{version}.tar.gz
Buildroot:	%{_tmppath}/%{name}-%{version}-%{release}-root
Provides:	libedit.so

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
rm -fr %{buildroot}
make PREFIX=%{buildroot} install
mkdir %{buildroot}/%{_prefix}/share
mv %{buildroot}/%{_prefix}/man %{buildroot}/%{_prefix}/share

%clean
rm -fr %{buildroot}

%post
/sbin/ldconfig

%postun
if [ "$1" -ge "1" ]; then
  /sbin/ldconfig
fi

%files
%defattr(-,root,root)
%{_mandir}/man5/*
%{_libdir}/libedit.so*

%files devel
%defattr(-,root,root)
%{_mandir}/man3/*
%{_includedir}/histedit.h
%{_libdir}/libedit.a
