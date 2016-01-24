API reference
=============

Libao is designed to make it easy to do simple audio output using
various audio devices and libraries.  For this reason, complex audio
control features are missing and will probably never be added.
However, if you just want to be able to open whatever audio device is
available and play sound, libao should be just fine.

The libao API makes a distinction between drivers and devices.  A
driver is a set of functions that allow audio to be played on a
particular platform (i.e. Solaris, ESD, etc.).  A device is a
particular output target that uses a driver.  In addition, libao
distinguishes between *live* output drivers, which write audio to
playback devices (sound cards, etc.), and *file* output drivers,
which write audio to disk in a particular format.

To use libao in your program, you need to follow these steps:

- Include the :file:`ao/ao.h` header into your program.
- Call :c:func:`ao_initialize` to initialize the library.  This loads the
  plugins from disk, reads the libao :doc:`configuration
  files <config>`, and identifies an appropriate default output driver if none is
  specified in the configuration files.
- Call :c:func:`ao_default_driver_id` to get the
  ID number of the default output driver.  This may not be successful if no
  audio hardware is available, it is in use, or is not in the "standard"
  configuration.  If you want to specify a particular output driver, you may
  call :c:func:`ao_driver_id` with a string corresponding to the short name of
  the device (i.e. "oss", "wav", etc.) instead.
- If you are using the default device, no extra options are needed.
  However, if you wish to to pass special options to the driver, you will need
  to:

  - Create an option list pointer of type :c:type:`ao_option *` and
    initialize it to :c:macro:`NULL`.
  - Through successive calls to :c:func:`ao_append_option`, add any
    driver-specific options you need.  Note that the options take the form of
    key/value pairs where supported keys are listed in the :doc:`driver
    documentation <drivers>`.
- Call :c:func:`ao_open_live` and save the returned device pointer.  If you are
  using a file output driver, you will need to call :c:func:`ao_open_file`
  instead.
- Call :c:func:`ao_play` to output each block of audio.
- Call :c:func:`ao_close` to close the device.  Note that this will
  automatically free the memory that was allocated for the device.  Do not
  attempt to free the device pointer yourself!
- Call :c:func:`ao_shutdown` to close the library.

Setup/Teardown
--------------

.. c:function:: void ao_initialize()

    This function initializes the internal libao data structures and loads all
    of the available plugins. The system and user :doc:`configuration files
    <config>` are also read at this time if available.
    :c:func:`ao\_initialize` must be called in the main thread and before any
    other libao functions can be used.

    More background on initialization in the main thread:
    :c:func:`ao\_initialize` must be called in the main thread because several
    sound system interfaces used by libao must be initialized in the main
    thread. One example is the system aRts interface, which stores some global
    state in thread-specific keys that it fails to delete on shutdown. If aRts
    is initialized in a non-main thread that later exits, these undeleted keys
    will cause a segmentation fault.

    .. note::
        Do not invoke this function more than once before calling
        :c:func:`ao\_shutdown`. If you want to reload the configuration files
        without restarting your program, first call :c:func:`ao\_shutdown`,
        then call :c:func:`ao\_initialize` again.

.. c:function:: void ao_shutdown()

    This function unloads all of the plugins and deallocates any internal data
    structures the library has created. It should be called prior to program exit.

    .. note::

        Do not invoke this function before closing all of the open devices.
        You may call :c:func:`ao\_initialize` after calling
        :c:func:`ao\_shutdown`.


Device Setup/Playback/Teardown
------------------------------

.. c:function:: int ao_append_global_option(const char *key, const char *value)

    Append a key-value pair to the internal linked list of global
    options obeyed by libao itself, as well as passed to any driver in
    use.  The key and value strings are duplicated into newly allocated
    memory, so the calling function retains ownership of the string
    parameters.

    :param key: A string holding the option key.
    :param value: A string holding the option value.
    :return:
        - 1 indicates success.
        - 0 indicates memory allocation failure.

.. c:function:: int ao_append_option(ao_option **options, const char *key, const char *value)

    Append a key-value pair to a linked list of options.  The key and
    value strings are duplicated into newly allocated memory, so the
    calling function retains ownership of the string parameters.

    :param options: Address of the pointer to the head of the option list.  For an empty list <tt>*options</tt> will be NULL.</dd>
    :param key: A string holding the option key.
    :param value: A string holding the option value.
    :return: 
        - 1 indicates success.
        - 0 indicates memory allocation failure.

