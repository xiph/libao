/*
 *
 *  mmsound.c
 *
 *      Copyright (C) Matthew Brown - Sept. July 2001
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


#define MAXBUF 5            // Max buffers in circular buffer queue
#define DEFAULTBUFTIME 3    // default to three seconds total buffer time

typedef struct ao_mmsound_internal 
{
	HWAVEOUT          m_hWaveOut;
	WAVEHDR	          m_waveHeader[MAXBUF];
	uint_32           buf_size;
	void             *buffer[MAXBUF];
  int               next_in;
  int               next_out;
  int               timeout_msec;
  HANDLE            hSema;  // semaphore for foreground/background coordination
  CRITICAL_SECTION  lock;   // concurrency control for atomic state updates
} ao_mmsound_internal;

static char *ao_mmsound_options[] = {"buf_size"};

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
  ao_mmsound_internal *internal = (ao_mmsound_internal *)dwInstance;

//  printf("*********  %x   Next in: %d  Next out: %d\n", 
//    uMsg, internal->next_in, internal->next_out);

	if (uMsg == WOM_DONE)
	{
    EnterCriticalSection(&internal->lock);
    internal->next_out++;

    if (internal->next_out == internal->next_in)
    {
      internal->next_in = internal->next_out = 0;
		  ReleaseSemaphore(internal->hSema, 1, NULL);
    }

    LeaveCriticalSection(&internal->lock);
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
  int i;
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
							(DWORD)device->internal,
							(DWORD)CALLBACK_FUNCTION);

	if (errCode != MMSYSERR_NOERROR) return 0;

	if(internal->buf_size == 0)
	{
		internal->buf_size = wfx.nAvgBytesPerSec * DEFAULTBUFTIME;
	}

  internal->timeout_msec = (internal->buf_size / wfx.nAvgBytesPerSec) * 1000;

  for (i=0; i<MAXBUF; i++)
  {
	  internal->buffer[i] = malloc(internal->buf_size);
    memset(&internal->m_waveHeader[i],0,sizeof(WAVEHDR));
  }

  internal->next_in = internal->next_out = 0;

	internal->hSema = CreateSemaphore(NULL, MAXBUF-1, MAXBUF, NULL);
  InitializeCriticalSection(&internal->lock);

	device->driver_byte_format = AO_FMT_NATIVE;
	
	return 1;
}

/*
 * play the sample to the already opened file descriptor
 */
int ao_plugin_play(ao_device *device, const char *output_samples, 
		uint_32 num_bytes)
{
	ao_mmsound_internal *internal = (ao_mmsound_internal *) device->internal;

  EnterCriticalSection(&internal->lock);
	
	if(num_bytes > internal->buf_size)
  {
    LeaveCriticalSection(&internal->lock);
		return 0;
  }

//  printf("*** Next in: %d   Next out: %d\n", 
//    internal->next_in, internal->next_out);
	if (internal->m_waveHeader[internal->next_in].dwFlags&WHDR_PREPARED)
		waveOutUnprepareHeader(internal->m_hWaveOut,
                          &internal->m_waveHeader[internal->next_in],
                           sizeof(WAVEHDR));
	
	// Prepare internal->buffer[n] to be inserted into WaveOut buffer.
	memcpy(internal->buffer[internal->next_in], output_samples, num_bytes);

	internal->m_waveHeader[internal->next_in].lpData = 
    (char*)internal->buffer[internal->next_in];

	internal->m_waveHeader[internal->next_in].dwBufferLength = 
    (unsigned long)num_bytes;

	waveOutPrepareHeader(internal->m_hWaveOut,
                      &internal->m_waveHeader[internal->next_in], sizeof(WAVEHDR));

	// Send internal->buffer[n] to the WaveOut device buffer.
	waveOutWrite(internal->m_hWaveOut,
              &internal->m_waveHeader[internal->next_in], sizeof(WAVEHDR));

  internal->next_in++;
  LeaveCriticalSection(&internal->lock);
  WaitForSingleObject(internal->hSema, internal->timeout_msec);

	return 1;
}

int ao_plugin_close(ao_device *device)
{
  int i;
	ao_mmsound_internal *internal = (ao_mmsound_internal *) device->internal;


  // Wait for last block to finish playing
  while ( internal->next_in !=  internal->next_out )
  {
    WaitForSingleObject(internal->hSema, internal->timeout_msec);
  }

	CloseHandle(internal->hSema);
  DeleteCriticalSection(&internal->lock);

  for (i=0; i<MAXBUF; i++)
	  free(internal->buffer[i]);
	
	waveOutReset(internal->m_hWaveOut);
	waveOutClose(internal->m_hWaveOut);

	return 1;
}

void ao_plugin_device_clear(ao_device *device)
{
	ao_mmsound_internal *internal = (ao_mmsound_internal *) device->internal;

	free(internal);
}
