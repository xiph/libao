/*
 *
 *  ao_oss.c
 *
 *      Original Copyright (C) Aaron Holtzman - May 1999
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
#if defined(__OpenBSD__) || defined(__NetBSD__)
#include <soundcard.h>
#elif defined(__FreeBSD__)
#include <machine/soundcard.h>
#else
#include <sys/soundcard.h>
#endif
#include <sys/ioctl.h>

#include <ao/ao.h>

ao_info_t ao_oss_info =
{
	"OSS audio driver output ",
	"oss",
	"Aaron Holtzman <aholtzma@ess.engr.uvic.ca>",
	"Outputs audio to the Open Sound System driver."
};


typedef struct ao_oss_internal_s {
	char *dev;
	int fd;
} ao_oss_internal_t;

static int _is_big_endian(void)
{
	uint_16 pattern = 0xbabe;
	unsigned char *bytewise = (unsigned char *)&pattern;

	if (bytewise[0] == 0xba) return 1;
	return 0;
}

void ao_oss_parse_options(ao_oss_internal_t *state, ao_option_t *options)
{
	state->dev = NULL;

	while (options) {
		if (!strcmp(options->key, "dsp"))
			state->dev = strdup(options->value);
		
		options = options->next;
	}

	/* otherwise, the NULL setting indicates the open()
	   routine should choose something from hardwired defaults */
}

/*
 * open the audio device for writing to
 */
ao_internal_t *plugin_open(uint_32 bits, uint_32 rate, uint_32 channels, ao_option_t *options)
{
	ao_oss_internal_t *state;
	int tmp;
	
	/* Allocate a state structure to hold instance
	   information.  (Long live C++!) */
	state = malloc(sizeof(ao_oss_internal_t));

	if (state == NULL)	
	{
		fprintf(stderr,"libao - %s: Allocating state memory.\n",
			strerror(errno));
		goto ERR;
	}

	ao_oss_parse_options(state, options);

	/* Open the device driver */
	
	if (state->dev != NULL) {
		/* open the user-specified path */
		state->fd=open(state->dev,O_WRONLY);
		if(state->fd < 0) 
		{
			fprintf(stderr,"libao - %s: Opening audio device %s\n",
				strerror(errno), state->dev);
			goto ERR;
		}
	} else {
		/* default: first try the devfs path */
		state->dev = strdup("/dev/sound/dsp");
		state->fd=open(state->dev,O_WRONLY);
		if(state->fd < 0) 
		{
			/* no? then try the traditional path */
			char *err = strdup(strerror(errno));
			char *dev = strdup(state->dev);
			free(state->dev);
			state->dev = strdup("/dev/dsp");
			state->fd=open(state->dev,O_WRONLY);
			if(state->fd < 0) 
			{
				fprintf(stderr,
					"libao - error: Could not open either default device:\n"
					"  %s - %s\n"
					"  %s - %s\n",
					err, dev,
					strerror(errno), state->dev);
				free(err);
				free(dev);
				goto ERR;
			}
		}
	}

	switch (channels)
	{
	case 1: tmp = 0;
		break;
	case 2: tmp = 1;
		break;
	default:fprintf(stderr,"libao - Unsupported number of channels: %d.",
			channels);
		goto ERR;
	}
	ioctl(state->fd,SNDCTL_DSP_STEREO,&tmp);
	
	switch (bits)
	{
	case 8: tmp = AFMT_S8;
		break;
        case 16: tmp = _is_big_endian() ? AFMT_S16_BE : AFMT_S16_LE;
	        break;
	default:fprintf(stderr,"libao - Unsupported number of bits: %d.",
			bits);
		goto ERR;
	}
	ioctl(state->fd,SNDCTL_DSP_SAMPLESIZE,&tmp);
	
	tmp = rate;
	ioctl(state->fd,SNDCTL_DSP_SPEED, &tmp);
	
	return state;

 ERR:
	if(state != NULL)
	{ 
		if (state->fd >= 0) { close(state->fd); }
		if (state->dev) { free(state->dev); }
		free(state);
	}

	return NULL;
}

/*
 * play the sample to the already opened file descriptor
 */
void plugin_play(ao_internal_t *state, void *output_samples, uint_32 num_bytes)
{
	write( ((ao_oss_internal_t *)state)->fd, output_samples, num_bytes);
}


void plugin_close(ao_internal_t *state)
{
	ao_oss_internal_t *s = (ao_oss_internal_t *) state;
	close(s->fd);
	free(s->dev);
	free(s);
}

int plugin_get_latency(ao_internal_t *state)
{
	int odelay = 0;
	ioctl(((ao_oss_internal_t *)state)->fd, SNDCTL_DSP_GETODELAY, &odelay);
	return odelay;
}

ao_info_t *plugin_get_driver_info(void)
{
	return &ao_oss_info;
}
