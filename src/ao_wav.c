/*
 *
 *  ao_wav.c
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
#include <errno.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <signal.h>
#include <ao/ao.h>

#define WAVE_FORMAT_PCM  0x0001
#define FORMAT_MULAW     0x0101
#define IBM_FORMAT_ALAW  0x0102
#define IBM_FORMAT_ADPCM 0x0103

#define WAV_HEADER_LEN 44

#define WRITE_U32(buf, x) *(buf)     = (unsigned char)(x&0xff);\
						  *((buf)+1) = (unsigned char)((x>>8)&0xff);\
						  *((buf)+2) = (unsigned char)((x>>16)&0xff);\
						  *((buf)+3) = (unsigned char)((x>>24)&0xff);

#define WRITE_U16(buf, x) *(buf)     = (unsigned char)(x&0xff);\
						  *((buf)+1) = (unsigned char)((x>>8)&0xff);

#define DEFAULT_SWAP_BUFFER_SIZE 2048

struct riff_struct {
	unsigned char id[4];   /* RIFF */
	unsigned int len;
	unsigned char wave_id[4]; /* WAVE */
};


struct chunk_struct 
{
	unsigned char id[4];
	unsigned int len;
};

struct common_struct 
{
	unsigned short wFormatTag;
	unsigned short wChannels;
	unsigned int dwSamplesPerSec;
	unsigned int dwAvgBytesPerSec;
	unsigned short wBlockAlign;
	unsigned short wBitsPerSample;  /* Only for PCM */
};

struct wave_header 
{
	struct riff_struct   riff;
	struct chunk_struct  format;
	struct common_struct common;
	struct chunk_struct  data;
};


static ao_info_t ao_wav_info =
{
	"WAV file output",
	"wav",
	"Aaron Holtzman <aholtzma@ess.engr.uvic.ca>",
	"Sends output to a .wav file"
};

typedef struct ao_wav_internal_s
{
	char *output_file;
	int fd;
	int byte_swap;
	char *swap_buffer;
	int buffer_size;
	struct wave_header wave;
} ao_wav_internal_t;


/* Ack! This is icky.  We have to keep a global linked list of states
   so the signal handler can shut them all down when asked. */
typedef struct ao_wav_state_list_s {
	ao_wav_internal_t *state;
	struct ao_wav_state_list_s *next;
} ao_wav_state_list_t;

static ao_wav_state_list_t *states = NULL;
static ao_wav_state_list_t *last = NULL;


static void (*old_sig)(int);
static void signal_handler(int sig);


static void ao_wav_parse_options(ao_wav_internal_t *state, ao_option_t *options)
{
	state->output_file = NULL;

	while (options) {
		if (!strcmp(options->key, "file"))
			state->output_file = strdup(options->value);
		
		options = options->next;
	}

	if (state->output_file == NULL)
		state->output_file = strdup("output.wav");
}

static ao_internal_t *ao_wav_open(uint_32 bits, uint_32 rate, uint_32 channels, ao_option_t *options)
{
	ao_wav_internal_t *state;
	unsigned char buf[WAV_HEADER_LEN];

	memset(buf, 0, WAV_HEADER_LEN);

	state = malloc(sizeof(ao_wav_internal_t));
	if (state == NULL) {
		fprintf(stderr, "ao_wav: Could not allocate state memory.\n");
		goto ERR;
	}

	// Grab options here
	ao_wav_parse_options(state, options);
	state->byte_swap = (bits == 16) && (ao_is_big_endian());
	if (state->byte_swap) {
		state->buffer_size = DEFAULT_SWAP_BUFFER_SIZE;
		state->swap_buffer = calloc(sizeof(char), state->buffer_size);
	       
		if (state->swap_buffer == NULL) {
			fprintf(stderr, "ao_wav: Could not allocate byte-swapping buffer.\n");
			goto ERR;
		}
	}
		
	state->fd=open(state->output_file,O_WRONLY | O_TRUNC | O_CREAT, 0644);

	if(state->fd < 0) {
		fprintf(stderr,"%s: Opening audio output %s\n", strerror(errno), state->output_file);
		goto ERR;
	}

	/* Write out a ZEROD wave header first */
	memset(&(state->wave), 0, sizeof(state->wave));

	/* Store information */
	state->wave.common.wChannels = channels;
	state->wave.common.wBitsPerSample = bits;
	state->wave.common.dwSamplesPerSec = rate;

	if (write(state->fd, buf, WAV_HEADER_LEN) != WAV_HEADER_LEN) {
		fprintf(stderr,"failed to write wav-header: %s\n", strerror(errno));
		goto ERR;
	}

	if (last == NULL) {
		// Empty list, install our signal handler only once
		old_sig = signal(SIGINT,signal_handler);		

		last = states = malloc(sizeof(ao_wav_state_list_t));
	} else {
		last->next = malloc(sizeof(ao_wav_state_list_t));
		last = last->next;
	}

	last->state = state;
	last->next = NULL;

	return state;

ERR:
	if(state->fd >= 0) { close(state->fd); }
	return NULL;
}


