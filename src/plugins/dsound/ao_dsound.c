/*
 * $Id: ao_dsound.c,v 1.1 2001/09/05 19:28:07 cwolf Exp $
 * $Name:  $
 *
 * Skeleton DirectSound plugin, just a place holder until test code 
 * is integrated.
 *
 * 04 Sept 2001, Chris Wolf - create.
 * 
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <ao/ao.h>
#include <ao/plugin.h>

#include <dsound.h>

static char *ao_dsound_options[] = {"hwnd"};

static ao_info ao_dsound_info =
{
	AO_TYPE_LIVE,
	"DirectSound output",
	"dsound",
	"Chris Wolf <code@starclass.com>",
	"Output via DirectSound",
	AO_FMT_NATIVE,
	10,
	ao_dsound_options,
	1
};


typedef struct ao_dsound_internal
{
  HWND          hwnd;           /* Controlling window handle */
	LPDIRECTSOUND lpDS;           /* DirectSound object */
  LPDIRECTSOUNDBUFFER lpDSBuf;  /* DirectSound buffer object */
} ao_dsound_internal;

int ao_plugin_test()
{
		return 1;
}

ao_info *ao_plugin_driver_info(void)
{
	return &ao_dsound_info;
}

int ao_plugin_device_init(ao_device *device)
{
	ao_dsound_internal *internal;

	internal = (ao_dsound_internal *) malloc(sizeof(ao_dsound_internal));

	if (internal == NULL)	
		return 0; /* Could not initialize device memory */
	
	device->internal = internal;

	return 1; /* Memory alloc successful */
}

int ao_plugin_set_option(ao_device *device, const char *key, const char *value)
{
	ao_dsound_internal *internal = (ao_dsound_internal *) device->internal;

	if (!strcmp(key, "HWND")) 
  {
		internal->hwnd = *(HWND *)atoi(value);
	}

	return 1;
}

int ao_plugin_open(ao_device *device, ao_sample_format *format)
{
	ao_dsound_internal *internal = (ao_dsound_internal *) device->internal;


	return 1;
}

int ao_plugin_play(ao_device *device, const char* output_samples, 
		uint_32 num_bytes)
{
	ao_dsound_internal *internal = (ao_dsound_internal *) device->internal;

	return 1;
}

int ao_plugin_close(ao_device *device)
{
	ao_dsound_internal *internal = (ao_dsound_internal *) device->internal;

	return 1;
}

void ao_plugin_device_clear(ao_device *device)
{
	ao_dsound_internal *internal = (ao_dsound_internal *) device->internal;

  if (device->internal)
	  free(internal);
}
