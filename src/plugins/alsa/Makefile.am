## Process this file with automake to produce Makefile.in

AUTOMAKE_OPTIONS = foreign

if HAVE_ALSA

alsaltlibs = libalsa.la
alsaldflags = -export-dynamic -avoid-version
alsasources = ao_alsa.c

else

alsaltlibs =
alsaldflags =
alsasources =

endif

INCLUDES = -I$(top_srcdir) -I$(top_srcdir)/include

libdir = $(plugindir)
lib_LTLIBRARIES = $(alsaltlibs)

libalsa_la_LDFLAGS = $(alsaldflags)
libalsa_la_LIBADD = @ALSA_LIBS@
libalsa_la_SOURCES = $(alsasources)

EXTRA_DIST = ao_alsa.c
