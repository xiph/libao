/*
 *
 *  ao_au.c
 *
 *      Copyright (C) Wil Mahan - May 2001
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

#define AUDIO_FILE_MAGIC ((uint_32)0x2e736e64)  /* ".snd" */

#define AUDIO_UNKNOWN_SIZE (~0) /* (unsigned) -1 */

/* Format codes (not comprehensive) */
#define AUDIO_FILE_ENCODING_LINEAR_8 (2) /* 8-bit linear PCM */
#define AUDIO_FILE_ENCODING_LINEAR_16 (3) /* 16-bit linear PCM */

#define AU_HEADER_LEN (28)

#define DEFAULT_SWAP_BUFFER_SIZE 2048

/* Write a uint_32 in big-endian order. */
#define WRITE_U32(buf, x) \
	*(buf) = (unsigned char)(((x)>>24)&0xff);\
	*((buf)+1) = (unsigned char)(((x)>>16)&0xff);\
	*((buf)+2) = (unsigned char)(((x)>>8)&0xff);\
	*((buf)+3) = (unsigned char)((x)&0xff);

typedef struct {
	uint_32 magic; /* magic number */
	uint_32 hdr_size; /* offset of the data */
	uint_32 data_size; /* length of data (optional) */
	uint_32 encoding; /* data format code */
	uint_32 sample_rate; /* samples per second */
	uint_32 channels; /* number of interleaved channels */
	char info[4]; /* optional text information */
} Audio_filehdr;

static ao_info_t ao_au_info =
{
	"AU file output",
	"au",
	"Wil Mahan <wtm2@duke.edu>",
	"Sends output to a .au file"
};

typedef struct ao_au_internal_s
{
	char *output_file;
	int fd;
	int byte_swap;
	char *swap_buffer;
	int buffer_size;
	Audio_filehdr au; 		
} ao_au_internal_t;


static void ao_au_parse_options(ao_au_internal_t *state, ao_option_t *options)
{
	state->output_file = NULL;

	while (options) {
		if (!strcmp(options->key, "file"))
			state->output_file = strdup(options->value);

		options = options->next;
	}

	if (state->output_file == NULL)
		state->output_file = strdup("output.au");
}

static ao_internal_t *ao_au_open(uint_32 bits, uint_32 rate, uint_32 channels, ao_option_t *options)
{
	ao_au_internal_t *state;
	unsigned char buf[AU_HEADER_LEN];
	int i;

	state = malloc(sizeof(ao_au_internal_t));
	if (state == NULL) {
		fprintf(stderr, "ao_au: Could not allocate state memory.\n");
		goto ERR;
	}
	state->output_file = NULL;

	/* Grab options here */
	ao_au_parse_options(state, options);

	/* The AU format is big-endian */
	state->byte_swap = (bits == 16) && (!ao_is_big_endian());
		
	if (state->byte_swap) {
		state->buffer_size = DEFAULT_SWAP_BUFFER_SIZE;
		state->swap_buffer = calloc(sizeof(char), state->buffer_size);
	       
		if (state->swap_buffer == NULL) {
			fprintf(stderr, "ao_au: Could not allocate byte-swapping buffer.\n");
			goto ERR;
		}
	}

	/* Set up output file */
	if (strncmp(state->output_file, "-", 2) == 0)
		state->fd = STDOUT_FILENO;
	else
		state->fd = open(state->output_file,
			O_WRONLY | O_TRUNC | O_CREAT, 0644);

	if(state->fd < 0) {
		fprintf(stderr,"%s: Opening audio output %s\n", strerror(errno), state->output_file);
		goto ERR;
	}

	/* Write a zeroed au header first */
	memset(&(state->au), 0, sizeof(state->au));

	/* Fill out the header */
	state->au.magic = AUDIO_FILE_MAGIC;
	state->au.channels = channels;
	if (bits == 8)
		state->au.encoding = AUDIO_FILE_ENCODING_LINEAR_8;
	else if (bits == 16)
		state->au.encoding = AUDIO_FILE_ENCODING_LINEAR_16;
	else {
		/* Only 8 and 16 bits are supported at present. */
		fprintf(stderr,"ao_au: unsupported number of bits");
	}	
	state->au.sample_rate = rate;
	state->au.hdr_size = AU_HEADER_LEN;

	/* From the AU specification:  "When audio files are passed
	 * through pipes, the 'data_size' field may not be known in
	 * advance.  In such cases, the 'data_size' should be set
	 * to AUDIO_UNKNOWN_SIZE."
	 */
	state->au.data_size = AUDIO_UNKNOWN_SIZE;
	/* strncpy(state->au.info, "OGG ", 4); */

	/* Write the header in big-endian order */
	WRITE_U32(buf, state->au.magic);
	WRITE_U32(buf + 4, state->au.hdr_size);
	WRITE_U32(buf + 8, state->au.data_size);
	WRITE_U32(buf + 12, state->au.encoding);
	WRITE_U32(buf + 16, state->au.sample_rate);
	WRITE_U32(buf + 20, state->au.channels);
	strncpy (buf + 24, state->au.info, 4);

	if (write(state->fd, buf, AU_HEADER_LEN) != AU_HEADER_LEN) {
		fprintf(stderr,"ao_au: writing header: %s\n", strerror(errno));
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
static void ao_au_play(ao_internal_t *state, void *output_samples, uint_32 num_bytes)
{
	int i;
	ao_au_internal_t *s = (ao_au_internal_t *)state;

	/* Swap all of the bytes if things are not big-endian */
	if (s->byte_swap) {
		/* Resize buffer larger if needed */
		if (num_bytes > s->buffer_size) {
			s->swap_buffer = realloc(s->swap_buffer, sizeof(char)*num_bytes);
			if (s->swap_buffer == NULL) {
				fprintf(stderr, "ao_au: Could not resize swap buffer.\n");
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

		write(s->fd, s->swap_buffer, num_bytes);
	} else {
		/* Otherwise just write the output buffer directly */
		write(s->fd, output_samples, num_bytes);
	}
}

static void ao_au_close(ao_internal_t *state)
{
	ao_au_internal_t *s = (ao_au_internal_t *)state; 
        off_t size;
	unsigned char buf[4];

	/* Try to find the total file length, including header */
	size = lseek(s->fd, 0, SEEK_CUR);

	/* It's not a problem if the lseek() fails; the AU
	 * format does not require a file length.  This is
	 * useful for writing to non-seekable files (e.g.
	 * pipes).
	 */
	if (size > 0) {
		s->au.data_size = size - AU_HEADER_LEN;

		/* Rewind the file */
		if (lseek(s->fd, 8 /* offset of data_size */,
					SEEK_SET) < 0)
		{
			fprintf(stderr,"ao_au: rewind failed\n");
			goto ERR;
		}

		/* Fill in the file length */
		WRITE_U32 (buf, s->au.data_size);
		if (write(s->fd, buf, 4) < 4) {
			fprintf(stderr,"ao_au: header write failed\n");
			goto ERR;
		}
	}

ERR:
	close(s->fd);
	free(s->output_file);
	if (s->byte_swap)
		free(s->swap_buffer);
	free(s);
}

static int ao_au_get_latency(ao_internal_t *state)
{
	return 0;
}

static ao_info_t *ao_au_get_driver_info(void)
{
	return &ao_au_info;
}

ao_functions_t ao_au =
{
        ao_au_get_driver_info,
        ao_au_open,
        ao_au_play,
        ao_au_close,
	ao_au_get_latency
};

