/*
 *  ao_haiku.cpp
 *
 *      Copyright (C) Julian Harnath - March 2014
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
 ********************************************************************/

#include <algorithm>
#include <string.h>
#include <stdio.h>

#include <MediaDefs.h>
#include <MediaRoster.h>
#include <OS.h>
#include <SoundPlayer.h>

#include <ao/ao.h>
#include <ao/plugin.h>


static char* ao_haiku_options[] = {
	"matrix"
};


struct ao_info ao_haiku_info = {
	AO_TYPE_LIVE,
	"Haiku Media Kit Output",
	"haiku",
	"Julian Harnath <julian.harnath@rwth-aachen.de>",
	"Outputs to the Haiku Media Kit",
	AO_FMT_NATIVE,
	30,
	ao_haiku_options,
	sizeof(ao_haiku_options) / sizeof(*ao_haiku_options)
};


typedef struct ao_haiku_internal {
	media_raw_audio_format* format;
	BSoundPlayer* sound_player;

	sem_id new_buffer;
	sem_id buffer_done;

	uint8* buffer;
	size_t buffer_size;
	size_t buffer_filled;
} ao_haiku_internal;


int
ao_plugin_test()
{
	BSoundPlayer testPlayer;
	return testPlayer.InitCheck() == B_OK ? 1 :0;
}


ao_info*
ao_plugin_driver_info()
{
	return &ao_haiku_info;
}


int
ao_plugin_device_init(ao_device* device)
{
	ao_haiku_internal* const internal = (ao_haiku_internal*)calloc(1,
		sizeof(ao_haiku_internal));
	if (internal == NULL)
		return 0;

	internal->format = (media_raw_audio_format*)malloc(
		sizeof(media_raw_audio_format));
	if (internal->format == NULL) {
		free(internal);
		return 0;
	}

	internal->new_buffer = create_sem(0, "New buffer request");
	internal->buffer_done = create_sem(1, "Buffer done");

	device->output_matrix_order = AO_OUTPUT_MATRIX_FIXED;
	device->internal = internal;

	return 1;
}


int
ao_plugin_set_option(ao_device* device, const char* key, const char* value)
{
	return 1;
}


static void
fill_buffer(void* cookie, void* buffer, size_t size,
	const media_raw_audio_format& format)
{
	ao_haiku_internal* const internal = (ao_haiku_internal*)cookie;

	internal->buffer = (uint8*)buffer;
	internal->buffer_size = size;
	internal->buffer_filled = 0;
	release_sem(internal->new_buffer);
	acquire_sem(internal->buffer_done);
}


int
ao_plugin_open(ao_device* device, ao_sample_format* format)
{
	ao_haiku_internal* const internal = (ao_haiku_internal*)device->internal;
	media_raw_audio_format* const mediaRawFormat = internal->format;

	mediaRawFormat->frame_rate = format->rate;
	mediaRawFormat->channel_count = device->output_channels;

	switch (format->bits) {
		case 8:
			mediaRawFormat->format = media_raw_audio_format::B_AUDIO_CHAR;
			break;
		case 16:
			mediaRawFormat->format = media_raw_audio_format::B_AUDIO_SHORT;
			break;
		case 32:
			mediaRawFormat->format = media_raw_audio_format::B_AUDIO_INT;
			break;
		default:
			aerror("Unsupported sample bit depth");
			return 0;
	}

	device->driver_byte_format = AO_FMT_NATIVE;
	if (B_HOST_IS_LENDIAN)
		mediaRawFormat->byte_order = B_MEDIA_LITTLE_ENDIAN;
	else
		mediaRawFormat->byte_order = B_MEDIA_BIG_ENDIAN;

	mediaRawFormat->buffer_size = BMediaRoster::Roster()->AudioBufferSizeFor(
		mediaRawFormat->channel_count, mediaRawFormat->format,
		mediaRawFormat->frame_rate, B_UNKNOWN_BUS) * 2;

	internal->sound_player = new BSoundPlayer(mediaRawFormat, "ao player",
		fill_buffer, NULL, internal);

	if (internal->sound_player->InitCheck() != B_OK) {
		delete internal->sound_player;
		internal->sound_player = NULL;
		return 0;
	}

	internal->sound_player->SetVolume(1.0);
	internal->sound_player->Start();
	internal->sound_player->SetHasData(false);

	if (device->inter_matrix == NULL) {
		if (device->output_channels <= 2)
			device->inter_matrix = strdup("L,R");
	}

	return 1;
}


int
ao_plugin_play(ao_device* device, const char* output_samples,
	uint_32 num_bytes)
{
	ao_haiku_internal* const internal = (ao_haiku_internal*)device->internal;
	BSoundPlayer* const soundPlayer = internal->sound_player;

	if (num_bytes == 0) {
		soundPlayer->SetHasData(false);
		return 1;
	}

	soundPlayer->SetHasData(true);
	acquire_sem(internal->new_buffer);

	size_t bytesLeft = num_bytes;
	while (bytesLeft > 0) {
		if (internal->buffer_filled == internal->buffer_size) {
			// Request another buffer from BSoundPlayer
			release_sem(internal->buffer_done);
			acquire_sem(internal->new_buffer);
		}

		const size_t copyBytes = std::min(bytesLeft, internal->buffer_size
			- internal->buffer_filled);
		memcpy(internal->buffer + internal->buffer_filled, output_samples,
			copyBytes);
		internal->buffer_filled += copyBytes;
		output_samples += copyBytes;
		bytesLeft -= copyBytes;
	}

	if (internal->buffer_filled < internal->buffer_size) {
		// Continue filling this buffer the next time this function is called
		release_sem(internal->new_buffer);
	} else {
		// Buffer is full
		release_sem(internal->buffer_done);
		soundPlayer->SetHasData(false);
	}

	return 1;
}


int
ao_plugin_close(ao_device* device)
{
	ao_haiku_internal* const internal = (ao_haiku_internal*)device->internal;

	release_sem(internal->buffer_done);
	internal->sound_player->SetHasData(false);
	internal->sound_player->Stop();
	delete internal->sound_player;

	return 1;
}


void
ao_plugin_device_clear(ao_device* device)
{
	ao_haiku_internal* const internal = (ao_haiku_internal*)device->internal;

	free(internal->format);
	delete_sem(internal->new_buffer);
	delete_sem(internal->buffer_done);
	free(internal);
}
