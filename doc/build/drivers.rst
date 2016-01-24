Drivers
=======

Libao supports both *live* output drivers and *file* output drivers.
Live output drivers send audio data to sound cards and sound daemons.
File output drivers write audio to disk using a particular file format
(such as WAV, AU, etc.). You must invoke the ao\_open\_????() function
that corresponds to the type of driver you are using, but otherwise live
and file drivers are treated identically in libao.

Driver options may be passed to the drivers via the ``*options``
argument to :c:func:`ao\_open\_live` and
:c:func:`ao\_open\_file`, or they may be set in the
:doc:`configuration file <config>` as ``name=value`` pairs.

Options Understood by All Drivers
---------------------------------

debug
   (value not required) Requests driver print detailed
   debugging information.
matrix
   Set an output channel mapping similar to the use of the
   *matrix* field in :c:type:`ao\_sample\_format`. The
   specified matrix overrides the backend's native channel
   ordering/numbering. The channel numbering used by the driver does not
   necessarily reflect the physical ordering; for example, the fourth
   PulseAudio channel is always 'Center' by default whether the channel
   physically exists or not.
quiet
   (value not required) Requests the driver print no output
   whatsoever, even in the event of error.
verbose
   (value not required) Requests that the driver print more
   detailed information concerning normal operation.

Standard Driver Options
-----------------------

These are options that have the same use in each driver, but may not be
supported by all drivers (due to the option being meaningless or
unimplemented for a given audio backend).

-  "id" - Request a specific playback device/sink/output by number. In
   most audio backends, this will correspond to a specific output
   device, but for all devices, this means 'give me the Nth output'.
-  "dev" - Request a specific playback device/sink/output by name. This
   name will be in a format determined by the specific driver backend
   (eg, the first hardware device in ALSA format would be 'hw:0', and
   for OSS it would be '/dev/dsp').
-  "server" - Daemon-based sound subsystems often support connecting to
   non-default local or remote servers. The "server" option allows
   specifying the connection for a given driver in a format specific to
   that system.
-  "client\_name" - specify a descriptive name for the application; this
   is often used by sound backends to display status information about
   which applications are currently making use of playback.

Live Output Drivers
-------------------

aixs
~~~~

IBM AIX sound driver. According to the author, "tested on AIX 5.1 with
the Crystal chipsets only (found as internal audio in the 7043-140 and
on the MCA adapter DFE5 (7-6)), but it should work with other AIX
releases and the ACPA also."

**Option keys:**

-  "dev" - (see 'Standard Driver Options' above). By default, the driver
   tries "/dev/baud0/1" (device for MCA machines with the Crystal
   chipset). Other possible devices are "/dev/paud0/1" for PCI machines
   with the Crystal chipset, and "/dev/acpa0/1" for MCA machines with
   the ACPA.
-  "id" - (see 'Standard Driver Options' above).

alsa
~~~~

Advanced Linux Sound Architecture (API versions 0.9.x/1.x.x; earlier API
versions are now deprecated).

**Option keys:**

-  "buffer\_time" - Override the default hardware buffer size (in
   milliseconds).
-  "dev" - (see 'Standard Driver Options' above). ALSA device label to
   use. Examples include "hw:0" for the first soundcard and "hw:1" for
   the second. The alsa driver normally chooses one of "surround71",
   "surround51", "surround40", "front", or "default" automatically
   depending on number of output channels.
-  "id" - (see 'Standard Driver Options' above).
-  "period\_time" - Override the default hardware period size (in
   microseconds).
-  "use\_mmap" - set to "yes" or "no" to override the compiled-in
   default to use or not use mmap device access. In the past, some buggy
   alsa drivers have behaved better when not using mmap access at the
   penalty of slightly higher CPU usage.

arts
~~~~

aRts Sound Daemon live output driver.

**Option keys:**

-  "multi" - set to "yes" to allow opening the aRts playback device for
   multiply concurrent playback. Although the driver works properly in
   multi mode, it is known to occasionally crash the aRts server itself.
   Default behavior is "no".

esd
~~~

ESounD audio driver. Although declining in poularity, this sound daemon
is still used on some Linux systems. It permits multiple programs to
play sound simultaneously and sound to be sent to networked computers.

**Option keys:**

-  "host" - Deprecated synonym for 'server' below.
-  "server" - (see 'Standard Driver Options' above). The hostname where
   esd is running. By default sound is played on the local host. A port
   number can be specified after a colon, as in "whizbang.com:555".
-  "client\_name" - descriptive name for this client (eg, the
   application name).

irix
~~~~

IRIX audio driver. This was inherited from the original libao, but has
not been tested. Use at your own risk. (Better yet, fix it! I don't have
access to an IRIX system.)

macosx
~~~~~~

MacOS X AUHAL live output driver. This driver supports MacOS X 10.5 and
later (10.4 and earlier uses an earlier, incompatable interface).

**Option keys:**

-  "buffer\_time" - Set the hardware buffer size to the equivalent of
   value in milliseconds.
-  "dev" - specify the audio device to use by name (eg, 'speaker',
   'headphones', 'hdmi' or 'iMic'), or by audio device UID (eg
   'AppleHDAEngineOutputDP:8,5,1,0:0:{2D4C-05ED-00000000}'). Partial
   matches are allowed, matching is case insensivtive.

