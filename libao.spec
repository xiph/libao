Summary:	Cross Platform Audio Output Library
Name:		libao
Version:	0.8.2
Release:	3
Group:		Libraries/Multimedia
Copyright:	GPL
URL:		http://www.xiph.org/
Vendor:		Xiphophorus <team@xiph.org>
Source:		http://www.xiph.org/ogg/vorbis/download/%{name}-%{version}.tar.gz
BuildRoot:	%{_tmppath}/%{name}-%{version}-root
BuildPrereq:	esound-devel >= 0.2.8
Requires:	esound >= 0.2.8

%description
Libao is a cross platform audio output library.  It currently supports
ESD, aRts, ALSA, OSS, *BSD and Solaris.

%package devel
Summary: Cross Platform Audio Output Library Development
Group: Development/Libraries
Requires: libao = %{version}

%description devel
The libao-devel package contains the header files and documentation
needed to develop applications with libao.

%prep
%setup -q -n %{name}-%{version}
if [ ! -f configure ]; then
  aclocal
  libtoolize --automake
  automake --add-missing
  autoconf 
fi
perl -p -i -e "s/-O20 -ffast-math/$RPM_OPT_FLAGS/" configure

%build
%configure
make

%install
[ "$RPM_BUILD_ROOT" != "/" ] && rm -rf $RPM_BUILD_ROOT
make DESTDIR=$RPM_BUILD_ROOT install

%files
%defattr(-,root,root)
%doc AUTHORS
%doc CHANGES
%doc COPYING
%doc README
%{_libdir}/libao.so*
%{_libdir}/ao

%files devel
%doc doc/*.html
%doc doc/*.css
%doc doc/ao_example.c
%{_includedir}/ao
%{_libdir}/libao.so
%{_datadir}/aclocal/ao.m4

%clean 
[ "$RPM_BUILD_ROOT" != "/" ] && rm -rf $RPM_BUILD_ROOT

%post
/sbin/ldconfig

%postun
if [ "$1" -ge "1" ]; then
  /sbin/ldconfig
fi

%changelog
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
