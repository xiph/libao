/*
 *
 *  ao_oss.c
 *
 *      Original Copyright (C) Aaron Holtzman - May 1999
 *      Modifications Copyright (C) Stan Seibert - July 2000, June 2001
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
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <math.h>
#if defined(__OpenBSD__) || defined(__NetBSD__)
#include <soundcard.h>
#elif defined(__FreeBSD__)
#include <machine/soundcard.h>
#else
#include <sys/soundcard.h>
#endif
#include <sys/ioctl.h>

#include "ao/ao.h"
#include "ao/plugin.h"


static char *ao_oss_options[] = {"dsp"};
static ao_info ao_oss_info =
{
	AO_TYPE_LIVE,
	"OSS audio driver output ",
	"oss",
	"Aaron Holtzman <aholtzma@ess.engr.uvic.ca>",
	"Outputs audio to the Open Sound System driver.",
	AO_FMT_NATIVE,
	20,
	ao_oss_options,
	1
};


typedef struct ao_oss_internal {
	char *dev;
	int fd;
} ao_oss_internal;


/*
 * open either the devfs device or the traditional device and return a
 * file handle.  Also strdup() path to the selected device into
 * *dev_path.  Assumes that *dev_path does not need to be free()'ed
 * initially.
 */
int _open_default_oss_device (char **dev_path)
{
	int fd;

	/* default: first try the devfs path */
	*dev_path = strdup("/dev/sound/dsp");
	fd = open(*dev_path, O_WRONLY);

	if(fd < 0) 
	{
		/* no? then try the traditional path */
		char *err = strdup(strerror(errno));
		char *dev = strdup(*dev_path);
		free(*dev_path);
		*dev_path = strdup("/dev/dsp");
		fd = open(*dev_path,O_WRONLY);

		if(fd < 0) 
		{
		  /*			fprintf(stderr,
				"libao - error: Could not open either default device:\n"
				"  %s - %s\n"
				"  %s - %s\n",
				err, dev,
				strerror(errno), *dev_path); */
			free(err);
			free(dev);
			free(*dev_path);
			*dev_path = NULL;
		}
	}

	return fd;
}


int ao_plugin_test()
{
	char *dev_path;
	int fd;

	if ( (fd = _open_default_oss_device(&dev_path)) < 0 )
		return 0; /* Cannot use this plugin with default parameters */
	else {
		close(fd);
		return 1; /* This plugin works in default mode */
	}
}


ao_info *ao_plugin_driver_info(void)
{
	return &ao_oss_info;
}


int ao_plugin_device_init(ao_device *device)
{
	ao_oss_internal *internal;

	internal = (ao_oss_internal *) malloc(sizeof(ao_oss_internal));

	if (internal == NULL)	
		return 0; /* Could not initialize device memory */
	
	internal->dev = NULL;
	
	device->internal = internal;

	return 1; /* Memory alloc successful */
}

int ao_plugin_set_option(ao_device *device, const char *key, const char *value)
{
	ao_oss_internal *internal = (ao_oss_internal *) device->internal;


	if (!strcmp(key, "dsp")) {
		/* Free old string in case "dsp" set twice in options */
		free(internal->dev); 
		internal->dev = strdup(value);
	}

	return 1;
}


/*
 * open the audio device for writing to
 */
int ao_plugin_open(ao_device *device, ao_sample_format *format)
{
	ao_oss_internal *internal = (ao_oss_internal *) device->internal;
	int tmp;

	/* Open the device driver */
	
	if (internal->dev != NULL) {
		/* open the user-specified path */
		internal->fd = open(internal->dev, O_WRONLY);

		if(internal->fd < 0) {
		  /* fprintf(stderr,"libao - %s: Opening audio device %s\n",
		     strerror(errno), internal->dev); */
			return 0;  /* Cannot open device */
		}

	} else {
		internal->fd = _open_default_oss_device(&internal->dev);
		if (internal->fd < 0)
			return 0;  /* Cannot open default device */
	}

	/* Now set all of the parameters */

	switch (format->channels)
	{
	case 1: tmp = 0;
		break;
	case 2: tmp = 1;
		break;
	default:fprintf(stderr,"libao - Unsupported number of channels: %d.",
			format->channels);
		goto ERR;
	}
	ioctl(internal->fd,SNDCTL_DSP_STEREO,&tmp);
	
	/* To eliminate the need for a swap buffer, we set the device
	   to use whatever byte format the client selected. */
	switch (format->bits)
	{
	case 8: tmp = AFMT_S8;
		break;
        case 16: tmp = device->client_byte_format == AO_FMT_BIG ? 
		   AFMT_S16_BE : AFMT_S16_LE;
	        device->driver_byte_format = device->client_byte_format;
	        break;
	default:fprintf(stderr,"libao - Unsupported number of bits: %d.",
			format->bits);
		goto ERR;
	}
	ioctl(internal->fd,SNDCTL_DSP_SAMPLESIZE,&tmp);
	
	tmp = format->rate;
	ioctl(internal->fd,SNDCTL_DSP_SPEED, &tmp);
	
	return 1; /* Open successful */

 ERR:
	close(internal->fd);
	return 0; /* Failed to open device */
}

/*
 * play the sample to the already opened file descriptor
 */
int ao_plugin_play(ao_device *device, const char *output_samples, 
		uint_32 num_bytes)
{
	ao_oss_internal *internal = (ao_oss_internal *) device->internal;

	if (write(internal->fd, output_samples, num_bytes) < 0)
		return 0;
	else
		return 1;
}


int ao_plugin_close(ao_device *device)
{
	ao_oss_internal *internal = (ao_oss_internal *) device->internal;
	close(internal->fd);

	return 1;
}


void ao_plugin_device_clear(ao_device *device)
{
	ao_oss_internal *internal = (ao_oss_internal *) device->internal;

	free(internal->dev);
	free(internal);
}