nas
~~~

Network Audio System live output driver.

**Option keys:**

-  "buf\_size" - Set size of audio buffer on server in bytes.
-  "host" - Deprecated synonym for 'server' below.
-  "server" - (see 'Standard Driver Options' above). Set location of NAS
   server; See nas(1) for format.

null
~~~~

Null driver. This is just a test device which does not write the audio
data anywhere.

oss
~~~

Open Sound System driver. This is the audio system for older Linux and
FreeBSD as well as some other UNIX-like systems.

**Option keys:**

-  "buffer\_time" - Override the default hardware buffer size (in
   milliseconds).
-  "dev" - (see 'Standard Driver Options' above). By default, the driver
   tries "/dev/sound/dsp", followed by "/dev/dsp".
-  "dsp" - Deprecated synonym for "dev".
-  "id" - (see 'Standard Driver Options' above).

pulse
~~~~~

PulseAudio live audio sound driver. Pulse is a sound server daemon used
by the modern Gnome desktop on UNIX-like systems.

**Option keys:**

-  "buffer\_time" - Override the default hardware buffer size (in
   milliseconds).
-  "dev" - (see 'Standard Driver Options' above). This maps to a
   specific Pulse sink; it may be specified by Pulse sink name, or by
   number.
-  "id" - (see 'Standard Driver Options' above). Maps to a specific
   pulse sink number.
-  "server" - Specifies Pulseaudio server to use.
-  "sink" - Deprecated synonym for "dev".

roar
~~~~

RoarAudio live audio sound driver.

**Option keys:**

-  "server" - Specifies Roar server to use.
-  "host" - Specifies Roar server to use (legacy synonym for 'server').
-  "id" - Selects audio device to use for playback by number. For a roar
   server, this is mapped to a given output mixer.
-  "dev" - Selects audio device to use for playback by name. For a roar
   server, this is a number and thus synonymous with 'id' above.
-  "client\_name" - descriptive name for this client (eg, the
   application name).
-  "role" - stream type as enumerated by Roar; see RoarAudio
   documentation.

sndio
~~~~~

SNDIO is the modern audio interface used by OpenBSD.

**Option keys:**

-  "dev" - (see 'Standard Driver Options' above).
-  "id" - (see 'Standard Driver Options' above).

sun
~~~

Sun audio driver. This is the audio system for NetBSD, OpenBSD, and
Solaris.

**Option keys:**

-  "dev" - (see 'Standard Driver Options' above). By default, the driver
   tries "/dev/sound/0"
-  "id" - (see 'Standard Driver Options' above).

wmm
~~~

Windows MMSound output driver for Win98 and later.

**Option keys:**

-  "dev" - (see 'Standard Driver Options' above).
-  "id" - (see 'Standard Driver Options' above).

File Output Drivers
-------------------

au
~~

Sun audio file driver. Writes a .au file from audio output. This driver
can write usable data to unseekable files (like standard out), which the
wav driver cannot do.

raw
~~~

Raw sample driver. Writes the sound to disk in uncompressed, headerless
form using the byte order specified.

**Option keys:**

-  "byteorder" - Byte order used in the output. Use "native" for native
   machine byte order, "big" for big-endian order, and "little" for
   little-endian order. By default this is "native".

wav
~~~

Windows 'WAV' sound file output. Because of the way WAV files are
structured, this driver cannot correct files unless the target file is
seekable. Writing WAVs to stdout will result in broken files. Use either
the raw or the au driver instead.

Default Driver Detection
------------------------

In the absence of :doc:`configuration files <config>` to explicit identify a
default driver, the library will try to detect a suitable default driver. It
does this by testing every available live output driver (using
:c:func:`ao\_plugin\_test`) and finding the driver with the highest priority
(see the :c:type:`ao\_info` struct) that works. Drivers with priority 0, such
as the null and file output drivers, are never selected as the default.

The ranking system currently used is:

+------------+-----------------------------+
| Priority   | Drivers                     |
+============+=============================+
| 50         | pulse, roar                 |
+------------+-----------------------------+
| 45         | arts [#arts]_               |
+------------+-----------------------------+
| 40         | esd                         |
+------------+-----------------------------+
| 35         | alsa                        |
+------------+-----------------------------+
| 30         | macosx, sndio               |
+------------+-----------------------------+
| 20         | aixs, oss, irix, sun, wmm   |
+------------+-----------------------------+
| 15         | arts [#arts]_               |
+------------+-----------------------------+
| 10         | nas                         |
+------------+-----------------------------+
| 0          | null, all file output       |
+------------+-----------------------------+

.. [#arts] priority depends on whether or not the arts install was built with
   :c:macro:`HAVE_ARTS_SUSPENDED`; when present, the default aRts priority is
   45, else it is 15.

Clearly, any ranking scheme will fail to make everybody happy. For such cases,
the :doc:`configuration files <config>` can be easily used to define an
appropriate default output device.

Adding ``debug`` to the libao :doc:`configuration file <config>` on a line by
itself will cause libao to print what static and dynamic drivers are available
for use, as well as print the testing order.