.. c:function:: int ao_close(ao_device *device)

    Closes the audio device and frees the memory allocated by the
    device structure.

    :param device:
        Pointer to device structure as returned by :c:func:`ao_open_live` or
        :c:func:`ao_open_file`
    :return:
        - 1 indicates remaining data written correctly and device closed.
        - 0 indicates an error while the device was being closed.  If this
          device was writing to a file, the file may be corrupted.

.. c:function:: void ao_free_options(ao_option *options)

    Free all of the memory allocated to an option list, including the key and
    value strings.

    :param options:
        Pointer to the head of the option list. If NULL is passed, the function
        does nothing.

.. c:function:: ao_device* ao_open_file(int driver_id, const char *filename, int overwrite, ao_sample_format *format, ao_option *options)

    Open a file for audio output. The file format is determined by the audio
    driver used.

    :param driver\_id:
        The ID number of the driver as returned by :c:func:`ao\_driver\_id`.
    :param filename:
        Name of the file in which to store the audio. The special filename
        "-" corresponds to ``stdout``.
    :param overwrite:
        If non-zero, the file is automatically overwritten. If zero, then a
        preexisting file will cause the function to report a failure.
    :param format:
        Pointer to a struct describing the sample format. The caller retains
        ownership of this structure.
    :param options:
        A linked list of options to be passed to the driver or NULL if no
        options are needed. Unsupported options are ignored.

    :return:
        non-NULL pointer inicates success. This pointer must be passed in
        subsequent calls to :c:func:`ao\_play` and :c:func:`ao\_close`.

        NULL indicates failure. ``errno`` will contain the specific cause of
        the failure:

        -  AO\_ENODRIVER - No driver corresponds to ``driver_id``.
        -  AO\_ENOTFILE - This driver is not a file output driver.
        -  AO\_EBADOPTION - A valid option key has an invalid value.
        -  AO\_EOPENFILE - Cannot open the file.
        -  AO\_EFILEEXISTS - The file already exists. (Only if
           ``overwrite == 0``)
        -  AO\_EFAIL - Any other cause of failure.

    .. note::
        Live output drivers cannot be used with this function. Use
        :c:func:`ao\_open\_live` instead. Some file formats (notably .WAV)
        cannot be correctly written to non-seekable files (like ``stdout``).

        When passed to :c:func:`ao\_open\_file`,
        :c:type:`ao_sample_format.byte_format` does not specify the byte
        format that will be used in the file *output*, just the input sample
        format.


.. c:function:: ao_device* ao_open_live(int driver_id, ao_sample_format *format, ao_option *options)

    Open a live playback audio device for output.

    :param driver\_id:
        The ID number of the driver as returned by either
        :c:func:`ao\_driver\_id` or :c:func:`ao\_default\_driver\_id`.
    :param format:
        Pointer to a struct describing the sample format. The caller retains
        ownership of this structure.
    :param options:
        A linked list of options to be passed to the driver or NULL if no
        options are needed. Unsupported options are ignored.
    :return:
        non-NULL pointer inicates success. This pointer must be passed in
        subsequent calls to :c:func:`ao\_play` and :c:func:`ao\_close`.

        NULL indicates failure. ``errno`` will contain the specific cause of
        the failure:

        -  AO\_ENODRIVER - No driver corresponds to ``driver_id``.
        -  AO\_ENOTLIVE - This driver is not a live output device.
        -  AO\_EBADOPTION - A valid option key has an invalid value.
        -  AO\_EOPENDEVICE - Cannot open the device (for example, if
           /dev/dsp cannot be opened for writing).
        -  AO\_EFAIL - Any other cause of failure.

    .. note::
        File output drivers cannot be used with this function. Use
        :c:func:`ao\_open\_file` instead.

.. c:function:: int ao_play(ao_device *device, char *output_samples, uint32_t num_bytes)

    Play a block of audio data to an open device. Samples are interleaved by
    channels (Time 1, Channel 1; Time 1, Channel 2; Time 2, Channel 1; etc.) in
    the memory buffer.

    :param device:
        Pointer to device structure as returned by :c:func:`ao\_open\_live` or
        :c:func:`ao\_open\_file`
    :param output\_samples:
        Memory buffer containing audio data.
    :param num\_bytes:
        Number of bytes of audio data in the memory buffer.
    :return: non-zero value indicates success.
        0 indicates failure. The device should be closed.

Driver Information
------------------

