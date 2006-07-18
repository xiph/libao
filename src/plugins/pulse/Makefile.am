## Process this file with automake to produce Makefile.in

AUTOMAKE_OPTIONS = foreign

if HAVE_POLYP

polypltlibs = libpolyp.la
polypsources = ao_polyp.c

else

polypltlibs =
polypsources =

endif

INCLUDES = -I$(top_builddir)/include/ao -I$(top_srcdir)/include

libdir = $(plugindir)
lib_LTLIBRARIES = $(polypltlibs)

libpolyp_la_LDFLAGS= @PLUGIN_LDFLAGS@
libpolyp_la_SOURCES=$(polypsources)
libpolyp_la_LIBADD=$(AM_LIBADD) $(POLYP_LIBS)
libpolyp_la_CFLAGS=$(AM_CFLAGS) $(POLYP_CFLAGS)

EXTRA_DIST = ao_polyp.c
