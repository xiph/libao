/*
 *
 *  ao_esd.c
 *
 *      Copyright (C) Stan Seibert - July 2000
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

typedef struct ao_esd_internal_s
{
	int sock;
	char *host;
} ao_esd_internal_t;

ao_info_t ao_esd_info =
{
	"ESounD output",
	"esd",
	"Stan Seibert <volsung@asu.edu>",
	"Outputs to the Enlighted Sound Daemon."
};

void ao_esd_parse_options(ao_esd_internal_t *state, ao_option_t *options)
{
	state->host = NULL;

	while (options) {
		if (!strcmp(options->key, "host"))
			state->host = strdup(options->value);
		
		options = options->next;
	}
}

ao_internal_t *plugin_open(uint_32 bits, uint_32 rate, uint_32 channels, ao_option_t *options)
{
	ao_esd_internal_t *state;
	int esd_bits;
	int esd_channels;
	int esd_mode = ESD_STREAM;
	int esd_func = ESD_PLAY;
	int esd_format;

	switch (bits)
	{
	case 8  : esd_bits = ESD_BITS8;
		  break;
	case 16 : esd_bits = ESD_BITS16;
		  break;
	default : return NULL;
	}

	switch (channels)
	{
	case 1 : esd_channels = ESD_MONO;
		 break;
	case 2 : esd_channels = ESD_STEREO;
		 break;
	default: return NULL;
	}
	
	esd_format = esd_bits | esd_channels | esd_mode | esd_func;

	state = malloc(sizeof(ao_esd_internal_t));

	if (state == NULL)
		return NULL;

	ao_esd_parse_options(state, options);

	state->sock = esd_play_stream(esd_format, rate, state->host,
				      "libao output");
	if ( state->sock <= 0 ) {
		free(state->host);
		free(state);
		return NULL;
	}

	return state;
}

void plugin_close(ao_internal_t *state)
{
	ao_esd_internal_t *s = (ao_esd_internal_t *)state;
	close(s->sock);
	free(s->host);
	free(s);
}

void plugin_play(ao_internal_t *state, void* output_samples, uint_32 num_bytes)
{
	write(((ao_esd_internal_t *) state)->sock, output_samples, num_bytes);
}

int plugin_get_latency(ao_internal_t *state)
{
	ao_esd_internal_t *s = (ao_esd_internal_t *)state;
	return (esd_get_latency(s->sock));
}

ao_info_t *plugin_get_driver_info(void)
{
	return &ao_esd_info;
}
