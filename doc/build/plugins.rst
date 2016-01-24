Plugin API
==========

Plugins are drivers that are loaded dynamically when libao is first
initialized by the client application. Drivers that are operating system
dependent, like the ``oss`` and ``sun`` drivers, or that depend on
external libraries, like the ``esd`` driver, must be implemented as
plugins in order to keep binary packagers happy. There are also
statically linked drivers, which are written in a nearly identical way,
but won't be covered here. In nearly all cases, a dynamically loadable
plugin is the preferred way to write a driver, and the required way if
the driver depends upon any external libraries.

Life Cycle
----------

The life cycle of a plugin is:

-  When libao is first :c:func:`initialized <ao_initialize>`, it loads
   all of the plugins from disk.
-  Libao then :c:func:`tests <ao_plugin_test>` each plugin to see if can
   be used as the default driver.
-  When the user opens a device, libao will:

   -  Call :c:func:`ao\_plugin\_device\_init` to allow the plugin to allocate
      and initialize any private data structures it will use.
   -  Call :c:func:`ao\_plugin\_set\_option` for each parameter passed to the
      library by the client application.
   -  Call :c:func:`ao\_plugin\_open` to open the device for playback.

-  Each time the client app calls :c:func:`ao\_play`, the
   library will reorder the byte format (little-endian vs. big-endian)
   and rearrange input channels to match the format requested by the
   plugin. The library will then call
   :c:func:`ao\_plugin\_play` for the block of audio
   data.
-  When the client app closes the audio device, the library calls
   :c:func:`ao\_plugin\_close` to close the device,
   followed by a call to
   :c:func:`ao\_plugin\_device\_clear` to deallocate
   the private data structures.
-  When the library is :c:func:`shutdown <ao_shutdown>`, the plugin will
   be unloaded from memory.

In case of errors, :c:func:`ao\_plugin\_device\_clear` will always be called if
:c:func:`ao\_plugin\_device\_init` executed successfully. Similarly,
:c:func:`ao\_plugin\_close` will always be called if :c:func:`ao\_plugin\_open`
executed successfully.

Creating a New Plugin
---------------------

In order to write a new plugin, follow these steps:

-  Decide upon a new short name for your plugin. It should be less than
   8 characters and contain only alphanumeric characters (underscores
   are okay, but discouraged).
-  Make a new directory in the src/plugins directory with the short name
   of your plugin.
-  Study the contents of one of the other plugin directories. The Sun
   driver is a good example of a driver that uses system devices for
   output, and the ALSA driver is a good example of a plugin that uses
   an external library. Rename the source file to ao\_shortname.c, where
   "shortname" is the short name of your plugin.
-  Create an :c:type:`ao\_info` structure.
-  Implement the all of the methods defined in the :doc:`plugin
   API <plugins>`.
-  Create src/plugins/shortname/Makefile.am ("shortname" is as described
   above) and edit the files configure.ac and src/plugins/Makefile.am.
   There should be a an configure option to disable your plugin. Look at
   the existing configure.ac file for examples of how to do this.
-  Test it thoroughly! :)
-  Send a tarball of the src/plugin/shortname directory (only this
   directory, please!) and a cvs diff -u of the changes you have made to
   the `vorbis-dev <mailto:vorbis-dev@xiph.org>`__ list and we'll take a
   look at it for inclusion.

API Implementation Tips
-----------------------

-  Remember to close any devices/connections you openned in
   :c:func:`ao\_plugin\_test`.
-  Although you should try to allocate all of your data structures in
   :c:func:`ao\_plugin\_device\_init`, there are cases where you won't be able
   to allocate memory until :c:func:`ao\_plugin\_open` is called. That is
   acceptable, but the rule is that you must deallocate memory in
   :c:func:`ao\_plugin\_close` that was allocated in :c:func:`ao\_plugin\_open`
   and deallocate memory in :c:func:`ao\_plugin\_device\_clear` that was
   allocated in :c:func:`ao\_plugin\_device\_init`.
-  Don't forget to set device->driver\_byte\_format in
   :c:func:`ao\_plugin\_open` to the byte ordering your plugin needs. The libao
   core will reorder the bytes for you if it necessary.
