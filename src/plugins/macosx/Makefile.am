## Process this file with automake to produce Makefile.in

AUTOMAKE_OPTIONS = foreign

if HAVE_MACOSX

macosxltlibs = libmacosx.la
macosxldflags = -export-dynamic -avoid-version
macosxsources = ao_macosx.c

else

macosxltlibs =
macosxldflags =
macosxsources =

endif

INCLUDES = -I$(top_builddir)/include/ao -I$(top_srcdir)/include -framework CoreAudio

libdir = $(plugindir)
lib_LTLIBRARIES = $(macosxltlibs)

libmacosx_la_LDFLAGS = $(macosxldflags)
libmacosx_la_SOURCES = $(macosxsources)

EXTRA_DIST = ao_macosx.c
