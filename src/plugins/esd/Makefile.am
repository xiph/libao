## Process this file with automake to produce Makefile.in

AUTOMAKE_OPTIONS = foreign

if HAVE_ESD

esdltlibs = libesd.la
esdldflags = -export-dynamic -avoid-version
esdsources = ao_esd.c

else

esdltlibs =
esdldflags =
esdsources =

endif

INCLUDES = -I$(top_srcdir) -I$(top_srcdir)/include @ESD_CFLAGS@

libdir = $(plugindir)
lib_LTLIBRARIES = $(esdltlibs)

libesd_la_LDFLAGS = $(esdldflags)
libesd_la_LIBADD = @ESD_LIBS@
libesd_la_SOURCES = $(esdsources)

EXTRA_DIST = ao_esd.c
