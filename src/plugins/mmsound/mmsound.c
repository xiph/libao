/*
 *
 *  mmsound.c
 *
 *      Copyright (C) Matthew Brown - Sept. 2001
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

#include <windows.h>
#include <stdio.h>
#include <mmsystem.h>
#include <ao/ao.h>
#include <ao/plugin.h>

typedef struct ao_mmsound_internal {
	HWAVEOUT m_hWaveOut;
	WAVEHDR	m_waveHeader[2];
	uint_32 buf_size;
	uint_32 num_bytes[2];
	void *buffer[2];
	int currentb;
} ao_mmsound_internal;

static char *ao_mmsound_options[] = {"buf_size"};

HANDLE devicenum;
CRITICAL_SECTION  mylock;

static ao_info ao_mmsound_info =
{
	AO_TYPE_LIVE,
	"Win9x waveout audio output ",
	"mmsound",
	"Matthew Brown <matt@digitalblues.org>",
	"WARNING: This driver is untested!",
	AO_FMT_NATIVE,
	20,
	ao_mmsound_options,
	1
};


static	void CALLBACK waveOutProc(HWAVEOUT hwo,UINT uMsg,DWORD dwInstance,DWORD dwParam1,DWORD dwParam2)
{
	if (uMsg == WOM_DONE)
	{
		EnterCriticalSection(&mylock);
		ReleaseSemaphore(devicenum, 1, NULL);
		LeaveCriticalSection(&mylock);
	}
}

int ao_plugin_test()
{
	return 1; /* This plugin works in default mode */
}

ao_info *ao_plugin_driver_info(void)
{
	return &ao_mmsound_info;
}

int ao_plugin_device_init(ao_device *device)
{
	ao_mmsound_internal *internal;

	internal = (ao_mmsound_internal *) malloc(sizeof(ao_mmsound_internal));

	if (internal == NULL)	
		return 0; /* Could not initialize device memory */

	device->internal = internal;

	internal->buf_size = 0;

	return 1; /* Memory alloc successful */
}

int ao_plugin_set_option(ao_device *device, const char *key, const char *value)
{
	ao_mmsound_internal *internal = (ao_mmsound_internal *) device->internal;
	if (!strcmp(key, "buf_size"))
		internal->buf_size = atoi(value);
	
	return 1;
}

/*
 * open the audio device for writing to
 */
int ao_plugin_open(ao_device *device, ao_sample_format *format)
{
	ao_mmsound_internal *internal = (ao_mmsound_internal *) device->internal;
	
	MMRESULT errCode;
	WAVEFORMATEX wfx;

	wfx.wFormatTag = 1;	// WAVE_FORMAT_PCM
	wfx.nChannels = format->channels;
	wfx.nSamplesPerSec = format->rate;
	wfx.nAvgBytesPerSec = ((format->rate)*((format->channels*format->bits)/8));
	wfx.nBlockAlign = ((format->channels*format->bits)/8);
	wfx.wBitsPerSample = format->bits;
	wfx.cbSize = 0;

	errCode = waveOutOpen(	&internal->m_hWaveOut,
							WAVE_MAPPER,
							&wfx,
							(DWORD)waveOutProc,
							NULL,
							(DWORD)CALLBACK_FUNCTION);

	if (errCode != MMSYSERR_NOERROR) return 0;

	if(internal->buf_size == 0)
	{
		internal->buf_size = 352800;
	}

	memset(&internal->m_waveHeader[0],0,sizeof(WAVEHDR));
	memset(&internal->m_waveHeader[1],0,sizeof(WAVEHDR));
	internal->buffer[0] = malloc(internal->buf_size);
	internal->buffer[1] = malloc(internal->buf_size);
	internal->num_bytes[0] = 0;
	internal->num_bytes[1] = 0;
	internal->currentb = 0;
	
	devicenum = CreateSemaphore(NULL, 0, 2, NULL);

	device->driver_byte_format = AO_FMT_NATIVE;

	InitializeCriticalSection(&mylock);

	return 1;
}

/*
 * play the sample to the already opened file descriptor
 */
int ao_plugin_play(ao_device *device, const char *output_samples, 
		uint_32 num_bytes)
{
	ao_mmsound_internal *internal = (ao_mmsound_internal *) device->internal;
	
	if(num_bytes > internal->buf_size)
		return 0;

	if(internal->num_bytes[internal->currentb] != 0)
		WaitForSingleObject(devicenum, INFINITE);

	if (internal->m_waveHeader[internal->currentb].dwFlags&WHDR_PREPARED)
		waveOutUnprepareHeader(internal->m_hWaveOut,&internal->m_waveHeader[internal->currentb],sizeof(WAVEHDR));
	
	memcpy(internal->buffer[internal->currentb], output_samples, num_bytes);
	internal->num_bytes[internal->currentb] = num_bytes;
	
	internal->m_waveHeader[internal->currentb].lpData = (char*)internal->buffer[internal->currentb];
	internal->m_waveHeader[internal->currentb].dwBufferLength = (unsigned long)internal->num_bytes[internal->currentb];
	waveOutPrepareHeader(internal->m_hWaveOut,&internal->m_waveHeader[internal->currentb],sizeof(WAVEHDR));

	waveOutWrite(internal->m_hWaveOut,&internal->m_waveHeader[internal->currentb],sizeof(WAVEHDR));

	internal->currentb++;
	if(internal->currentb == 2)
		internal->currentb = 0;

	return 1;
}

int ao_plugin_close(ao_device *device)
{
	ao_mmsound_internal *internal = (ao_mmsound_internal *) device->internal;

	if (internal->m_waveHeader[0].dwFlags&WHDR_PREPARED)
		waveOutUnprepareHeader(internal->m_hWaveOut,&internal->m_waveHeader[0],sizeof(WAVEHDR));

	if (internal->m_waveHeader[1].dwFlags&WHDR_PREPARED)
		waveOutUnprepareHeader(internal->m_hWaveOut,&internal->m_waveHeader[1],sizeof(WAVEHDR));

	WaitForSingleObject(devicenum, 2000);
	if(internal->num_bytes[1] != 0)
		WaitForSingleObject(devicenum, 4000);

	waveOutReset(internal->m_hWaveOut);
	waveOutClose(internal->m_hWaveOut);
	CloseHandle(devicenum);
	DeleteCriticalSection(&mylock);
	free(internal->buffer[0]);
	free(internal->buffer[1]);

	return 1;
}

void ao_plugin_device_clear(ao_device *device)
{
	ao_mmsound_internal *internal = (ao_mmsound_internal *) device->internal;

	free(internal);
}