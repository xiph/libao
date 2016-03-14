Configuration Files
===================

When libao is :c:func:`initialized <ao_initialize>`, it reads two configuration
files: the system-wide configuration in :file:`/etc/libao.conf` and the user
configuration in :file:`~/.libao`. Neither file needs to be present on the
system for libao to load. If both are present, the system configuration file is
read first, followed by the user configuration file. Options set in the user
configuration will take precedence.

Options are specified in these files in the form:

::

    option=value

There can be no extra spaces anywhere on the line. Comment lines begin
with a ``#`` symbol.

AO Options
----------

*default\_driver*
    Set this equal to the short name of the driver you want the system
    to use by default. If this is not specified in any of the
    configuration files, the library will try to guess an appropriate
    driver to use.
*debug* (Value optional/ignored)
    Sets all the drivers as well as AO itself into debugging output
    mode. Unlike passing the debug option to a driver, ``debug`` will
    also print debugging information from driver loading and testing.
*quiet* (Value optional/ignored)
    Sets all the drivers as well as AO itself into silent mode. Errors
    will return only error codes; neither ao nor the drivers will print
    any output whatsoever to stderr.
*verbose* (Value optional/ignored)
    Sets all the drivers as well as AO itself into verbose mode.

Driver Options
--------------

:doc:`Driver options <drivers>` may be set in the configuration files using
``option=value`` pairs, just as they would be set by passing :c:func:`options
<ao_option>` to :c:func:`ao\_open\_live` or :c:func:`ao\_open\_file`. Options
passed to :c:func:`ao\_open\_live` or :c:func:`ao\_open\_file` take precedence
over options specified in a configuration file. Options specified in a
configuration file will be passed to whatever driver is eventually opened; they
cannot be set specific to a single driver.