.. c:function:: int ao_default_driver_id()

    Returns the ID number of the default live output driver.  If the
    :doc:`configuration files <config>` specify a default driver, its ID is
    returned, otherwise the library tries to pick a live output driver that
    will work on the host platform.

    :return:
        - a non-negative value is the ID number of the default driver.
        - -1 indicates failure to find a usable audio output device.

    .. note::
        If no default device is available, you may still use the null device
        to test your application.

.. c:function:: int ao_driver_id(char *short_name)

    Looks up the ID number for a driver based upon its short name. The ID
    number is need to open the driver or get info on it.

    :param short_name:
        The short name of the driver. See the :doc:`list of drivers <drivers>`
        for valid short names.
    :return:
        - a non-negative value is the ID number of the driver.
        - -1 indicates failure. No driver by that name exists.

.. c:function:: ao_info* ao_driver_info(int driver_id)

    Get information about a particular driver.

    :param driver\_id: The ID number of the driver as returned by either
        :c:func:`ao\_driver\_id` or :c:func:`ao\_default\_driver\_id`.
    :return: non-NULL pointer indicates success. It points to a static info
        structure that should not be modified under any circumstances.
        NULL indicates failure because ``driver_id`` does not correspond to
        an actual driver

.. c:function:: ao_info** ao_driver_info_list(int *driver_count)

    Get a list of the :c:type:`ao\_info` structures for all of the
    available drivers. :c:func:`ao\_driver\_info\_list` passes back the number
    of drivers through the ``driver_count`` pointer.

    :param driver\_count: Pointer to an integer that will contain the number of
        drivers.
    :return:
        An array of driver info structures with ``*driver_count`` elements.


.. c:function:: const char* ao_file_extension(int driver_id)

    Returns the normal file extension associated with a particular driver (like
    "wav" or "au"). This is just an information function to allow library users
    to guess appropriate file names. You can safely ignore the recommended
    extension.

    :param driver\_id:
        The ID number of the driver as returned by either
        :c:func:`ao\_driver\_id` or :c:func:`ao\_default\_driver\_id`.
    :return:
        pointer to string containing suggested file extension.  NULL if this
        driver has no file extension associated with it or if this driver does
        not exist.

Miscellaneous
-------------

.. c:function:: int ao_is_big_endian(void)

    Test if this computer uses big-endian byte ordering. Provided as a
    convenience function.

    :return:
        1 indicates that this platform using big-endian byte ordering.
        0 indicates that this computer uses little-endian byte ordering.

Data structures
---------------

.. c:type:: ao_option

    A linked list structure to hold option key/value pairs as standard
    null-terminated C strings.  It is used by :c:func:`ao_open_live` and
    :c:func:`ao_open_file`.

    .. c:member:: char *key
    .. c:member:: char *value
    .. c:member:: struct ao_option *next

        Pointer to the next option in the list.  Set to :c:macro:`NULL` to mark
        the end of the list.

.. c:type:: ao_device

    This structure holds all of the data for an open device.  

    .. versionchanged:: 0.9.0

        The ao_device structure is opaque now.

