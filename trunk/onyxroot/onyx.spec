# Various configuration options.  If any of these are changed, the resulting
# packages will be incompatible with the standard packages.
%define		disable_threads		0
%define		enable_pth		0
%define		disable_real		0
%define		disable_handle		0
%define		disable_oop		0
%define		disable_regex		0
%define		disable_posix		0
%define		disable_posix_file	0
%define		disable_socket		0
%define		disable_inlines		0
%define		disable_modprompt	0
%define		enable_debug		0

%define		package_name		onyx
%define		package_custom_name	onyx-custom
%define		package_prefix		/usr
%define		package_custom_prefix	/usr/onyx_custom

%if %{disable_threads}
%define		package_name		%{package_custom_name}
%define		package_prefix		%{package_custom_prefix}
%endif

%if %{enable_pth}
%define		package_name		%{package_custom_name}
%define		package_prefix		%{package_custom_prefix}
%endif

%if %{disable_real}
%define		package_name		%{package_custom_name}
%define		package_prefix		%{package_custom_prefix}
%endif

%if %{disable_handle}
%define		package_name		%{package_custom_name}
%define		package_prefix		%{package_custom_prefix}
%endif

%if %{disable_oop}
%define		package_name		%{package_custom_name}
%define		package_prefix		%{package_custom_prefix}
%endif

%if %{disable_regex}
%define		package_name		%{package_custom_name}
%define		package_prefix		%{package_custom_prefix}
%endif

%if %{disable_posix}
%define		package_name		%{package_custom_name}
%define		package_prefix		%{package_custom_prefix}
%endif

%if %{disable_posix_file}
%define		package_name		%{package_custom_name}
%define		package_prefix		%{package_custom_prefix}
%endif

%if %{disable_socket}
%define		package_name		%{package_custom_name}
%define		package_prefix		%{package_custom_prefix}
%endif

%if %{disable_inlines}
%define		package_name		%{package_custom_name}
%define		package_prefix		%{package_custom_prefix}
%endif

%if %{disable_modprompt}
%define		package_name		%{package_custom_name}
%define		package_prefix		%{package_custom_prefix}
%endif

%if %{enable_debug}
%define		package_name		%{package_custom_name}
%define		package_prefix		%{package_custom_prefix}
%define		optflags		-Wall -Wno-uninitialized -pipe -g3
%endif

Name:		%{package_name}
Version:	5.0a22.0
Release:	1
Summary:	An embeddable stack-based threaded interpreted language.
Copyright:	BSD
Group:		Development/Languages

Vendor:		Jason Evans <jasone@canonware.com>
Packager:	Jason Evans <jasone@canonware.com>
URL:		http://www.canonware.com/
Source:		http://www.canonware.com/download/onyx/onyx-%{version}.tar.bz2

Buildrequires:	cook
Buildroot:	%{_tmppath}/%{name}-root
Prefix:		%{package_prefix}

%if ! %{disable_modprompt}
Buildrequires:	libedit-devel
Requires:	libedit
%endif

%if ! %{disable_regex}
Buildrequires:	pcre-devel >= 4.0
Requires:	pcre >= 4.0
%endif

%description
Onyx is a powerful stack-based, multi-threaded, interpreted, general purpose
programming language similar to PostScript and Forth. It can be embedded as an
extension language into other applications, and was designed to have a small
memory footprint. It is among the smallest embeddable interpreters available.

%package devel
Summary: Libraries and include files for embedding Onyx in your application.
Group: Development/Libraries
Requires: %{name} = %{version}

%description devel
Onyx is a powerful stack-based, multi-threaded, interpreted, general purpose
programming language similar to PostScript and Forth. It can be embedded as an
extension language into other applications, and was designed to have a small
memory footprint. It is among the smallest embeddable interpreters available.

This package contains the header files, static library, and onyx_config.  The
headers are necessary when embedding Onyx into an application or when building
Onyx loadable modules.

%prep
%setup -q -n onyx-%{version}

%build
%configure \
 	--prefix=%{package_prefix} \
 	--bindir=%{package_prefix}/bin \
 	--datadir=%{package_prefix}/share \
 	--includedir=%{package_prefix}/include \
 	--libdir=%{package_prefix}/lib \
 	--mandir=%{package_prefix}/share/man \
%if %{disable_threads}
	--disable-threads \
%endif
%if %{enable_pth}
	--enable-pth \
%endif
%if %{disable_real}
	--disable-real \
%endif
%if %{disable_handle}
	--disable-handle \
%endif
%if %{disable_oop}
	--disable-oop \
%endif
%if %{disable_regex}
	--disable-regex \
%endif
%if %{disable_posix}
	--disable-posix \
%endif
%if %{disable_posix_file}
	--disable-posix-file \
%endif
%if %{disable_socket}
	--disable-socket \
%endif
%if %{disable_inlines}
	--disable-inlines \
%endif
%if %{disable_modprompt}
	--disable-modprompt \
%endif
%if %{enable_debug}
	--enable-debug \
%endif

cook
#cook check

%install
[ "$RPM_BUILD_ROOT" -a "$RPM_BUILD_ROOT" != / ] && rm -rf $RPM_BUILD_ROOT
cook PREFIX=$RPM_BUILD_ROOT%{prefix} MANDIR=$RPM_BUILD_ROOT%{prefix}/share/man install

%clean
[ "$RPM_BUILD_ROOT" -a "$RPM_BUILD_ROOT" != / ] && rm -rf $RPM_BUILD_ROOT

%files
%defattr(-,root,root)
%{package_prefix}/bin/onyx
%{package_prefix}/bin/onyx-%{version}
%{package_prefix}/share/man/man1/onyx.1*
%{package_prefix}/share/onyx
%if ! %{disable_modprompt}
%{package_prefix}/share/onyx-%{version}/nx/*
%{package_prefix}/share/onyx-%{version}/nxm/*
%endif
%{package_prefix}/lib/libonyx.so*

%doc %{package_prefix}/share/onyx-%{version}/doc/html
%doc %{package_prefix}/share/onyx-%{version}/doc/pdf
%doc %{package_prefix}/share/onyx-%{version}/doc/ps

%files devel
%defattr(-,root,root)
%{package_prefix}/bin/onyx_config
%{package_prefix}/bin/onyx_config-%{version}
%{package_prefix}/share/man/man1/onyx_config.1*
%{package_prefix}/include/libonyx
%{package_prefix}/lib/libonyx.a
