/*
 *
 *  ao_alsa.c
 *
 *      Copyright (C) Stan Seibert - July 2000, July 2001
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
#include <ao/plugin.h>

#define AO_ALSA_BUF_SIZE 32768


static char *ao_alsa_options[] = {"card","dev","buf_size"};
static ao_info ao_alsa_info =
{
	AO_TYPE_LIVE,
	"Advanced Linux Sound Architecture (ALSA) output",
	"alsa05",
	"Stan Seibert <volsung@asu.edu>",
	"Outputs to the Advanced Linux Sound Architecture version 0.5.x.",
	AO_FMT_NATIVE,
	34,
	ao_alsa_options,
	3
};

typedef struct ao_alsa_internal
{
	snd_pcm_t *pcm_handle;
	char *buf;
	int buf_size;
	int buf_end;
	int card;
	int dev;
} ao_alsa_internal;


int ao_plugin_test()
{
	snd_pcm_t *handle;

	if (snd_pcm_open(&handle, 0, 0, 
			 SND_PCM_OPEN_PLAYBACK | SND_PCM_OPEN_NONBLOCK) != 0)
		return 0; /* Cannot use this plugin with default parameters */
	else {
		snd_pcm_close(handle);
		return 1; /* This plugin works in default mode */
	}
}


ao_info *ao_plugin_driver_info(void)
{
	return &ao_alsa_info;
}


int ao_plugin_device_init(ao_device *device)
{
	ao_alsa_internal *internal;

	internal = (ao_alsa_internal *) malloc(sizeof(ao_alsa_internal));

	if (internal == NULL)	
		return 0; /* Could not initialize device memory */
	
	internal->buf_size = AO_ALSA_BUF_SIZE;
	internal->card = 0;
	internal->dev = 0;
	
	device->internal = internal;

	return 1; /* Memory alloc successful */
}


int ao_plugin_set_option(ao_device *device, const char *key, const char *value)
{
	ao_alsa_internal *internal = (ao_alsa_internal *) device->internal;
	if (!strcmp(key, "card"))
		internal->card = atoi(value);
	else if (!strcmp(key, "dev"))
		internal->dev = atoi(value);
	else if (!strcmp(key, "buf_size"))
		internal->buf_size = atoi(value);

	return 1;
}

int ao_plugin_open(ao_device *device, ao_sample_format *format)
{
	ao_alsa_internal *internal = (ao_alsa_internal *) device->internal;

	snd_pcm_channel_params_t param;
	int err;

	memset(&param, 0, sizeof(param));

	param.channel = SND_PCM_CHANNEL_PLAYBACK;
	param.mode = SND_PCM_MODE_BLOCK;

	param.format.interleave = 1;

	switch (format->bits) {
	case 8  : param.format.format = SND_PCM_SFMT_S8;
		  break;
        case 16 : param.format.format = 
		    device->client_byte_format == AO_FMT_BIG ?
		    SND_PCM_SFMT_S16_BE : SND_PCM_SFMT_S16_LE;
	          device->driver_byte_format = device->client_byte_format;
		  break;
	default : return 0;
	}

	if (format->channels == 1 || format->channels == 2)
		param.format.voices = format->channels;
	else
		return 0;

	/* Finish filling in the parameter structure */
	param.format.rate = format->rate;

	param.start_mode = SND_PCM_START_FULL;
	
	param.stop_mode = SND_PCM_STOP_STOP;

	param.buf.block.frag_size = internal->buf_size;
	param.buf.block.frags_min = 1;
	param.buf.block.frags_max = 8;

	internal->buf = malloc(internal->buf_size);
	internal->buf_end = 0;
	if (internal->buf == NULL)
	  return 0;  /* Could not alloc swap buffer */


	/* Open the ALSA device */
	err = snd_pcm_open(&(internal->pcm_handle), 
			   internal->card, 
			   internal->dev,
			   SND_PCM_OPEN_PLAYBACK | SND_PCM_OPEN_NONBLOCK);
	if (err < 0) {
		free(internal->buf);
		return 0;
	}

	err = snd_pcm_channel_params(internal->pcm_handle, &param);

	if (err < 0) {
		snd_pcm_close(internal->pcm_handle);
		free(internal->buf);
		return 0;
	}

	snd_pcm_nonblock_mode(internal->pcm_handle, 0);
	snd_pcm_channel_prepare(internal->pcm_handle, 
				SND_PCM_CHANNEL_PLAYBACK);

	return 1;
}


int _alsa_write_buffer(ao_alsa_internal *s)
{
	snd_pcm_channel_status_t status;
	snd_pcm_t *pcm_handle = s->pcm_handle;
	int len = s->buf_end;
	ssize_t written, snd_pcm_write_ret;

	s->buf_end = 0;

	snd_pcm_write_ret = written = 0;
	while ((snd_pcm_write_ret >= 0) && (written < len)) {
		while ((snd_pcm_write_ret = snd_pcm_write(pcm_handle, s->buf, len)) == -EINTR)
			;
		if (snd_pcm_write_ret > 0)
			written += snd_pcm_write_ret;
	}

	memset(&status, 0, sizeof(status));
	if (snd_pcm_channel_status(pcm_handle, &status) < 0) {
		fprintf(stderr, "ALSA: could not get channel status\n");
		return 0;
	}       
	if (status.underrun) {
		/* fprintf(stderr, "ALSA: underrun. resetting channel\n"); */
		snd_pcm_channel_flush(pcm_handle, SND_PCM_CHANNEL_PLAYBACK);
		snd_pcm_playback_prepare(pcm_handle);
		snd_pcm_write(pcm_handle, s->buf, len);
		if (snd_pcm_channel_status(pcm_handle, &status) < 0) {
			fprintf(stderr, "ALSA: could not get channel status. giving up\n");
			return 0;
		}
		if (status.underrun) {
			fprintf(stderr, "ALSA: write error. giving up\n");
					return 0;
		}               
	}

	return 1;
}	


int ao_plugin_play(ao_device *device, const char *output_samples, 
		uint_32 num_bytes)
{
	ao_alsa_internal *internal = (ao_alsa_internal *) device->internal;
	
	int packed = 0;
	int copy_len;
	char *samples = (char *) output_samples;
	int ok = 1;

	while (packed < num_bytes && ok) {
		/* Pack the buffer */
		if (num_bytes-packed < internal->buf_size - internal->buf_end)
			copy_len = num_bytes - packed;
		else
			copy_len = internal->buf_size - internal->buf_end;

		memcpy(internal->buf + internal->buf_end, samples + packed, 
		       copy_len); 
		packed += copy_len;
		internal->buf_end += copy_len;

		if(internal->buf_end == internal->buf_size)
			ok = _alsa_write_buffer(internal);
	}

	return ok;
}


int ao_plugin_close(ao_device *device)
{
	ao_alsa_internal *internal = (ao_alsa_internal *) device->internal;
	int result;

	/* Clear buffer */
	result = _alsa_write_buffer(internal);
	snd_pcm_close(internal->pcm_handle);
	free(internal->buf);

	return result;
}


void ao_plugin_device_clear(ao_device *device)
{
	ao_alsa_internal *internal = (ao_alsa_internal *) device->internal;

	free(internal);
}
