## Process this file with automake to produce Makefile.in

AUTOMAKE_OPTIONS = foreign

if HAVE_HAIKU

haikultlibs = libhaiku.la
haikusources = ao_haiku.cpp

else

haikultlibs =
haikusources =

endif

INCLUDES = -I$(top_builddir)/include/ao -I$(top_srcdir)/include

libdir = $(plugindir)
lib_LTLIBRARIES = $(haikultlibs)

libhaiku_la_LDFLAGS = @PLUGIN_LDFLAGS@ -lbe -lmedia
libhaiku_la_SOURCES = $(haikusources)

EXTRA_DIST = ao_haiku.cpp
