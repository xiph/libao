/*
 *
 *  ao_esd.c
 *
 *      Copyright (C) Stan Seibert - July 2000, July 2001
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
#include <unistd.h>
#include <stdlib.h>
#include <dirent.h>
#include <sys/stat.h>
#include <string.h>

#include <esd.h>
#include <ao/ao.h>
#include <ao/plugin.h>

static char *ao_esd_options[] = {"host"};
static ao_info ao_esd_info =
{
	AO_TYPE_LIVE,
	"ESounD output",
	"esd",
	"Stan Seibert <volsung@asu.edu>",
	"Outputs to the Enlightened Sound Daemon.",
	AO_FMT_NATIVE,
	40,
	ao_esd_options,
	1
};


typedef struct ao_esd_internal
{
	int sock;
	char *host;
} ao_esd_internal;


int ao_plugin_test()
{
	int sock;

	/* don't wake up the beast while detecting */
	putenv("ESD_NO_SPAWN=1"); 
	sock = esd_open_sound(NULL);
	if (sock < 0)
		return 0;
	if (esd_get_standby_mode(sock) != ESM_RUNNING) {
		esd_close(sock);
		return 0;
	}

	esd_close(sock);
	return 1;
}


ao_info *ao_plugin_driver_info(void)
{
	return &ao_esd_info;
}


int ao_plugin_device_init(ao_device *device)
{
	ao_esd_internal *internal;

	internal = (ao_esd_internal *) malloc(sizeof(ao_esd_internal));

	if (internal == NULL)	
		return 0; /* Could not initialize device memory */
	
	internal->host = NULL;
	
	device->internal = internal;

	return 1; /* Memory alloc successful */
}

int ao_plugin_set_option(ao_device *device, const char *key, const char *value)
{
	ao_esd_internal *internal = (ao_esd_internal *) device->internal;

	if (!strcmp(key, "host")) {
		if(internal->host) free(internal->host);
		internal->host = strdup(value);
	}

	return 1;
}

int ao_plugin_open(ao_device *device, ao_sample_format *format)
{
	ao_esd_internal *internal = (ao_esd_internal *) device->internal;
	int esd_bits;
	int esd_channels;
	int esd_mode = ESD_STREAM;
	int esd_func = ESD_PLAY;
	int esd_format;

	switch (format->bits)
	{
	case 8  : esd_bits = ESD_BITS8;
		  break;
	case 16 : esd_bits = ESD_BITS16;
		  break;
	default : return 0;
	}

	switch (format->channels)
	{
	case 1 : esd_channels = ESD_MONO;
		 break;
	case 2 : esd_channels = ESD_STEREO;
		 break;
	default: return 0;
	}
	
	esd_format = esd_bits | esd_channels | esd_mode | esd_func;

	internal->sock = esd_play_stream(esd_format, format->rate, 
					 internal->host,
					 "libao output");
	if (internal->sock < 0)
		return 0; /* Could not contact ESD server */
	
	device->driver_byte_format = AO_FMT_NATIVE;

	return 1;
}

int ao_plugin_play(ao_device *device, const char* output_samples, 
		uint_32 num_bytes)
{
	ao_esd_internal *internal = (ao_esd_internal *) device->internal;

	if (write(internal->sock, output_samples, num_bytes) < 0)
		return 0;
	else
		return 1;
}


int ao_plugin_close(ao_device *device)
{
	ao_esd_internal *internal = (ao_esd_internal *) device->internal;

	esd_close(internal->sock);

	return 1;
}


void ao_plugin_device_clear(ao_device *device)
{
	ao_esd_internal *internal = (ao_esd_internal *) device->internal;

	if(internal->host) free(internal->host);
	free(internal);
}
