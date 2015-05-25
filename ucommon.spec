#
# spec file for package ucommon
#
# Copyright (c) 2015 Cherokees of Idaho.
# Copyright (c) 2008-2014 David Sugar, Tycho Softworks.
#
# All modifications and additions to the file contributed by third parties
# remain the property of their copyright owners, unless otherwise agreed
# upon. The license for this file, and modifications and additions to the
# file, is the same license as for the pristine package itself (unless the
# license for the pristine package is not an Open Source License, in which
# case the license is the MIT License). An "Open Source License" is a
# license that conforms to the Open Source Definition (Version 1.9)
# published by the Open Source Initiative.

# Please submit bugfixes or comments via http://bugs.opensuse.org/
#

%define libname	libucommon7
%if %{_target_cpu} == "x86_64"
%define	build_docs	1
%else
%define	build_docs	0
%endif
Name:           ucommon
Version:        6.3.2
Release:        1
Summary:        Portable C++ runtime for threads and sockets
License:        LGPL-3.0+
Group:          Productivity/Other
Url:            http://www.gnu.org/software/commoncpp
Source:         %{name}-%{version}.tar.gz
BuildRequires:  cmake
BuildRequires:  doxygen
BuildRequires:  gcc-c++
BuildRequires:  graphviz-gd
BuildRequires:  pkgconfig
BuildRequires:  pkgconfig(openssl)
Requires:       %{libname}%{_isa} = %{version}-%{release}
# historically we used a -bin for ucommon applications...
Obsoletes:      %{name}-bin < %{version}-%{release}
Provides:       %{name}-bin = %{version}-%{release}
BuildRoot:      %{_tmppath}/%{name}-%{version}-build

%description
GNU uCommon C++ is a lightweight library to facilitate using C++ design
patterns even for very deeply embedded applications, such as for systems using
uClibc along with POSIX threading support. For this reason, uCommon disables
language features that consume memory or introduce runtime overhead. uCommon
introduces some design patterns from Objective-C, such as reference counted
objects, memory pools, and smart pointers.  GNU uCommon introduces some new
concepts for handling of thread locking and synchronization.

%package -n %{libname}
Summary:        GNU ucommon runtime library
Group:          System/Libraries

%description -n %{libname}
GNU uCommon C++ is a lightweight library to facilitate using C++ design
patterns even for very deeply embedded applications, such as for systems using
uClibc along with POSIX threading support. For this reason, uCommon disables
language features that consume memory or introduce runtime overhead. uCommon
introduces some design patterns from Objective-C, such as reference counted
objects, memory pools, and smart pointers.  GNU uCommon introduces some new
concepts for handling of thread locking and synchronization.

%package devel
Summary:        Headers for building ucommon applications
Group:          Development/Libraries/C and C++
Requires:       %{libname}%{_isa} = %{version}-%{release}
Requires:       gcc-c++
Requires:       libopenssl-devel%{_isa}
Requires:       pkgconfig

%description devel
This package provides header and support files needed for building
applications that use the uCommon library and frameworks.

%if %{build_docs}

%package doc
Summary:        Generated class documentation for ucommon
Group:          Documentation/HTML
BuildArch:      noarch

%description doc
Generated class documentation for GNU uCommon library from header files,
html browsable.
%endif

%prep
%setup -q

%build
%cmake \
    -DCMAKE_INSTALL_SYSCONFDIR:PATH=%{_sysconfdir} \
    -DCMAKE_INSTALL_LOCALSTATEDIR:PATH=%{_localstatedir}

make %{?_smp_mflags}
%if	%{build_docs}
rm -rf doc/html
make doc %{?_smp_mflags}
%endif

%install
%cmake_install
chmod 0755 %{buildroot}%{_bindir}/ucommon-config
chmod 0755 %{buildroot}%{_bindir}/commoncpp-config

%files -n %{libname}
%defattr(-,root,root,-)
%{_libdir}/libucommon.so.*
%{_libdir}/libusecure.so.*
%{_libdir}/libcommoncpp.so.*

%files
%defattr(-,root,root,-)
%doc AUTHORS README COPYING COPYING.LESSER COPYRIGHT NEWS SUPPORT ChangeLog
%{_bindir}/args
%{_bindir}/car
%{_bindir}/keywait
%{_bindir}/scrub-files
%{_bindir}/mdsum
%{_bindir}/sockaddr
%{_bindir}/zerofill
%{_bindir}/pdetach
%{_mandir}/man1/args.*
%{_mandir}/man1/car.*
%{_mandir}/man1/scrub-files.*
%{_mandir}/man1/mdsum.*
%{_mandir}/man1/sockaddr.*
%{_mandir}/man1/zerofill.*
%{_mandir}/man1/keywait.*
%{_mandir}/man1/pdetach.*

%files devel
%defattr(-,root,root,-)
%{_libdir}/*.so
%{_includedir}/ucommon/
%{_includedir}/commoncpp/
%{_libdir}/pkgconfig/*.pc
%{_bindir}/ucommon-config
%{_bindir}/commoncpp-config
%{_mandir}/man1/ucommon-config.*
%{_mandir}/man1/commoncpp-config.*

%dir %{_datadir}/ucommon
%dir %{_datadir}/ucommon/cmake
%{_datadir}/ucommon/cmake/Cape*.cmake

%if %{build_docs}

%files doc
%defattr(-,root,root,-)
%doc build/doc/html/*
%endif

%post -n %{libname} -p /sbin/ldconfig

%postun -n %{libname} -p /sbin/ldconfig

%changelog
