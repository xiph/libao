/*
 *
 *  ao_alsa.c
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

#include <sys/asoundlib.h>
#include <ao/ao.h>

#define AO_ALSA_BUF_SIZE 32768

typedef struct ao_alsa_internal_s
{
	snd_pcm_t *pcm_handle;
	char *buf;
	int buf_size;
	int buf_end;
	int card;
	int dev;
} ao_alsa_internal_t;

ao_info_t ao_alsa_info =
{
	"Advanced Linux Sound Architecture (ALSA) output",
	"alsa",
	"Stan Seibert <volsung@asu.edu>",
	"Otuputs to the Advanced Linux Sound Architecture."
};

static int _is_big_endian(void)
{
        uint_16 pattern = 0xbabe;
        unsigned char *bytewise = (unsigned char *)&pattern;

        if (bytewise[0] == 0xba) return 1;
        return 0;
}

void ao_alsa_parse_options(ao_alsa_internal_t *state, ao_option_t *options)
{
	state->card = 0;
	state->dev = 0;
	state->buf_size = AO_ALSA_BUF_SIZE;

	while (options) {
		if (!strcmp(options->key, "card"))
			state->card = atoi(options->value);
		else if (!strcmp(options->key, "dev"))
			state->dev = atoi(options->value);
		else if (!strcmp(options->key, "buf_size"))
			state->buf_size = atoi(options->value);
		
		options = options->next;
	}
}

ao_internal_t *plugin_open(uint_32 bits, uint_32 rate, uint_32 channels, ao_option_t *options)
{
	ao_alsa_internal_t *state;
	snd_pcm_channel_params_t param;
	int err;

	memset(&param, 0, sizeof(param));

	param.channel = SND_PCM_CHANNEL_PLAYBACK;
	param.mode = SND_PCM_MODE_BLOCK;

	param.format.interleave = 1;

	switch (bits) {
	case 8  : param.format.format = SND_PCM_SFMT_S8;
		  break;
        case 16 : param.format.format = _is_big_endian() ?
		    SND_PCM_SFMT_S16_BE : SND_PCM_SFMT_S16_LE;
		  break;
	default : return NULL;
	}

	if (channels > 0 && channels < 3)
		param.format.voices = channels;
	else
		return NULL;

	// Allocate the state structure and parse the options
	state = malloc(sizeof(ao_alsa_internal_t));

	if (state == NULL)
		return NULL;

	ao_alsa_parse_options(state, options);

	// Finish filling in the parameter structure
	param.format.rate = rate;

	param.start_mode = SND_PCM_START_FULL;
	
	param.stop_mode = SND_PCM_STOP_STOP;

	param.buf.block.frag_size = state->buf_size;
	param.buf.block.frags_min = 1;
	param.buf.block.frags_max = 8;


	err = snd_pcm_open(&(state->pcm_handle), 
			   state->card, 
			   state->dev,
			   SND_PCM_OPEN_PLAYBACK | SND_PCM_OPEN_NONBLOCK);
	if (err < 0) {
		free(state);
		return NULL;
	}

	err = snd_pcm_channel_params(state->pcm_handle, &param);

	if (err < 0) {
		snd_pcm_close(state->pcm_handle);
		free(state);
		return NULL;
	}

	state->buf = malloc(state->buf_size);
	state->buf_end = 0;

	snd_pcm_nonblock_mode(state->pcm_handle, 0);
	snd_pcm_channel_prepare(state->pcm_handle, SND_PCM_CHANNEL_PLAYBACK);

	return state;
}

void plugin_close(ao_internal_t *state)
{
	ao_alsa_internal_t *s = (ao_alsa_internal_t *) state;
	snd_pcm_close(s->pcm_handle);
	free(s);
}

void ao_alsa_write_buffer(ao_alsa_internal_t *s)
{
	snd_pcm_channel_status_t status;
	snd_pcm_t *pcm_handle = s->pcm_handle;
	int len = s->buf_end;

	s->buf_end = 0;
	snd_pcm_write(pcm_handle, s->buf, len);
	memset(&status, 0, sizeof(status));
	if (snd_pcm_channel_status(pcm_handle, &status) < 0) {
		fprintf(stderr, "ALSA: could not get channel status\n");
		return;
	}       
	if (status.underrun) {
		fprintf(stderr, "ALSA: underrun. resetting channel\n");
		snd_pcm_channel_flush(pcm_handle, SND_PCM_CHANNEL_PLAYBACK);
		snd_pcm_playback_prepare(pcm_handle);
		snd_pcm_write(pcm_handle, s->buf, len);
		if (snd_pcm_channel_status(pcm_handle, &status) < 0) {
			fprintf(stderr, "ALSA: could not get channel status. giving up\n");
			return;
		}
		if (status.underrun) {
			fprintf(stderr, "ALSA: write error. giving up\n");
					return;
		}               
	}
}	

void plugin_play(ao_internal_t *state, void* output_samples, uint_32 num_bytes)
{
	ao_alsa_internal_t *s = (ao_alsa_internal_t *) state;
	int packed = 0;
	int copy_len;
	char *samples = (char *) output_samples;

	while (packed < num_bytes) {
		/* Pack the buffer */
		if (num_bytes-packed < s->buf_size-s->buf_end)
			copy_len = num_bytes - packed;
		else
			copy_len = s->buf_size-s->buf_end;

		memcpy(s->buf + s->buf_end, samples + packed, copy_len); 
		packed += copy_len;
		s->buf_end += copy_len;

		if(s->buf_end == s->buf_size)
			ao_alsa_write_buffer(s);
	}
}

ao_info_t *plugin_get_driver_info(void)
{
	return &ao_alsa_info;
}
