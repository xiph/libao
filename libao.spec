Name:		libao
Version:	0.8.3
Release:	1
Summary:	Cross-Platform Audio Output Library

Group:		System Environment/Libraries
License:	GPL
URL:		http://www.xiph.org/
Vendor:		Xiph.org Foundation <team@xiph.org>
Source:		http://www.xiph.org/ogg/vorbis/download/%{name}-%{version}.tar.gz
BuildRoot:	%{_tmppath}/%{name}-%{version}-root

# glibc-devel is needed for oss plug-in build
BuildRequires:  glibc-devel
BuildRequires: 	esound-devel >= 0.2.8
BuildRequires: 	arts-devel
BuildRequires: 	alsa-lib-devel >= 0.9.0
# FIXME: perl is needed for the dirty configure flag trick, which should be
# solved differently
BuildRequires:  perl

%description
Libao is a cross-platform audio output library.  It currently supports
ESD, aRts, ALSA, OSS, *BSD and Solaris.

This package provides plug-ins for OSS, ESD, aRts, and ALSA (0.9).  You will
need to install the supporting libraries for any plug-ins you want to use
in order for them to work.

%package devel
Summary: Cross Platform Audio Output Library Development
Group: Development/Libraries
Requires: libao = %{version}

%description devel
The libao-devel package contains the header files, libraries and
documentation needed to develop applications with libao.

%prep
%setup -q -n %{name}-%{version}

perl -p -i -e "s/-O20/$RPM_OPT_FLAGS/" configure
perl -p -i -e "s/-ffast-math//" configure

%build

%configure

make

%install
[ "$RPM_BUILD_ROOT" != "/" ] && rm -rf $RPM_BUILD_ROOT

#FIXME: makeinstall breaks the plugin install location; they end up in /usr/lib
make DESTDIR=$RPM_BUILD_ROOT install

%clean 
[ "$RPM_BUILD_ROOT" != "/" ] && rm -rf $RPM_BUILD_ROOT

%post -p /sbin/ldconfig

%postun
if [ "$1" -ge "1" ]; then
  /sbin/ldconfig
fi

%files
%defattr(-,root,root)
%doc AUTHORS CHANGES COPYING README
%{_libdir}/libao.so.*
%{_libdir}/ao/*/liboss.so
%{_libdir}/ao/*/libesd.so
%{_libdir}/ao/*/libarts.so
%{_libdir}/ao/*/libalsa09.so
%{_mandir}/man5/*

%files devel
%doc doc/*
%{_includedir}/ao
%{_libdir}/libao.so
%{_libdir}/libao.a
%{_libdir}/ao/*/*.a
%{_datadir}/aclocal/ao.m4

%changelog
* Fri Jul 19 2002 Michael Smith <msmith@labyrinth.net.au> 0.8.3-2
- re-disable static libraries (they do not work - at all)

* Sun Jul 14 2002 Thomas Vander Stichele <thomas@apestaart.org> 0.8.3-1
- new release for vorbis 1.0
- small cleanups
- added better BuildRequires
- added alsa-lib-devel 0.9.0 buildrequires
- added static libraries to -devel
- added info about plug-ins to description
- listed plug-in so files explicitly to ensure package build fails when one
  is missing

* Mon Jan  7 2002 Peter Jones <pjones@redhat.com> 0.8.2-4
- minor cleanups, even closer to RH .spec 
- arts-devel needs a build dependancy to be sure the
  plugin will get built

* Wed Jan  2 2002 Peter Jones <pjones@redhat.com> 0.8.2-3
- fix libao.so's provide

* Wed Jan  2 2002 Peter Jones <pjones@redhat.com> 0.8.2-2
- merge RH and Xiphophorous packages

* Tue Dec 18 2001 Jack Moffitt <jack@xiph.org>
- Update for 0.8.2 release.

* Sun Oct 07 2001 Jack Moffitt <jack@xiph.org>
- supports configurable prefixes

* Sun Oct 07 2001 Stan Seibert <indigo@aztec.asu.edu>
- devel packages look for correct documentation files
- added ao/plugin.h include file to devel package
- updated package description

* Sun Sep 03 2000 Jack Moffitt <jack@icecast.org>
- initial spec file created
