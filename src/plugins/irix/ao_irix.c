/*
 *
 *  ao_irix.h
 *
 *      Original Copyright (C) Aaron Holtzman - May 1999
 *      Port to IRIX by Jim Miller, SGI - Nov 1999
 *      Modifications Copyright (C) Stan Seibert - July 2000, July 2001
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

#include <audio.h>

#include <ao/ao.h>


typedef struct ao_irix_internal {
	static ALport alport = 0;
	static ALconfig alconfig = 0;
	static int bytesPerWord = 1;
	static int nChannels = 2;
} ao_irix_internal;


static ao_info ao_irix_info =
{
	AO_TYPE_LIVE,
	"Irix audio output ",
	"irix",
	"Jim Miller <???@sgi.com>",
	"WARNING: This driver is untested!"
	AO_FMT_NATIVE,
	20,
	NULL,
	1
};


int ao_plugin_test()
{
	char *dev_path;
	ALport port;


	if ( !(port = alOpenPort("libao test", "w", 0)) )
		return 0; /* Cannot use this plugin with default parameters */
	else {
		alClosePort(port);
		return 1; /* This plugin works in default mode */
	}
}


ao_info *ao_plugin_driver_info(void)
{
	return &ao_irix_info;
}


int ao_plugin_device_init(ao_device *device)
{
	ao_irix_internal *internal;

	internal = (ao_irix_internal *) malloc(sizeof(ao_irix_internal));

	if (internal == NULL)	
		return 0; /* Could not initialize device memory */

	internal->alconfig = alNewConfig();
		
	device->internal = internal;

	return 1; /* Memory alloc successful */
}


int ao_plugin_set_option(ao_device *device, const char *key, const char *value)
{
	return 1; /* No options */
}


/*
 * open the audio device for writing to
 */
int ao_plugin_open(ao_device *device, ao_sample_format *format)
{
	ao_irix_internal *internal = (ao_irix_internal *) device->internal;
	ALpv params[2];
	int  dev = AL_DEFAULT_OUTPUT;
	int  wsize = AL_SAMPLE_16;


	internal->nChannels = channels;

	if (alSetQueueSize(internal->alconfig, BUFFER_SIZE) < 0) {
		fprintf(stderr, "alSetQueueSize failed: %s\n", 
		alGetErrorString(oserror()));
		return 0;
	}

	if (alSetChannels(internal->alconfig, channels) < 0) {
		fprintf(stderr, "alSetChannels(%d) failed: %s\n", 
		channels, alGetErrorString(oserror()));
		return 0;
	}
	
	if (alSetDevice(internal->alconfig, dev) < 0) {
		fprintf(stderr, "alSetDevice failed: %s\n", 
		alGetErrorString(oserror()));
		return 0;
	}
	
	if (alSetSampFmt(internal->alconfig, AL_SAMPFMT_TWOSCOMP) < 0) {
		fprintf(stderr, "alSetSampFmt failed: %s\n", 
		alGetErrorString(oserror()));
		return 0;
	}

	switch (format->bits) {
	case 8:         
		internal->bytesPerWord = 1;
		wsize = AL_SAMPLE_8;
		break;
		
	case 16: 
		internal->bytesPerWord = 2;
		wsize = AL_SAMPLE_16;
		break;
		
	case 24:
		internal->bytesPerWord = 4;
		wsize = AL_SAMPLE_24;
		break;
		
	default:
	    fprintf(stderr,"Irix audio: unsupported bit with %d\n", bits);
		break;
	}


	internal->alport = alOpenPort("libao", "w", 0);

	if (!internal->alport) {
		fprintf(stderr, "alOpenPort failed: %s\n", 
			alGetErrorString(oserror()));
		return 0;
	}


	if (alSetWidth(internal->alconfig, wsize) < 0) {
		fprintf(stderr, "alSetWidth failed: %s\n", alGetErrorString(oserror()));
		alClosePort(internal->alport);
		return 0;
	}
	
	params[0].param = AL_RATE;
	params[0].value.ll = alDoubleToFixed((double)rate);
	params[1].param = AL_MASTER_CLOCK;
	params[1].value.i = AL_CRYSTAL_MCLK_TYPE;
	if ( alSetParams(dev, params, 1) < 0) {
		printf("alSetParams() failed: %s\n", alGetErrorString(oserror()));
		alClosePort(internal->alport);
		return 0;
	}
	
	device->driver_byte_format = AO_FMT_NATIVE;
	
	return 1;
}

/*
 * play the sample to the already opened file descriptor
 */
int ao_plugin_play(ao_device *device, const char *output_samples, 
		uint_32 num_bytes)
{
	ao_irix_internal *internal = (ao_irix_internal *) device->internal;
	
	alWriteFrames(internal->alport, output_samples, num_bytes); 

	return 1; /* FIXME: Need to check if the above function failed */
}

int ao_plugin_close(ao_device *device)
{
	ao_irix_internal *internal = (ao_irix_internal *) device->internal;

	alClosePort(internal->alport);

	return 1;
}


void ao_plugin_device_clear(ao_device *device)
{
	ao_irix_internal *internal = (ao_irix_internal *) device->internal;

	alFreeConfig(internal->alconfig);
	free(internal);
}
