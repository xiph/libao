%define name		libao
%define version		0.8.1
%define release 	1
%define apiversion 	2

Summary:	Cross Platform Audio Output Library
Name:		%{name}
Version:	%{version}
Release:	%{release}
Group:		Libraries/Multimedia
Copyright:	GPL
URL:		http://www.xiph.org/
Vendor:		Xiphophorus <team@xiph.org>
Source:		ftp://ftp.xiph.org/pub/ao/%{name}-%{version}.tar.gz
BuildRoot:	%{_tmppath}/%{name}-root
Requires:	esound >= 0.2.8

%description
Libao is a cross platform audio output library.  It currently supports
ESD, aRts, ALSA, OSS, *BSD and Solaris.

%package devel
Summary: Cross Platform Audio Output Library Development
Group: Development/Libraries

%description devel
The libao-devel package contains the header files and documentation
needed to develop applications with libao.

%prep
%setup -q -n %{name}-%{version}

%build
if [ ! -f configure ]; then
  CFLAGS="$RPM_OPT_FLAGS" ./autogen.sh --prefix=/usr
else
  CFLAGS="$RPM_OPT_FLAGS" ./configure --prefix=/usr
fi
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
/usr/lib/libao.so.*
/usr/lib/ao/plugins-%{apiversion}/*.so

%files devel
%doc doc/*.html
%doc doc/*.css
%doc doc/ao_example.c
/usr/include/ao/ao.h
/usr/include/ao/os_types.h
/usr/include/ao/plugin.h
/usr/lib/libao.so
/usr/share/aclocal/ao.m4

%clean 
[ "$RPM_BUILD_ROOT" != "/" ] && rm -rf $RPM_BUILD_ROOT

%post
/sbin/ldconfig

%postun
/sbin/ldconfig

%changelog
* Sun Oct 07 2001 Stan Seibert <indigo@aztec.asu.edu>
- devel packages look for correct documentation files
- added ao/plugin.h include file to devel package
- updated package description

* Sun Sep 03 2000 Jack Moffitt <jack@icecast.org>
- initial spec file created