/*
 * play the sample to the already opened file descriptor
 */
static void ao_wav_play(ao_internal_t *state, void *output_samples, uint_32 num_bytes)
{
	int i;
	ao_wav_internal_t *s = (ao_wav_internal_t *)state;

	/* Swap all of the bytes if things are not little_endian */
	if (s->byte_swap) {
		/* Resize buffer larger if needed */
		if (num_bytes > s->buffer_size) {
			s->swap_buffer = realloc(s->swap_buffer, sizeof(char)*num_bytes);
			if (s->swap_buffer == NULL) {
				fprintf(stderr, "ao_wav: Could not resize swap buffer.\n");
				return;
			} else {
				s->buffer_size = num_bytes;
			}
		}

		/* Swap the bytes into the swap buffer (so we don't
		 mess up the output_samples buffer) */
		for(i = 0; i < num_bytes/2; i+=2) {
			s->swap_buffer[i]   = ((char *) output_samples)[i+1];
			s->swap_buffer[i+1] = ((char *) output_samples)[i];
		}

		write(s->fd, s->swap_buffer, num_bytes );
	} else {
		/* Otherwise just write the output buffer directly */
		write(s->fd, output_samples, num_bytes );
	}
}

static void ao_wav_close(ao_internal_t *state)
{
	unsigned char buf[WAV_HEADER_LEN];

	ao_wav_internal_t *s = (ao_wav_internal_t *)state;

	off_t size;

	/* Find how long our file is in total, including header */
	size = lseek(s->fd, 0, SEEK_CUR);

	if (size < 0) {
		fprintf(stderr,"lseek failed - wav-header is corrupt\n");
		goto ERR;
	}

	/* Rewind file */
	if (lseek(s->fd, 0, SEEK_SET) < 0) {
		fprintf(stderr,"rewind failed - wav-header is corrupt\n");
		goto ERR;
	}

	/* Fill out our wav-header with some information. */

	strncpy(s->wave.riff.id, "RIFF",4);
	s->wave.riff.len = size - 8;
	strncpy(s->wave.riff.wave_id, "WAVE",4);

	strncpy(s->wave.format.id, "fmt ",4);
	s->wave.format.len = 16;

	s->wave.common.wFormatTag = WAVE_FORMAT_PCM;
	s->wave.common.dwAvgBytesPerSec = 
		s->wave.common.wChannels * s->wave.common.dwSamplesPerSec *
		(s->wave.common.wBitsPerSample >> 3);

	s->wave.common.wBlockAlign = s->wave.common.wChannels * 
		(s->wave.common.wBitsPerSample >> 3);

	strncpy(s->wave.data.id, "data",4);

	s->wave.data.len = size - 44;

	strncpy(buf, s->wave.riff.id, 4);
	WRITE_U32(buf+4, s->wave.riff.len);
	strncpy(buf+8, s->wave.riff.wave_id, 4);
	strncpy(buf+12, s->wave.format.id,4);
	WRITE_U32(buf+16, s->wave.format.len);
	WRITE_U16(buf+20, s->wave.common.wFormatTag);
	WRITE_U16(buf+22, s->wave.common.wChannels);
	WRITE_U32(buf+24, s->wave.common.dwSamplesPerSec);
	WRITE_U32(buf+28, s->wave.common.dwAvgBytesPerSec);
	WRITE_U16(buf+32, s->wave.common.wBlockAlign);
	WRITE_U16(buf+34, s->wave.common.wBitsPerSample);
	strncpy(buf+36, s->wave.data.id, 4);
	WRITE_U32(buf+40, s->wave.data.len);

	if (write(s->fd, buf, WAV_HEADER_LEN) < WAV_HEADER_LEN) {
		fprintf(stderr,"wav-header write failed -- file is corrupt\n");
		goto ERR;
	}

ERR:
	close(s->fd);
	free(s);
}

static ao_info_t *ao_wav_get_driver_info(void)
{
	return &ao_wav_info;
}


static void signal_handler(int sig)
{
	ao_wav_state_list_t *temp = states;
	
	while (states) {
		ao_wav_close(states->state);
		temp = states;
		states = states->next;
		free(temp);
	}

	states = NULL;

	signal(sig, old_sig);
	raise(sig);
}

ao_functions_t ao_wav =
{
        ao_wav_get_driver_info,
        ao_wav_open,
        ao_wav_play,
        ao_wav_close
};
