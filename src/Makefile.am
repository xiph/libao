## Process this file with automake to produce Makefile.in

AUTOMAKE_OPTIONS = foreign

INCLUDES = -I$(top_srcdir) -I$(top_srcdir)/include

lib_LTLIBRARIES = libao.la

#libao_la_SOURCES = audio_out.c @LIBAO_FILES@
libao_la_SOURCES = audio_out.c ao_oss.c ao_wav.c ao_null.c
libao_la_LDFLAGS = -version-info @LIB_CURRENT@:@LIB_REVISION@:@LIB_AGE@

EXTRA_libao_la_SOURCES = ao_alsa.c ao_irix.c ao_oss.c ao_wav.c ao_esd.c ao_null.c ao_solaris.c audio_out.c


debug:
	$(MAKE) all CFLAGS="@DEBUG@"

profile:
	$(MAKE) all CFLAGS="@PROFILE@"
