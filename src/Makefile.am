## Process this file with automake to produce Makefile.in

AUTOMAKE_OPTIONS = foreign
SUBDIRS = plugins

INCLUDES = -I$(top_builddir)/include/ao -I$(top_srcdir)/include -DAO_PLUGIN_PATH=\"@plugindir@\"

lib_LTLIBRARIES = libao.la

libao_la_SOURCES = audio_out.c config.c ao_null.c ao_wav.c ao_au.c ao_raw.c ao_aixs.c ao_private.h
libao_la_LDFLAGS = -version-info @LIB_CURRENT@:@LIB_REVISION@:@LIB_AGE@ -ldl


debug:
	$(MAKE) all CFLAGS="@DEBUG@"

profile:
	$(MAKE) all CFLAGS="@PROFILE@"
