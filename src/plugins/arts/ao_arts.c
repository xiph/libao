/*
 *
 *  ao_arts.c
 *
 *      Copyright (C) Rik Hemsley (rikkus) <rik@kde.org> 2000
 *
 *  This file is part of libao, a cross-platform library.  See
 *  README for a history of this source code.
 *
 *  libao is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2, or (at your option)
 *  any later version.
 *
 *  libao is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with GNU Make; see the file COPYING.  If not, write to
 *  the Free Software Foundation, 675 Mass Ave, Cambridge, MA 02139, USA.
 *
 */

#include <stdio.h>
#include <errno.h>

#include <artsc.h>
#include <ao/ao.h>
#include <ao/plugin.h>


static ao_info ao_arts_info =
{
	AO_TYPE_LIVE,
	"aRts output",
	"arts",
	"Rik Hemsley (rikkus) <rik@kde.org>",
	"Outputs to the aRts soundserver.",
	AO_FMT_NATIVE,
#ifdef HAVE_ARTS_SUSPENDED
	45,
#else
	15,
#endif
	NULL,
	0
};


typedef struct ao_arts_internal
{
	arts_stream_t stream;
} ao_arts_internal;


int ao_plugin_test()
{
	if (arts_init() == 0) {
#ifdef HAVE_ARTS_SUSPENDED
		if (arts_suspended() == 1) {
			arts_free();
			return 0;
		}
#endif
		arts_free();
		return 1;
	} else
		return 0;
}


ao_info *ao_plugin_driver_info(void)
{
	return &ao_arts_info;
}


int ao_plugin_device_init(ao_device *device)
{
	ao_arts_internal *internal;

	internal = (ao_arts_internal *) malloc(sizeof(ao_arts_internal));

	if (internal == NULL)	
		return 0; /* Could not initialize device memory */
	
	device->internal = internal;

	return 1; /* Memory alloc successful */
}


int ao_plugin_set_option(ao_device *device, const char *key, const char *value)
{
	return 1; /* No options */
}

int ao_plugin_open(ao_device *device, ao_sample_format *format)
{
	ao_arts_internal *internal = (ao_arts_internal *) device->internal;
	int errorcode;

	errorcode = arts_init();

	if (0 != errorcode)
	{
		return 0; /* Could not connect to server */
	}

	device->driver_byte_format = AO_FMT_NATIVE;

	internal->stream = arts_play_stream(format->rate, 
					    format->bits, 
					    format->channels, 
					    "libao stream");
	return 1;
}


int ao_plugin_play(ao_device *device, const char *output_samples, 
		uint_32 num_bytes)
{
	ao_arts_internal *internal = (ao_arts_internal *) device->internal;

	if (arts_write(internal->stream, output_samples, 
		       num_bytes) < num_bytes)
		return 0;
	else
		return 1;
}


int ao_plugin_close(ao_device *device)
{
	ao_arts_internal *internal = (ao_arts_internal *) device->internal;
	arts_close_stream(internal->stream);
	arts_free();

	return 1;
}


void ao_plugin_device_clear(ao_device *device)
{
	ao_arts_internal *internal = (ao_arts_internal *) device->internal;

	free(internal);
}
