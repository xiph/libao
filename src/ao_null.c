/*
 *
 *  ao_null.c
 *
 *      Original Copyright (C) Aaron Holtzman - May 1999
 *      Modifications Copyright (C) Stan Seibert - July 2000
 *
 *  This file is part of libao, a cross-platform audio output library.  See
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
#include <ao/ao.h>

typedef struct ao_null_internal_s {
	unsigned long byte_counter;
} ao_null_internal_t;

static ao_info_t ao_null_info = {
	"Null output",
	"null",
	"Aaron Holtzman <aholtzma@ess.engr.uvic.ca>",
	"This plugin does nothing"
};

static ao_internal_t *ao_null_open(uint_32 bits, uint_32 rate, uint_32 channels, ao_option_t *options)
{
	ao_null_internal_t *state;

	state = malloc(sizeof(ao_null_internal_t));

	if (state != NULL) {
		state->byte_counter = 0;
		return state;
	} else {
		return NULL;
	}

	return NULL;
}

static void ao_null_close(ao_internal_t *state)
{
        /* why would we print in a lib :)
	fprintf(stderr, "ao_null: %ld bytes sent to null device.\n",
		((ao_null_internal_t *) state)->byte_counter);
	*/
	if (state) free(state);
}

static void ao_null_play(ao_internal_t *state, void* output_samples, uint_32 num_bytes)
{
	((ao_null_internal_t *)state)->byte_counter += num_bytes;
}

static ao_info_t *ao_null_get_driver_info(void)
{
	return &ao_null_info;
}

static int ao_null_get_latency(ao_internal_t *state)
{
	return 0;
}

ao_functions_t ao_null = {
	ao_null_get_driver_info,
	ao_null_open,
	ao_null_play,
	ao_null_close,
	ao_null_get_latency
};
