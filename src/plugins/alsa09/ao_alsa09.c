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
 *  Largely rewritten 2/18/2002 Kevin Cody Jr <kevinc@wuff.dhs.org>
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

/* default 500 millisecond buffer */
#define AO_ALSA_BUFFER_TIME 500000

/* the period time is calculated if not given as an option
 * note, playback starts after four of these are in the buffer */
#define AO_ALSA_PERIOD_TIME 0

/* number of samples between interrupts
 * supplying a period_time to ao overrides the use of this  */
#define AO_ALSA_SAMPLE_XFER 256

static char *ao_alsa_options[] = {
	"dev",
	"buffer_time",
        "period_time"
};


static ao_info ao_alsa_info =
{
	AO_TYPE_LIVE,
	"Advanced Linux Sound Architecture (ALSA) output",
	"alsa09",
	"Bill Currie <bill@taniwha.org>/Kevin Cody, Jr. <kevinc@wuff.dhs.org>",
	"Outputs to the Advanced Linux Sound Architecture version 0.9.x.",
	AO_FMT_NATIVE,
	35,
	ao_alsa_options,
	3
};


typedef struct ao_alsa_internal
{
	snd_pcm_t *pcm_handle;
	int buffer_time;
	int period_time;
	int buffer_size;
	int period_size;
	int sample_size;
	int bitformat;
	char *dev;
	char *cmd;
} ao_alsa_internal;


/* determine if parameters are requires for this particular plugin */
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


/* return the address of the driver info structure */
ao_info *ao_plugin_driver_info(void)
{
	return &ao_alsa_info;
}


/* initialize internal data structures */
int ao_plugin_device_init(ao_device *device)
{
	ao_alsa_internal *internal;

	internal = (ao_alsa_internal *) malloc(sizeof(ao_alsa_internal));

	if (internal == NULL)	
		return 0;
	
	internal->buffer_time = AO_ALSA_BUFFER_TIME;
	internal->period_time = AO_ALSA_PERIOD_TIME;

	if (!(internal->dev = strdup("default"))) {
		free (internal);
		return 0;
	}
	
	device->internal = internal;

	return 1;
}


/* pass application parameters regarding the sound device */
int ao_plugin_set_option(ao_device *device, const char *key, const char *value)
{
	ao_alsa_internal *internal = (ao_alsa_internal *) device->internal;

	if (!strcmp(key, "dev")) {
		if (internal->dev)
			free (internal->dev);
		if (!(internal->dev = strdup(value)))
			return 0;
	}
	else if (!strcmp(key, "buffer_time"))
		internal->buffer_time = atoi(value);
	else if (!strcmp(key, "period_time"))
		internal->period_time = atoi(value);

	return 1;
}


/* determine the alsa bitformat for a given bitwidth and endianness */
static inline int alsa_get_sample_bitformat(int bitwidth, int bigendian)
{
	int ret;

	switch (bitwidth) {
	case 8  : ret = SND_PCM_FORMAT_S8;
		  break;
	case 16 : ret = SND_PCM_FORMAT_S16;
		  break;
	case 24 : ret = SND_PCM_FORMAT_S24;
		  break;
	case 32 : ret = SND_PCM_FORMAT_S32;
		  break;
	default : fprintf(stderr,"ALSA: invalid bitwidth %d\n", bitwidth);
		  return -1;
	}

	return ret;
}


