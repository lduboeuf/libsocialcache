Name:       libsocialcache
Summary:    A library that manages data from social networks
Version:    0.2.1
Release:    1
License:    BSD and LGPLv2+
URL:        https://github.com/sailfishos/libsocialcache
Source0:    %{name}-%{version}.tar.bz2
BuildRequires:  pkgconfig(Qt5Core)
BuildRequires:  pkgconfig(Qt5Gui)
BuildRequires:  pkgconfig(Qt5Qml)
BuildRequires:  pkgconfig(Qt5Sql)
BuildRequires:  pkgconfig(Qt5DBus)
BuildRequires:  pkgconfig(Qt5Test)
BuildRequires:  qt5-qttools
BuildRequires:  qt5-qttools-linguist
BuildRequires:  pkgconfig(buteosyncfw5)
BuildRequires:  pkgconfig(libsailfishkeyprovider)

Requires:  qt5-plugin-sqldriver-sqlite

%description
libsocialcache is a library that manages data from social
networks. It also provides higher level models with a QML
plugin.

%package qml-plugin-ts-devel
Summary:   Translation source for libsocialcache

%description qml-plugin-ts-devel
Translation source for socialcache qml plugin

%package devel
Summary:   Development files for libsocialcache
Requires:  libsocialcache = %{version}

%description devel
This package contains development files for libsocialcache.

%package qml-plugin
Summary:   QML plugin for libsocialcache

%description qml-plugin
This package contains the qml plugin for socialcache

%package tests
Summary:    Unit tests for libsocialcache
License:    BSD
Requires:   %{name} = %{version}-%{release}

%description tests
This package contains unit tests for the libsocialcache library.

%prep
%setup -q -n %{name}-%{version}

%build
%qmake5 "VERSION=%{version}" \
    "CONFIG+=qml"
%make_build

%install
%qmake5_install

%post -p /sbin/ldconfig

%postun -p /sbin/ldconfig

%files
%{_libdir}/libsocialcache.so.*
%license COPYING

%files qml-plugin-ts-devel
%{_datadir}/translations/source/socialcache.ts

%files devel
%{_includedir}/socialcache/*.h
%{_libdir}/libsocialcache.so
%{_libdir}/pkgconfig/socialcache.pc

%files qml-plugin
%{_libdir}/qt5/qml/org/nemomobile/socialcache/plugins.qmltypes
%{_libdir}/qt5/qml/org/nemomobile/socialcache/qmldir
%{_libdir}/qt5/qml/org/nemomobile/socialcache/libsocialcacheqml.so
%{_datadir}/translations/socialcache_eng_en.qm

%files tests
/opt/tests/libsocialcache/
