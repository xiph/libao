/*
 *
 *  ao_raw.c
 *
 *      Copyright (C) Stan Seibert - January 2001
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
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <errno.h>
#include <unistd.h>
#include <ao/ao.h>

/* Byte ordering constants */
#define BYTEORDER_NATIVE        1
#define BYTEORDER_BIG_ENDIAN    2
#define BYTEORDER_LITTLE_ENDIAN 3

#define DEFAULT_SWAP_BUFFER_SIZE 2048


static ao_info_t ao_raw_info =
{
	"RAW sample output",
	"raw",
	"Stan Seibert <indigo@aztec.asu.edu>",
	"Writes raw audio samples to a file"
};

typedef struct ao_raw_internal_s
{
	char *output_file;
	int fd;
	int byteorder;
	int byte_swap;
	char *swap_buffer;
	int buffer_size;
} ao_raw_internal_t;


static void ao_raw_parse_options(ao_raw_internal_t *state, ao_option_t *options)
{
	state->output_file = NULL;

	while (options) {
		if (!strcmp(options->key, "file"))
			state->output_file = strdup(options->value);
		else if (!strcmp(options->key, "byteorder")) {
			
			if (!strcmp(options->value, "native"))
				state->byteorder = BYTEORDER_NATIVE;
			else if (!strcmp(options->value, "big"))
				state->byteorder = BYTEORDER_BIG_ENDIAN;
			else if (!strcmp(options->value, "little"))
				state->byteorder = BYTEORDER_LITTLE_ENDIAN;
		}
		options = options->next;
	}

	if (state->output_file == NULL)
		state->output_file = strdup("output.raw");
	if (state->byteorder == 0)
		state->byteorder = BYTEORDER_NATIVE;
}

static ao_internal_t *ao_raw_open(uint_32 bits, uint_32 rate, uint_32 channels, ao_option_t *options)
{
	ao_raw_internal_t *state;

	state = malloc(sizeof(ao_raw_internal_t));
	if (state == NULL) {
		fprintf(stderr, "ao_raw: Could not allocate state memory.\n");
		goto ERR;
	}
	state->byteorder = 0;
	state->output_file = NULL;

	/* Grab options here */
	ao_raw_parse_options(state, options);

	/* Figure out byte swapping */
	if (bits == 16) {
		switch (state->byteorder) {
		case BYTEORDER_NATIVE :
			state->byte_swap = 0;
			break;
		case BYTEORDER_BIG_ENDIAN :
			state->byte_swap = !ao_is_big_endian();
			break;
		case BYTEORDER_LITTLE_ENDIAN :
			state->byte_swap = ao_is_big_endian();
			break;
		default :
			fprintf(stderr, "ao_raw: Internal error - incorrect byte order constant %d\n", state->byteorder);
			goto ERR;
		}
	} else 
		state->byte_swap = 0;
		
	if (state->byte_swap) {
		state->buffer_size = DEFAULT_SWAP_BUFFER_SIZE;
		state->swap_buffer = calloc(sizeof(char), state->buffer_size);
	       
		if (state->swap_buffer == NULL) {
			fprintf(stderr, "ao_raw: Could not allocate byte-swapping buffer.\n");
			goto ERR;
		}
	}


	/* Setup output file */
	if (strncmp(state->output_file, "-", 2) == 0)
	  state->fd = STDOUT_FILENO;
	else
	  state->fd = open(state->output_file,
			   O_WRONLY | O_TRUNC | O_CREAT, 0644);

	if(state->fd < 0) {
		fprintf(stderr,"%s: Opening audio output %s\n", strerror(errno), state->output_file);
		goto ERR;
	}

	return state;

ERR:
	if(state->fd >= 0) { close(state->fd); }
	return NULL;
}


/*
 * play the sample to the already opened file descriptor
 */
static void ao_raw_play(ao_internal_t *state, void *output_samples, uint_32 num_bytes)
{
	int i;
	ao_raw_internal_t *s = (ao_raw_internal_t *)state;

	/* Swap all of the bytes if things are not little_endian */
	if (s->byte_swap) {
		/* Resize buffer larger if needed */
		if (num_bytes > s->buffer_size) {
			s->swap_buffer = realloc(s->swap_buffer, sizeof(char)*num_bytes);
			if (s->swap_buffer == NULL) {
				fprintf(stderr, "ao_raw: Could not resize swap buffer.\n");
				return;
			} else {
				s->buffer_size = num_bytes;
			}
		}

		/* Swap the bytes into the swap buffer (so we don't
		 mess up the output_samples buffer) */
		for(i = 0; i < num_bytes; i+=2) {
			s->swap_buffer[i]   = ((char *) output_samples)[i+1];
			s->swap_buffer[i+1] = ((char *) output_samples)[i];
		}

		write(s->fd, s->swap_buffer, num_bytes );
	} else {
		/* Otherwise just write the output buffer directly */
		write(s->fd, output_samples, num_bytes );
	}
}

static void ao_raw_close(ao_internal_t *state)
{
	ao_raw_internal_t *s = (ao_raw_internal_t *)state;

	close(s->fd);
	if (s->byte_swap)
		free(s->swap_buffer);
	free(s);
}

static int ao_raw_get_latency(ao_internal_t *state)
{
	return 0;
}

static ao_info_t *ao_raw_get_driver_info(void)
{
	return &ao_raw_info;
}

ao_functions_t ao_raw =
{
        ao_raw_get_driver_info,
        ao_raw_open,
        ao_raw_play,
        ao_raw_close,
	ao_raw_get_latency
};