/* setup alsa data format and buffer geometry */
static inline int alsa_set_hwparams(ao_alsa_internal *internal,
		ao_sample_format *format)
{
	snd_pcm_hw_params_t   *params;
	snd_pcm_access_mask_t *access;
	int err;

	/* allocate the hardware parameter structure */
	snd_pcm_hw_params_alloca(&params);
	snd_pcm_access_mask_alloca(&access);

	/* fetch the current hardware parameters */
	internal->cmd = "snd_pcm_hw_params_any";
	err = snd_pcm_hw_params_any(internal->pcm_handle, params);
	if (err < 0)
		return err;

	/* create a null access mask */
	snd_pcm_access_mask_none(access);

#ifdef USE_ALSA_MMIO
	/* allow interleaved memory-mapped access */
	snd_pcm_access_mask_set(access, SND_PCM_ACCESS_MMAP_INTERLEAVED);
#else
	/* allow interleaved non memory-mapped access */
	snd_pcm_access_mask_set(access, SND_PCM_ACCESS_RW_INTERLEAVED);
#endif

	/* commit the access value to params structure */
	internal->cmd = "snd_pcm_hw_params_set_access";
	err = snd_pcm_hw_params_set_access_mask(internal->pcm_handle,
			params, access);
	if (err < 0)
		return err;

	/* set the sample bitformat */
	internal->cmd = "snd_pcm_hw_params_set_format";
	err = snd_pcm_hw_params_set_format(internal->pcm_handle,
			params, internal->bitformat);
	if (err < 0)
		return err;

	/* set the number of channels */
	internal->cmd = "snd_pcm_hw_params_set_channels";
	err = snd_pcm_hw_params_set_channels(internal->pcm_handle,
			params, format->channels);
	if (err < 0)
		return err;

	/* save the sample size in bytes for posterity */
	internal->sample_size = format->bits * format->channels / 8;

	/* set the sample rate */
	internal->cmd = "snd_pcm_hw_params_set_rate_near";
	err = snd_pcm_hw_params_set_rate_near(internal->pcm_handle,
			params, format->rate, 0);
	if (err < 0)
		return err;

	/* set the length of the hardware sample buffer in milliseconds */
	internal->cmd = "snd_pcm_hw_params_set_buffer_time_near";
	err = snd_pcm_hw_params_set_buffer_time_near(internal->pcm_handle,
			params, internal->buffer_time, 0);
	if (err < 0)
		return err;

	/* save the buffer time in case alsa overrode it */
	internal->buffer_time = err;

	/* calculate a period time of one half sample time */
	if ((internal->period_time == 0) && (format->rate > 0))
		internal->period_time =
			1000000 * AO_ALSA_SAMPLE_XFER / format->rate;

	/* set the time per hardware sample transfer */
	internal->cmd = "snd_pcm_hw_params_set_period_time_near";
	err = snd_pcm_hw_params_set_period_time_near(internal->pcm_handle,
			params, internal->period_time, 0);
	if (err < 0)
		return err;

	/* commit the params structure to the hardware via ALSA */
	internal->cmd = "snd_pcm_hw_params";
	err = snd_pcm_hw_params(internal->pcm_handle, params);
	if (err < 0)
		return err;

	/* save the period size in bytes for posterity */
	internal->period_size = snd_pcm_hw_params_get_period_size(params, 0);

	/* save the buffer size in bytes for posterity */
	internal->buffer_size = snd_pcm_hw_params_get_buffer_size(params);

	return 1;
}


/* setup alsa data transfer behavior */
static inline int alsa_set_swparams(ao_alsa_internal *internal)
{
	snd_pcm_sw_params_t   *params;
	int err;

	/* allocate the software parameter structure */
	snd_pcm_sw_params_alloca(&params);

	/* fetch the current software parameters */
	internal->cmd = "snd_pcm_sw_params_current";
	err = snd_pcm_sw_params_current(internal->pcm_handle, params);
	if (err < 0)
		return err;

	/* require a minimum of one full transfer in the buffer */
	internal->cmd = "snd_pcm_sw_params_set_avail_min";
	err = snd_pcm_sw_params_set_avail_min(internal->pcm_handle, params,
			internal->period_size);
	if (err < 0)
		return err;

	/* allow transfers to start when there are four periods */
	internal->cmd = "snd_pcm_sw_params_set_start_threshold";
	err = snd_pcm_sw_params_set_start_threshold(internal->pcm_handle,
			params, internal->period_size << 2);
	if (err < 0)
		return err;

	/* do not align transfers */
	internal->cmd = "snd_pcm_sw_params_set_xfer_align";
	err = snd_pcm_sw_params_set_xfer_align(internal->pcm_handle, params, 1);
	if (err < 0)
		return err;

	/* commit the params structure to ALSA */
	internal->cmd = "snd_pcm_sw_params";
	err = snd_pcm_sw_params(internal->pcm_handle, params);
	if (err < 0)
		return err;

	return 1;
}


