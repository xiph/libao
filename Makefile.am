## Process this file with automake to produce Makefile.in

AUTOMAKE_OPTIONS = foreign dist-zip

SUBDIRS = src include doc debian

m4datadir = $(datadir)/aclocal
m4data_DATA = ao.m4

EXTRA_DIST = README AUTHORS CHANGES COPYING libao.spec ao.m4 acinclude.m4

debug:
	$(MAKE) all CFLAGS="@DEBUG@"

profile:
	$(MAKE) all CFLAGS="@PROFILE@"
