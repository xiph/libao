## Process this file with automake to produce Makefile.in

AUTOMAKE_OPTIONS = foreign

if HAVE_ARTS

artsltlibs = libarts.la
artsldflags = -export-dynamic -avoid-version
artssources = ao_arts.c

else

artsltlibs =
artsldflags =
artssources =

endif

INCLUDES = -I$(top_srcdir) -I$(top_srcdir)/include

libdir = $(plugindir)
lib_LTLIBRARIES = $(artsltlibs)

libarts_la_LDFLAGS = $(artsldflags)
CFLAGS = @ARTS_CFLAGS@
libarts_la_LIBADD = @ARTS_LIBS@
libarts_la_SOURCES = $(artssources)

EXTRA_DIST = ao_arts.c