/* prepare the audio device for playback */
int ao_plugin_open(ao_device *device, ao_sample_format *format)
{
	ao_alsa_internal *internal  = (ao_alsa_internal *) device->internal;
	int err;

	/* Get the ALSA bitformat first to make sure it's valid */
	err = alsa_get_sample_bitformat(format->bits,
			device->client_byte_format == AO_FMT_BIG);
	if (err < 0)
		goto error;

	internal->bitformat = err;

	/* Open the ALSA device */
	internal->cmd = "snd_pcm_open";
	err = snd_pcm_open(&(internal->pcm_handle), internal->dev,
			   SND_PCM_STREAM_PLAYBACK, 0);
	if (err < 0) {
		internal->pcm_handle = NULL;
		goto error;
	}

	/* Set up the hardware parameters, ie sample and buffer specs */
	err = alsa_set_hwparams(internal, format);
	if (err < 0)
		goto error;

	/* Set up the software parameters, ie de-buffering specs */
	err = alsa_set_swparams(internal);
	if (err < 0)
		goto error;

	/* alsa's endinness will be the same as the application's */
	if (format->bits > 8)
		device->driver_byte_format = device->client_byte_format;

	return 1;

error:
	fprintf(stderr, "ALSA %s error: %s\n",
			internal->cmd, snd_strerror(err));
	if (internal->pcm_handle)
		snd_pcm_close(internal->pcm_handle);
	return 0;
}


/* recover from an alsa exception */
static inline int alsa_error_recovery(ao_alsa_internal *internal, int err)
{
	if (err == -EPIPE) {
		/* FIXME: underrun length detection */
		fprintf(stderr,"ALSA: underrun, at least %dms.\n", 0);
		/* output buffer underrun */
		internal->cmd = "underrun recovery: snd_pcm_prepare";
		err = snd_pcm_prepare(internal->pcm_handle);
		if (err < 0)
			return -1;
	} else if (err == -ESTRPIPE) {
		/* application was suspended, wait until suspend flag clears */
		internal->cmd = "suspend recovery: snd_pcm_prepare";
		while ((err = snd_pcm_resume(internal->pcm_handle)) == -EAGAIN)
			sleep (1);

		if (err < 0) {
			/* unable to wake up pcm device, restart it */
			internal->cmd = "suspend recovery: snd_pcm_prepare";
			err = snd_pcm_prepare(internal->pcm_handle);
			if (err < 0)
				return err;
		}
		return 0;
	}

	/* error isn't recoverable */
	return err;
}


/* play num_bytes of audio data */
int ao_plugin_play(ao_device *device, const char *output_samples, 
		uint_32 num_bytes)
{
	ao_alsa_internal *internal = (ao_alsa_internal *) device->internal;
       	uint_32 len = num_bytes / internal->sample_size;
	char *ptr = (char *) output_samples;
	int err;

	/* the entire buffer might not transfer at once */
	while (len > 0) {
		/* try to write the entire buffer at once */
#ifdef USE_ALSA_MMIO
		err = snd_pcm_mmap_writei(internal->pcm_handle, ptr, len);
#else
		err = snd_pcm_writei(internal->pcm_handle, ptr, len);
#endif

		/* it's possible that no data was transferred */
		if (err == -EAGAIN)
			continue;

		if (err < 0) {
			/* this might be an error, or an exception */
			err = alsa_error_recovery(internal, err);
			if (err < 0) {
				fprintf(stderr,"ALSA write error: %s\n",
						snd_strerror(err));
				return 0;
			}

			/* abandon the rest of the buffer */
			break;
		}

		/* decrement the sample counter */
		len -= err;

		/* adjust the start pointer */
		ptr += err * internal->sample_size;
	}

	return 1;
}


/* close the audio device */
int ao_plugin_close(ao_device *device)
{
	ao_alsa_internal *internal;

	if (device) {
		if ((internal = (ao_alsa_internal *) device->internal)) {
			if (internal->pcm_handle) {
				snd_pcm_drain(internal->pcm_handle);
				snd_pcm_close(internal->pcm_handle);
			} else
				fprintf(stderr,"ao_plugin_close called with uninitialized ao_device->internal->pcm_handle\n");
		} else
			fprintf(stderr,"ao_plugin_close called with uninitialized ao_device->internal\n");
	} else
		fprintf(stderr,"ao_plugin_close called with uninitialized ao_device\n");

	return 1;
}


/* free the internal data structures */
void ao_plugin_device_clear(ao_device *device)
{
	ao_alsa_internal *internal;

	if (device) {
		if ((internal = (ao_alsa_internal *) device->internal)) {
			if (internal->dev)
				free (internal->dev);
			else
				fprintf(stderr,"ao_plugin_device_clear called with uninitialized ao_device->internal->dev\n");
			if (internal->cmd)
				internal->cmd = NULL;

			free(device->internal);
		} else
			fprintf(stderr,"ao_plugin_device_clear called with uninitialized ao_device->internal\n");
	} else
		fprintf(stderr,"ao_plugin_device_clear called with uninitialized ao_device\n");
}