.. c:type:: ao_sample_format

    This structure describes the format of audio samples.

    .. c:member:: int bits
    .. c:member:: int rate
    .. c:member:: int channels
    .. c:member:: int byte_format

        Specifies the ordering of the sample bytes. The value of this member
        is ignored when samples have only 8 bits. Use the following
        constants:

        -  AO\_FMT\_LITTLE - Samples are in little-endian order.
        -  AO\_FMT\_BIG - Samples are in big-endian order.
        -  AO\_FMT\_NATIVE - Samples are in the native ordering of the
           computer.

    .. c:member:: char *matrix

        .. versionadded:: 0.9.0

        Specifies the mapping of input channels to intended speaker/ouput
        location (or NULL to specify no mapping). The matrix is specified as
        a comma seperated list of channel locations equal to the number and
        in the order of the input channels. The channel mnemonics are as
        follows:

        -  L - Left speaker, located forward and to the left of the
           listener.
        -  R - Right speaker, located forward and to the right of the
           listener.
        -  C - Center speaker, located directly forward of the listener
           between the Left and Right speakers.
        -  M - Monophonic, a virtual speaker for single-channel output.
        -  CL - Left of Center speaker (used in some Widescreen formats),
           located forward of the listener between the Center and Left
           speakers. Alternatively referred to as 'Left Center'.
        -  CR - Right of Center speaker (used in some Widescreen formats),
           located forward of the listener between the Center and Right
           speakers. Alternatively referred to as 'Right Center'.
        -  BL - Back Left speaker, located behind and to the left of the
           listener. Alternatively called 'Left Surround' (primarily by
           Apple) or 'Surround Rear Left' (primarily by Dolby).
        -  BR - Back Right speaker, located behind and to the right of the
           listener. Alternatively called 'Right Surround' (primarily by
           Apple) or 'Surround Rear Right' (primarily by Dolby).
        -  BC - Back Center speaker, located directly behind the listener.
           Alternatively called 'Center Surround' (primarily by Apple) or
           'Surround Rear Center' (primarily by Dolby).
        -  SL - Side Left speaker, located directly to the listener's left
           side. The Side Left speaker is also referred to as 'Left Surround
           Direct' (primarily by Apple) or 'Surround Left' (primarily by
           Dolby)
        -  SR - Side Right speaker, located directly to the listener's right
           side. The Side Right speaker is also referred to as 'Right
           Surround Direct' (primarily by Apple) or 'Surround Right'
           (primarily by Dolby)
        -  LFE - Low Frequency Effect (subwoofer) channel. This is channel
           is usually lowpassed and meant only for bass, though in some
           recent formats it is a discrete, full-range channel. Microsoft
           calls this the 'Low Frequency' channel.
        -  A1, A2, A3, A4 - 'auxiliary' channels, not mapped to a location.
           Intended for driver-specific use.
        -  X - Unused/Invalid channel, to be dropped in the driver and not
           output to any speaker.

        .. note::
            Note that the 'surround' speakers referred to in other systems can
            be either the side or back speakers depending on vendor. For
            example, Apple calls the back speakers 'surround' and the side
            speakers 'direct surround'. Dolby calls the back speakers 'surround
            rear' and the side speakers 'surround', resulting in a direct naming
            conflict. For this reason, libao explicitly refers to speakers as
            'back' and 'side' rather than 'surround'.

        Common examples of channel orderings:

        -  "L,R" - Stereo ordering in virtually all file formats
        -  "L,R,BL,BR" - Quadraphonic ordering for most file formats
        -  "L,R,C,LFE,BR,BL" - channel order of a 5.1 WAV or FLAC file
        -  "L,R,C,LFE,BR,BL,SL,SR" - channel order of a 7.1 WAV or FLAC file
        -  "L,C,R,BR,BL,LFE" - channel order of a six channel (5.1) Vorbis I
           file
        -  "L,C,R,BR,BL,SL,SR,LFE" - channel order of an eight channel (7.1)
           Vorbis file
        -  "L,CL,C,R,RC,BC" - channel order of a six channel AIFF[-C] file

        Channel mappings for most formats are usually not tied to a single
        channel matrix (there are a few exceptions like Vorbis I, where the
        number of channels always maps to a specific order); the above
        examples cannot be blindly applied to a given file type and number
        of channels. The mapping must usually be read or intuited from the
        input.

.. c:type:: ao_info

    This structure describes the attributes of an output driver.

    .. c:member:: int type

        The output type of the driver:

        -  AO\_TYPE\_LIVE - Live output.
        -  AO\_TYPE\_FILE - File output.

    .. c:member:: char *name

        A longer name for the driver which may contain whitespace, but no
        newlines. It is useful for telling users what output driver is in
        use.

    .. c:member:: char *short\_name

        A short identifier for the driver. The short name contains only
        alphanumeric characters, and no whitespace. It is used to look up
        the driver ID number using :c:func:`ao\_driver\_id`.

    .. c:member:: int preferred\_byte\_format

        Specifies the preferred ordering of the sample bytes. Using the
        driver with this byte format usually results in slightly less memory
        usage and slightly less CPU usage because a swap buffer will not be
        needed. See :c:func:`ao\_sample\_format` for a list of allowed values.

    .. c:member:: int priority

        A positive integer ranking how likely it is for this driver to be
        the default. The default driver will be a functioning driver with
        highest priority. See the :doc:`drivers document <drivers>` for
        more explanation.

    .. c:member:: char *comment

        Pointer to a driver comment string (possibly :c:macro:`NULL`). It may
        contain newlines.

    .. c:member:: char **options

        An array of strings which list the option keys accepted by this
        driver.

    .. c:member:: int option\_count

        Number of strings in :c:type:`ao_info.options` array.


