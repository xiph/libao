/*
 *
 *  ao_alsa09.c
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

#include <alsa/asoundlib.h>
#include <ao/ao.h>
#include <ao/plugin.h>

#define AO_ALSA_BUF_SIZE 4096
#define AO_ALSA_PERIODS  4

static char *ao_alsa_options[] = {
	"dev",
	"buf_size"
        "periods"
};
static ao_info ao_alsa_info =
{
	AO_TYPE_LIVE,
	"Advanced Linux Sound Architecture (ALSA) output",
	"alsa09",
	"Bill Currie <bill@taniwha.org>",
	"Outputs to the Advanced Linux Sound Architecture version 0.9.x.",
	AO_FMT_NATIVE,
	30,
	ao_alsa_options,
	3
};


typedef struct ao_alsa_internal
{
	snd_pcm_t *pcm_handle;
	int buf_size;
	int sample_size;
	int periods;
	char *dev;
} ao_alsa_internal;


int ao_plugin_test()
{
	snd_pcm_t *handle;
	int err;

	/* Use nonblock flag when testing to avoid getting stuck if the device
	   is in use. */
	err = snd_pcm_open(&handle, "default", SND_PCM_STREAM_PLAYBACK,
			   SND_PCM_NONBLOCK);

	if (err != 0)
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
	internal->periods = AO_ALSA_PERIODS;
	internal->dev = strdup ("default");
	if (!internal->dev) {
		free (internal);
		return 0;
	}
	
	device->internal = internal;

	return 1; /* Memory alloc successful */
}


int ao_plugin_set_option(ao_device *device, const char *key, const char *value)
{
	ao_alsa_internal *internal = (ao_alsa_internal *) device->internal;

	if (!strcmp(key, "dev")) {
		if (internal->dev)
			free (internal->dev);
		internal->dev = strdup (value);
		if (!internal->dev)
			return 0;
	}
	else if (!strcmp(key, "buf_size"))
		internal->buf_size = atoi(value);
	else if (!strcmp(key, "periods"))
		internal->periods = atoi(value);

	return 1;
}

int ao_plugin_open(ao_device *device, ao_sample_format *format)
{
	ao_alsa_internal *internal = (ao_alsa_internal *) device->internal;
	snd_pcm_hw_params_t *hwparams;

	int err;
	int fmt;
	char *cmd;


	/* Open the ALSA device */
	err = snd_pcm_open(&(internal->pcm_handle), internal->dev,
			   SND_PCM_STREAM_PLAYBACK, 0);

	snd_pcm_hw_params_alloca(&hwparams);

	cmd = "snd_pcm_hw_params_any";
	err = snd_pcm_hw_params_any(internal->pcm_handle, hwparams);
	if (err < 0)
		goto error;

	cmd = "snd_pcm_hw_params_set_access";
	err = snd_pcm_hw_params_set_access(internal->pcm_handle, hwparams,
			SND_PCM_ACCESS_RW_INTERLEAVED);
	if (err < 0)
		goto error;

	switch (format->bits) {
	case 8  : fmt = SND_PCM_FORMAT_S8;
		  break;
	case 16 : fmt = 
		  device->client_byte_format == AO_FMT_BIG ?
		  SND_PCM_FORMAT_S16_BE : SND_PCM_FORMAT_S16_LE;
		  device->driver_byte_format = device->client_byte_format;
		  break;
	default : return 0;
	}
	cmd = "snd_pcm_hw_params_set_format";
	err = snd_pcm_hw_params_set_format(internal->pcm_handle, hwparams, fmt);
	if (err < 0)
		goto error;

	cmd = "snd_pcm_hw_params_set_channels";
	if (format->channels == 1 || format->channels == 2)
		err = snd_pcm_hw_params_set_channels(internal->pcm_handle,
				hwparams, format->channels);
	else
		return 0;

	if (err < 0)
		goto error;
	internal->sample_size = format->bits * format->channels / 8;

	cmd = "snd_pcm_hw_params_set_rate";
	err = snd_pcm_hw_params_set_rate_near(internal->pcm_handle, hwparams,
			format->rate, 0);
	if (err < 0)
		goto error;

	cmd = "snd_pcm_hw_params_set_period_size";
	err = snd_pcm_hw_params_set_period_size(internal->pcm_handle, hwparams,
			internal->buf_size / internal->sample_size, 0);
	if (err < 0)
		goto error;

	cmd = "snd_pcm_hw_params_set_periods";
	err = snd_pcm_hw_params_set_periods(internal->pcm_handle, hwparams,
			internal->periods * format->channels, 0);
	if (err < 0)
		goto error;

	cmd = "snd_pcm_hw_params";
	err = snd_pcm_hw_params(internal->pcm_handle, hwparams);
	if (err < 0)
		goto error;

	cmd = "snd_pcm_prepare";
	err = snd_pcm_prepare(internal->pcm_handle);
	if (err < 0)
		goto error;

	return 1;
error:
	fprintf(stderr, "ALSA %s error: %s\n", cmd, snd_strerror(err));
	snd_pcm_close(internal->pcm_handle);
	return 0;
}

int ao_plugin_play(ao_device *device, const char *output_samples, 
		uint_32 num_bytes)
{
	ao_alsa_internal *internal = (ao_alsa_internal *) device->internal;
	int res;
	char *buf = (char *)output_samples;
	int len = num_bytes / internal->sample_size;

	do {
		res = snd_pcm_writei(internal->pcm_handle, buf, len);
		if (res > 0) {
			len -= res;
			buf += res;
		}
	} while (len > 0 && (res > 0 || res == -EAGAIN));
	if (res == -EPIPE) {
		/* fprintf(stderr, "ALSA: underrun. resetting stream\n"); */
		snd_pcm_prepare(internal->pcm_handle);
		res = snd_pcm_writei(internal->pcm_handle, buf, len);
		if (res != len) {
			fprintf(stderr, "ALSA write error: %s\n", snd_strerror(res));
			return 0;
		} else if (res < 0) {
			fprintf(stderr, "ALSA write error: %s\n", snd_strerror(res));
			return 0;
		}
	}


	return 1;
}


int ao_plugin_close(ao_device *device)
{
	ao_alsa_internal *internal = (ao_alsa_internal *) device->internal;

	snd_pcm_close(internal->pcm_handle);

	return 1;
}


void ao_plugin_device_clear(ao_device *device)
{
	ao_alsa_internal *internal = (ao_alsa_internal *) device->internal;
	if (internal->dev)
		free (internal->dev);
	free(internal);
}
