Name:		onyx
Summary:	An embeddable stack-based threaded interpreted language.
Version:	5.1.0
Release:	1
License:	BSD-like
Group:		Development/Languages
URL:		http://www.canonware.com/
Source0:	http://www.canonware.com/download/onyx/onyx-%{version}.tar.bz2
Buildroot:	%{_tmppath}/%{name}-%{version}-%{release}-root
Requires:	libedit, pcre >= 4.0
BuildPrereq:	cook, libedit-devel, pcre-devel >= 4.0
Provides:	libonyx.so

%description
Onyx is a powerful stack-based, multi-threaded, interpreted, general purpose
programming language similar to PostScript and Forth. It can be embedded as an
extension language into other applications, and was designed to have a small
memory footprint. It is among the smallest embeddable interpreters available.

This package requires the pcre package for regular expressions, and libedit for
interactive command line editing.

%package devel
Summary: Programs, libraries and include files for developing with libonyx.
Group: Development/Libraries
Requires: %{name} = %{version}

%description devel
Onyx is a powerful stack-based, multi-threaded, interpreted, general purpose
programming language similar to PostScript and Forth. It can be embedded as an
extension language into other applications, and was designed to have a small
memory footprint. It is among the smallest embeddable interpreters available.

This package contains onyx_config, header files, and a static libonyx.
onyx_config provides a convenient method for configuring applications that use
Onyx.  The headers are necessary when embedding Onyx into an application or when
building Onyx loadable modules.

%prep
%setup -q -n onyx-%{version}

%build
%configure
cook

%install
rm -fr %{buildroot}
cook \
	PREFIX="%{buildroot}/%{_prefix}" \
	BINDIR="%{buildroot}/%{_bindir}" \
	DATADIR="%{buildroot}/%{_datadir}" \
	LIBDIR="%{buildroot}/%{_libdir}" \
	INCLUDEDIR="%{buildroot}/%{_includedir}" \
	MANDIR="%{buildroot}/%{_mandir}" \
	install

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
%{_bindir}/onyx
%{_bindir}/onyx-%{version}
%{_mandir}/man1/onyx.1*
%{_datadir}/onyx
%{_datadir}/onyx-%{version}
%{_libdir}/libonyx.so*

%doc %{_datadir}/doc/onyx
%doc %{_datadir}/doc/onyx-%{version}

%files devel
%defattr(-,root,root)
%{_bindir}/onyx_config
%{_bindir}/onyx_config-%{version}
%{_mandir}/man1/onyx_config.1*
%{_includedir}/libonyx
%{_libdir}/libonyx.a
