## Process this file with automake to produce Makefile.in

AUTOMAKE_OPTIONS = foreign

if HAVE_OSS

ossltlibs = liboss.la
ossldflags = -export-dynamic -avoid-version
osssources = ao_oss.c

else

ossltlibs =
ossldflags =
osssources =

endif

INCLUDES = -I$(top_srcdir) -I$(top_srcdir)/include

libdir = $(plugindir)
lib_LTLIBRARIES = $(ossltlibs)

liboss_la_LDFLAGS = $(ossldflags)
liboss_la_SOURCES = $(osssources)

EXTRA_DIST = ao_oss.c
