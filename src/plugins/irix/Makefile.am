## Process this file with automake to produce Makefile.in

AUTOMAKE_OPTIONS = foreign

if HAVE_IRIX

irixltlibs = libirix.la
irixldflags = -export-dynamic -avoid-version
irixsources = ao_irix.c

else

irixltlibs =
irixldflags =
irixsources =

endif

INCLUDES = -I$(top_srcdir) -I$(top_srcdir)/include

libdir = $(plugindir)
lib_LTLIBRARIES = $(irixltlibs)

libirix_la_LDFLAGS = $(irixldflags)
libirix_la_SOURCES = $(irixsources)

EXTRA_DIST = ao_irix.c