-  Depending on the driver, a channel mapping may be very easy, tricky,
   or impossible. If the audio backend uses a fixed numbering for its channels
   (not necessarily a fixed order), your new driver can simply set an
   ``output_matrix`` and ``output_matrix_ordering`` in
   :c:func:`ao\_plugin\_device\_init` and not need to worry about much else.
   Libao will automatically permute channels, as well has hand over the needed
   mapping information in a form that can usually be submitted directly to the
   audio backend during device configuration. Examples of drivers that do this
   are WAV, ALSA and PULSE.
-  Some drivers can't perform channel mapping determination until they
   see the input sample format in :c:func:`ao\_plugin\_open`. Such a driver
   supports channel mapping by setting the overall ``output_matrix_ordering``
   in :c:func:`ao\_plugin\_device\_init` and then setting the ``inter_matrix``
   field in :c:func:`ao\_plugin\_open`. One driver that works this way is the
   Roar plugin.
-  The number of channels to be sent to the hardware is not the number
   of channels declared in the sample format; use the
   device->output\_channels field instead. The number of channels an
   application submits to libao is not necessarily the same as the
   number of channels libao sends to the plugin to play.
-  Read the :doc:`driver documentation <drivers>` to see what priority
   you should set for your plugin in the :c:type:`ao\_info` structure.


.. c:function:: int ao_plugin_close(ao_device *device)

    Close the device. Private data structures are not deallocated yet.

    :param device:
        Pointer to pre-allocated device structure.
    :return:
        1 indicates remaining data written correctly and device closed.  0
        indicates an error while the device was being closed. If this device
        was writing to a file, the file may be corrupted.

.. c:function:: void ao_plugin_device_clear(ao_device *device)

    Deallocate the private device data structures.

    :param device:
        Pointer to pre-allocated device structure.

.. c:function:: int ao_plugin_device_init(ao_device *device)

    Initialize the private device data structures. Memory should be
    allocated, and default values set if necessary, but devices should not
    be opened until :c:func:`ao\_plugin\_open` is called.

    Drivers that wish to support automatic surround channel mapping should
    set :c:type:`ao_device.output_matrix_order` to one of:

    -  AO\_OUTPUT\_MATRIX\_FIXED - The audio driver outputs a fixed channel
       order that is always the same for a given number of channels and
       can't be changed. One example is ALSA where to play to the side right
       speaker, it is necessary to open eight channels and write to side
       right as channel eight, even if the other seven channels are unused.
    -  AO\_OUTPUT\_MATRIX\_COLLAPSIBLE - The audio driver outputs a fixed
       channel order that is always the same, but unused channels are not
       sent. In such a driver with the channel order 'L,R,C,LFE,BL,BR', we
       can write to the two rear speakers without opening six channels but
       the rear speakers must still be in the order 'BL,BR'. Such drivers
       usually use a bitmap to represent the channels present in the output.
       Examples are wav, wmm and macosx.
    -  AO\_OUTPUT\_MATRIX\_PERMUTABLE - Channels may be arranged in any
       order. An example of such a driver is pulse.

    In addition, a driver may set :c:type:`ao_device.output_matrix` to the list
    of possible output channels in order. Note that even
    AO\_OUTPUT\_MATRIX\_PERMUTABLE drivers usually have a fixed numbering
    scheme for channels even if the channels may be sent in any order. A driver
    that is unable to set an output matrix without inspecting the input sample
    format (for example, the roar driver), may set
    :c:type:`ao_device.output_matrix_order` to
    :c:macro:`AO_OUTPUT_MATRIX_FIXED` and delay channel mapping initialization
    until :c:func:`ao\_plugin\_open`.

    :param device:
        Pointer to pre-allocated device structure. The driver should store
        the address of its private memory in :c:type:`ao_device.internal`.
    :return:
        1 indicates success.
        0 indicates failure, probably due to memory allocation problems.
        :c:func:`ao\_plugin\_device\_clear` should be called to ensure the
        deallocate of all private data structures.

