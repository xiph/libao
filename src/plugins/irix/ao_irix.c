/*
 *
 *  ao_irix.h
 *
 *      Original Copyright (C) Aaron Holtzman - May 1999
 *      Port to IRIX by Jim Miller, SGI - Nov 1999
 *      Modifications Copyright (C) Stan Seibert - July 2000
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


typedef struct ao_irix_internal_s {
	static ALport alport = 0;
	static ALconfig alconfig = 0;
	static int bytesPerWord = 1;
	static int nChannels = 2;
} ao_irix_internal_t;


ao_info_t ao_irix_info =
{
	"Irix audio output ",
	"irix",
	"Jim Miller <???@sgi.com>",
	"WARNING: This driver is untested!"
};


/*
 * open the audio device for writing to
 */
ao_internal_t *plugin_open(uint_32 bits, uint_32 rate, uint_32 channels, ao_option_t *options)
{
	ALpv params[2];
	int  dev = AL_DEFAULT_OUTPUT;
	int  wsize = AL_SAMPLE_16;
	ao_irix_internal_t *state;

	state = malloc(sizeof(ao_irix_internal_t));
	if (state == NULL)
		return NULL;
	

	state->nChannels = channels;


	state->alconfig = alNewConfig();

	if (alSetQueueSize(state->alconfig, BUFFER_SIZE) < 0) {
		fprintf(stderr, "alSetQueueSize failed: %s\n", 
			alGetErrorString(oserror()));
		return 0;
	}

	if (alSetChannels(state->alconfig, channels) < 0) {
		fprintf(stderr, "alSetChannels(%d) failed: %s\n", 
			channels, alGetErrorString(oserror()));
		return 0;
	}
	
	if (alSetDevice(state->alconfig, dev) < 0) {
		fprintf(stderr, "alSetDevice failed: %s\n", 
			alGetErrorString(oserror()));
		return 0;
	}
	
	if (alSetSampFmt(state->alconfig, AL_SAMPFMT_TWOSCOMP) < 0) {
		fprintf(stderr, "alSetSampFmt failed: %s\n", 
			alGetErrorString(oserror()));
		return 0;
	}

	state->alport = alOpenPort("AC3Decode", "w", 0);

	if (!state->alport) {
		fprintf(stderr, "alOpenPort failed: %s\n", 
			alGetErrorString(oserror()));
		return 0;
	}

	switch (bits) {
	case 8:         
		state->bytesPerWord = 1;
		wsize = AL_SAMPLE_8;
		break;
		
	case 16: 
		state->bytesPerWord = 2;
		wsize = AL_SAMPLE_16;
		break;
		
	case 24:
		state->bytesPerWord = 4;
		wsize = AL_SAMPLE_24;
		break;
		
	default:
		fprintf(stderr,"Irix audio: unsupported bit with %d\n", bits);
		break;
	}

	if (alSetWidth(state->alconfig, wsize) < 0) {
		fprintf(stderr, "alSetWidth failed: %s\n", alGetErrorString(oserror()));
		return 0;
	}
	
	params[0].param = AL_RATE;
	params[0].value.ll = alDoubleToFixed((double)rate);
	params[1].param = AL_MASTER_CLOCK;
	params[1].value.i = AL_CRYSTAL_MCLK_TYPE;
	if ( alSetParams(dev, params, 1) < 0) {
		printf("alSetParams() failed: %s\n", alGetErrorString(oserror()));
		return 0;
	}
	
	return state;
}

/*
 * play the sample to the already opened file descriptor
 */

void plugin_play(ao_internal_t *state, void* output_samples, uint_32 num_bytes)
{
	alWriteFrames(((ao_irix_internal_t *)state)->alport, output_samples, num_bytes); 
}

void plugin_close(ao_internal_t *state)
{
	ao_irix_internal_t *s = (ao_irix_internal_t *)state;

	alClosePort(s->alport);
	alFreeConfig(s->alconfig);

	free(state);
}

int plugin_get_latency(ao_internal_t *state)
{
	/* TODO */
	return 0;
}

ao_info_t *plugin_get_driver_info(void)
{
	return &ao_irix_info;
}
