## Process this file with automake to produce Makefile.in

AUTOMAKE_OPTIONS = foreign
SUBDIRS = plugins

INCLUDES = -I$(top_srcdir) -I$(top_srcdir)/include

lib_LTLIBRARIES = libao.la

libao_la_SOURCES = audio_out.c config.c ao_null.c ao_wav.c ao_au.c ao_raw.c ao_private.h
libao_la_LDFLAGS = -version-info @LIB_CURRENT@:@LIB_REVISION@:@LIB_AGE@


debug:
	$(MAKE) all CFLAGS="@DEBUG@"

profile:
	$(MAKE) all CFLAGS="@PROFILE@"