.. c:function:: ao_info* ao_plugin_driver_info()

    Get the driver information structure.

    :return:
        Pointer to the driver information structure. This structure should not
        be modified.


.. c:function:: char *ao_plugin_file_extension(int driver_id)

    Returns the normal file extension associated with this driver (like "wav"
    or "au"). This is an **optional** function. Live audio plugins do not need
    to implement this function.

    :return:
        pointer to string containing suggested file extension.
        NULL if this driver has no file extension associated with it. Live
        audio plugins must always return NULL if they do implement this
        function.

.. c:function:: int ao_plugin_open(ao_device *device, ao_sample_format *format)

    Open the device for playback. All of the device options will have already
    been set by previous calls to :c:func:`ao\_plugin\_set\_option`. If this is
    a file output driver, the file itself will have be already opened and a
    pointer to a :c:type:`FILE` structure stored in :c:type:`ao_device.file`.

    If the plugin wishes to support automatic channel mapping, but it was not
    possible to set :c:type:`ao_device.output_matrix` in
    :c:func:`ao\_plugin\_device\_init`, the plugin should now set
    :c:type:`ao_device.inter_matrix` to the channel ordering that will be
    expected for buffers submitted to :c:func:`ao\_plugin\_play`.

    Plugins should use :c:type:`ao_device.output_channels` to determine the
    number of output channels to configure, not
    :c:type:`ao_sample_format.channels`.  :c:type:`ao_device.output_channels`
    is the number of channels libao will be submitting upon each call to
    :c:func:`ao\_plugin\_play`.  A plugin may alter the value of
    :c:type:`ao_device.output_channels` to demand a different number of
    channels if necessary (such as if a plugin is manufacturing a channel
    mapping manually).

    :param device:
        Pointer to pre-allocated device structure. The plugin should set its
        desired byte format in :c:type:`ao_device.driver_byte_format`.
    :param format:
        Output device sample format. :c:type:`ao_sample_format.byte_format`
        should be ignored as it is relevant only to the library core.
        :c:type:`ao_sample_format.channels` is relevant only if a plugin is
        constructing a channel mapping manually; otherwise
        :c:type:`ao_device.output` should be used as it is the actual number of
        channels that will be sent to :c:func:`ao\_plugin\_play`.
    :return:
        1 indicates success.
        0 indicates failure to open the device.
        :c:func:`ao\_plugin\_device\_clear` should be called to ensure the
        deallocate of all private data structures.

.. c:function:: int ao_plugin_play(ao_device *device, void *output_samples, uint32_t num_bytes)

    Write samples to the device. Channels are interleaved, and samples have the
    byte order requested by the plugin
    (:c:type:`ao_device.driver_byte_format`).

    :param device:
        Pointer to pre-allocated device structure. The plugin should set its
        desired byte format in :c:type:`ao_device.driver_byte_format`.
    :param output\_samples:
        Pointer to audio sample buffer.
    :param num\_bytes:
        Number of bytes in buffer.
    :return:
        1 indicates success.
        0 indicates failure to send the samples to the device.
        :c:func:`ao\_plugin\_close` should be called.

.. c:function:: int ao_plugin_set_option(ao_device *device, const char *key, const char *value)

    Set a new option key-value pair for a device. This will only be called after
    :c:func:`ao\_plugin\_device\_init` is called.  Unrecognized option keys are
    silently ignored, but invalid option values will cause the function to
    return a failing error code.

    :param device:
        Pointer to the device structure.
    :param key:
        A string holding the option key. The calling function retains
        ownership of this memory.
    :param value:
        A string holding the option value. The calling function retains
        ownership of this memory.
    :return:
        1 indicates success.
        0 indicates failure.
        :c:func:`ao\_plugin\_device\_clear` should be called to ensure the
        deallocate of all private data structures.

.. c:function:: int ao_plugin_test()

    Test if this driver could be used with only the default options. This function
    is used by the library core to determine if a driver is a possible candidate
    for the default driver. No devices are left open after this function returns.

    :return:
        1 indicates that the driver can be successfully opened with no
        options.
        0 indicates that the driver cannot be opened with no options.


