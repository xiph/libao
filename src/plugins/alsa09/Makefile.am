## Process this file with automake to produce Makefile.in

AUTOMAKE_OPTIONS = foreign

if HAVE_ALSA09

alsa09ltlibs = libalsa09.la
alsa09sources = ao_alsa09.c

else

alsa09ltlibs =
alsa09sources =

endif

INCLUDES = -I$(top_builddir)/include/ao -I$(top_srcdir)/include

libdir = $(plugindir)
lib_LTLIBRARIES = $(alsa09ltlibs)

libalsa09_la_LDFLAGS = @PLUGIN_LDFLAGS@
libalsa09_la_LIBADD = @ALSA09_LIBS@
libalsa09_la_SOURCES = $(alsa09sources)

EXTRA_DIST = ao_alsa09.c
