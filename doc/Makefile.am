## Process this with automake to create Makefile.in

AUTOMAKE_OPTIONS = foreign

docdir = $(datadir)/doc/$(PACKAGE)-$(VERSION)

# We list all of these as opposed to using a wildcard so that
# building outside the source directory works.
doc_DATA =  ao_append_option.html \
            ao_close.html \
            ao_default_driver_id.html \
            ao_device.html \
            ao_driver_id.html \
            ao_driver_info.html \
            ao_driver_info_list.html \
            ao_file_extension.html \
            ao_free_options.html \
            ao_info.html \
            ao_initialize.html \
            ao_is_big_endian.html \
            ao_open_file.html \
            ao_open_live.html \
            ao_option.html \
            ao_play.html \
            ao_plugin_close.html \
            ao_plugin_device_clear.html \
            ao_plugin_device_init.html \
            ao_plugin_driver_info.html \
            ao_plugin_file_extension.html \
            ao_plugin_open.html \
            ao_plugin_play.html \
            ao_plugin_set_option.html \
            ao_plugin_test.html \
            ao_sample_format.html \
            ao_shutdown.html \
            config.html \
            drivers.html \
            index.html \
            libao-api.html \
            overview.html \
            plugin-api.html \
            plugin-overview.html \
            ao_example.c \
            style.css

EXTRA_DIST = $(doc_DATA)
