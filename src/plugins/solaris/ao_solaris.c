/*
 *
 *  ao_null.c
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
#include <sys/audioio.h>
#include <sys/ioctl.h>
#include <stropts.h>
#include <signal.h>
#include <math.h>

//FIXME broken solaris headers!
int usleep(unsigned int useconds);

#include <ao/ao.h>


ao_info_t ao_solaris_info =
{
	"Solaris audio output",
	"solaris",
	"Aaron Holtzman <aholtzma@ess.engr.uvic.ca>",
	"WARNING: This driver is untested."
};

typedef struct ao_solaris_internal_s  {
 /* Global to keep track of old state */
	static audio_info_t info;
	static char *dev;
	static int fd;
} ao_solaris_internal_t;

/*
 * open the audio device for writing to
 */
ao_internal_t *plugin_open(uint_32 bits, uint_32 rate, uint_32 channels, ao_option_t *options)
{
	ao_solaris_internal_t *state;

	state = malloc(sizeof(ao_solaris_internal_t));
	if (state == NULL)
		return NULL;

	state->dev = strdup("/dev/audio");

	/*
	 * Open the device driver
	 */

	state->fd=open(state->dev,O_WRONLY);
	if(state->fd < 0) {
		fprintf(stderr,"%s: Opening audio device %s\n",
				strerror(errno), state->dev);
		goto ERR;
	}

	/* Setup our parameters */
	AUDIO_INITINFO(&(state->info));

	state->info.play.sample_rate = rate;
	state->info.play.precision = bits;
	state->info.play.channels = channels;
	state->info.play.buffer_size = 1024;
	state->info.play.encoding = AUDIO_ENCODING_LINEAR;
	//state->info.play.port = AUDIO_SPEAKER;
	//state->info.play.gain = 110;

	/* Write our configuration */
	/* An implicit GETINFO is also performed so we can get
	 * the buffer_size */

	if(ioctl(state->fd, AUDIO_SETINFO, &(state->info)) < 0) {
		fprintf(stderr, "%s: Writing audio config block\n",strerror(errno));
		goto ERR;
	}

	return state;

ERR:
	if(fd >= 0) { close(fd); }
	return NULL;
}

/*
 * play the sample to the already opened file descriptor
 */
void plugin_play(ao_internal_t *state, void *output_samples, uint_32 num_bytes)
{
	write(((ao_solaris_internal_t *)state)->fd, output_samples, num_bytes);
}


void plugin_close(ao_internal_t *state)
{
	close(((ao_solaris_internal_t *) state)->fd);
	free(state);
}

const ao_info_t *plugin_get_driver_info(void)
{
	return &ao_solaris_info;
}
