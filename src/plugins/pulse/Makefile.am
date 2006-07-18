## Process this file with automake to produce Makefile.in

AUTOMAKE_OPTIONS = foreign

if HAVE_PULSE

pulseltlibs = libpulse.la
pulsesources = ao_pulse.c

else

pulseltlibs =
pulsesources =

endif

INCLUDES = -I$(top_builddir)/include/ao -I$(top_srcdir)/include

libdir = $(plugindir)
lib_LTLIBRARIES = $(pulseltlibs)

libpulse_la_LDFLAGS= @PLUGIN_LDFLAGS@
libpulse_la_SOURCES=$(pulsesources)
libpulse_la_LIBADD=$(AM_LIBADD) $(PULSE_LIBS)
libpulse_la_CFLAGS=$(AM_CFLAGS) $(PULSE_CFLAGS)

EXTRA_DIST = ao_pulse.c
