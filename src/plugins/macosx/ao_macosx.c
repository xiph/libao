/*
 *
 *  ao_macosx.c
 *
 *      Original Copyright (C) Timothy J. Wood - Aug 2000
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

/*
  The MacOS X CoreAudio framework doesn't mesh as simply as some
  simpler frameworks do.  This is due to the fact that CoreAudio pulls
  audio samples rather than having them pushed at it (which is nice
  when you are wanting to do good buffering of audio).  */

#include <CoreAudio/AudioHardware.h>
#include <stdio.h>
#include <pthread.h>

#include "ao/ao.h"
#include "ao/plugin.h"

// Set this to 1 to see FIFO debugging messages
#define DEBUG_PIPE 0

//#define BUFFER_COUNT (323)
#define BUFFER_COUNT (2)

#ifndef MIN
#define MIN(a,b) ((a) < (b) ? (a) : (b))
#endif

#define true  1
#define false 0

static ao_info ao_macosx_info =
{
	AO_TYPE_LIVE,
	"MacOS X output",
	"macosx",
	"Timothy J. Wood <tjw@omnigroup.com>",
	"",
	AO_FMT_NATIVE,
	30,
	NULL,
	0
};


typedef struct ao_macosx_internal
{
    // Stuff describing the CoreAudio device
    AudioDeviceID                outputDeviceID;
    AudioStreamBasicDescription  outputStreamBasicDescription;
    
    // The amount of data CoreAudio wants each time it calls our IO function
    UInt32                       outputBufferByteCount;
    
    // Keep track of whether the output stream has actually been started/stopped
    Boolean                      started;
    Boolean                      isStopping;
    
    // Synchronization objects between the CoreAudio thread and the enqueuing thread
    pthread_mutex_t              mutex;
    pthread_cond_t               condition;

    // Our internal queue of samples waiting to be consumed by CoreAudio
    void                        *buffer;
    unsigned int                 bufferByteCount;
    unsigned int                 firstValidByteOffset;
    unsigned int                 validByteCount;
    
    // Temporary debugging state
    unsigned int bytesQueued;
    unsigned int bytesDequeued;
} ao_macosx_internal;

// The function that the CoreAudio thread calls when it wants more data
static OSStatus audioDeviceIOProc(AudioDeviceID inDevice, const AudioTimeStamp *inNow, const AudioBufferList *inInputData, const AudioTimeStamp *inInputTime, AudioBufferList *outOutputData, const AudioTimeStamp *inOutputTime, void *inClientData);

int ao_plugin_test()
{
	
	if (/* FIXME */ 0 )
		return 0; /* Cannot use this plugin with default parameters */
	else {
		return 1; /* This plugin works in default mode */
	}
}

ao_info *ao_plugin_driver_info(void)
{
	return &ao_macosx_info;
}


int ao_plugin_device_init(ao_device *device)
{
	ao_macosx_internal *internal;

	internal = (ao_macosx_internal *) malloc(sizeof(ao_macosx_internal));

	if (internal == NULL)	
		return 0; /* Could not initialize device memory */
	

	
	device->internal = internal;

	return 1; /* Memory alloc successful */
}


int ao_plugin_set_option(ao_device *device, const char *key, const char *value)
{
	ao_macosx_internal *internal = (ao_macosx_internal *) device->internal;

	/* No options */

	return 1;
}


int ao_plugin_open(ao_device *device, ao_sample_format *format)
{
    ao_macosx_internal *internal = (ao_macosx_internal *) device->internal;
    OSStatus status;
    UInt32 propertySize;
    int rc;
    
    if (format->rate != 44100) {
        fprintf(stderr, "ao_macosx_open: Only support 44.1kHz right now\n");
        return 0;
    }
    
    if (format->channels != 2) {
        fprintf(stderr, "ao_macosx_open: Only two channel audio right now\n");
        return 0;
    }

    propertySize = sizeof(internal->outputDeviceID);
    status = AudioHardwareGetProperty(kAudioHardwarePropertyDefaultOutputDevice, &propertySize, &(internal->outputDeviceID));
    if (status) {
        fprintf(stderr, "ao_macosx_open: AudioHardwareGetProperty returned %d\n", (int)status);
	return 0;
    }
    
    if (internal->outputDeviceID == kAudioDeviceUnknown) {
        fprintf(stderr, "ao_macosx_open: AudioHardwareGetProperty: outputDeviceID is kAudioDeviceUnknown\n");
	return 0;
    }
    
    propertySize = sizeof(internal->outputStreamBasicDescription);
    status = AudioDeviceGetProperty(internal->outputDeviceID, 0, false, kAudioDevicePropertyStreamFormat, &propertySize, &internal->outputStreamBasicDescription);
    if (status) {
        fprintf(stderr, "ao_macosx_open: AudioDeviceGetProperty returned %d when getting kAudioDevicePropertyStreamFormat\n", (int)status);
	return 0;
    }

    fprintf(stderr, "hardware format...\n");
    fprintf(stderr, "%f mSampleRate\n", internal->outputStreamBasicDescription.mSampleRate);
    fprintf(stderr, "%c%c%c%c mFormatID\n", (int)(internal->outputStreamBasicDescription.mFormatID & 0xff000000) >> 24,
                                            (int)(internal->outputStreamBasicDescription.mFormatID & 0x00ff0000) >> 16,
                                            (int)(internal->outputStreamBasicDescription.mFormatID & 0x0000ff00) >>  8,
                                            (int)(internal->outputStreamBasicDescription.mFormatID & 0x000000ff) >>  0);
    fprintf(stderr, "%5d mBytesPerPacket\n", (int)internal->outputStreamBasicDescription.mBytesPerPacket);
    fprintf(stderr, "%5d mFramesPerPacket\n", (int)internal->outputStreamBasicDescription.mFramesPerPacket);
    fprintf(stderr, "%5d mBytesPerFrame\n", (int)internal->outputStreamBasicDescription.mBytesPerFrame);
    fprintf(stderr, "%5d mChannelsPerFrame\n", (int)internal->outputStreamBasicDescription.mChannelsPerFrame);

    if (internal->outputStreamBasicDescription.mFormatID != kAudioFormatLinearPCM) {
        fprintf(stderr, "ao_macosx_open: Default Audio Device doesn't support Linear PCM!\n");
	return 0;
    }

    propertySize = sizeof(internal->outputBufferByteCount);
    
    internal->outputBufferByteCount = 8192;
    status = AudioDeviceSetProperty(internal->outputDeviceID, 0, 0, false, kAudioDevicePropertyBufferSize,
        propertySize, &internal->outputBufferByteCount);
        
    status = AudioDeviceGetProperty(internal->outputDeviceID, 0, false, kAudioDevicePropertyBufferSize, &propertySize, &internal->outputBufferByteCount);
    if (status) {
        fprintf(stderr, "ao_macosx_open: AudioDeviceGetProperty returned %d when getting kAudioDevicePropertyBufferSize\n", (int)status);
	return 0;
    }

    fprintf(stderr, "%5d outputBufferByteCount\n", (int)internal->outputBufferByteCount);

    // It appears that AudioDeviceGetProperty lies about this property in DP4
    // Set the actual value
    //internal->outputBufferByteCount = 32768;

    // Set the IO proc that CoreAudio will call when it needs data, but don't start
    // the stream yet.
    internal->started = false;
    status = AudioDeviceAddIOProc(internal->outputDeviceID, audioDeviceIOProc, internal);
    if (status) {
        fprintf(stderr, "ao_macosx_open: AudioDeviceAddIOProc returned %d\n", (int)status);
	return 0;
    }

    rc = pthread_mutex_init(&internal->mutex, NULL);
    if (rc) {
        fprintf(stderr, "ao_macosx_open: pthread_mutex_init returned %d\n", rc);
	return 0;
    }
    
    rc = pthread_cond_init(&internal->condition, NULL);
    if (rc) {
        fprintf(stderr, "ao_macosx_open: pthread_cond_init returned %d\n", rc);
	return 0;
    }
    
    /* Since we don't know how big to make the buffer until we open the device
       we allocate the buffer here instead of ao_plugin_device_init() */
    internal->bufferByteCount = BUFFER_COUNT * internal->outputBufferByteCount;
    internal->firstValidByteOffset = 0;
    internal->validByteCount = 0;
    internal->buffer = malloc(internal->bufferByteCount);
    memset(internal->buffer, 0, internal->bufferByteCount);
    if (!internal->buffer) {
        fprintf(stderr, "ao_macosx_open: Unable to allocate queue buffer.\n");
	return 0;
    }

    /* initialize debugging state */
    internal->bytesQueued = 0;
    internal->bytesDequeued = 0;
    
    device->driver_byte_format = AO_FMT_NATIVE;

    return 1;
}


int ao_plugin_play(ao_device *device, const char *output_samples, 
		uint_32 num_bytes)
{
    ao_macosx_internal *internal = (ao_macosx_internal *) device->internal;
    OSStatus status;

#if DEBUG_PIPE
    fprintf(stderr, "Enqueue: 0x%08x %d bytes\n", output_samples, num_bytes);
#endif

    while (num_bytes) {
        unsigned int bytesToCopy;
        unsigned int firstEmptyByteOffset, emptyByteCount;
        
        // Get a consistent set of data about the available space in the queue,
        // figure out the maximum number of bytes we can copy in this chunk,
        // and claim that amount of space
        pthread_mutex_lock(&internal->mutex);

        // Wait until there is some empty space in the queue
        emptyByteCount = internal->bufferByteCount - internal->validByteCount;
        while (emptyByteCount == 0) {
            pthread_cond_wait(&internal->condition, &internal->mutex);
            emptyByteCount = internal->bufferByteCount - internal->validByteCount;
        }

        // Compute the offset to the first empty byte and the maximum number of
        // bytes we can copy given the fact that the empty space might wrap
        // around the end of the queue.
        firstEmptyByteOffset = (internal->firstValidByteOffset + internal->validByteCount) % internal->bufferByteCount;
        if (firstEmptyByteOffset + emptyByteCount > internal->bufferByteCount)
            bytesToCopy = MIN(num_bytes, internal->bufferByteCount - firstEmptyByteOffset);
        else
            bytesToCopy = MIN(num_bytes, emptyByteCount);

        // Copy the bytes and get ready for the next chunk, if any
#if DEBUG_PIPE
        fprintf(stderr, "Enqueue:\tdst = 0x%08x src=0x%08x count=%d\n",
                internal->buffer + firstEmptyByteOffset, output_samples, bytesToCopy);
#endif
                
        memcpy(internal->buffer + firstEmptyByteOffset, output_samples, bytesToCopy);
        /*{
            unsigned int i;
            static unsigned char bufferIndex;
            
            bufferIndex++;
            memset(internal->buffer + firstEmptyByteOffset, bufferIndex, bytesToCopy);
        }*/
        
        num_bytes -= bytesToCopy;
        output_samples += bytesToCopy;
        internal->validByteCount += bytesToCopy;
        
        internal->bytesQueued += bytesToCopy;
        
        //fprintf(stderr, "Copy: %d bytes, %d bytes left\n", bytesToCopy, internal->availableByteCount);
        pthread_mutex_unlock(&internal->mutex);
        
        // We have to wait to start the device until we have some data queued.
        // It might be better to wait until we have some minimum amount of data
        // larger than whatever blob got enqueued here, but if we had a short
        // stream, we'd have to make sure that ao_macosx_close() would start
        // AND stop the stream when it had finished.  Yuck.  If the first
        // blob that is passed to us is large enough (and the caller passes
        // data quickly enough, this shouldn't be a problem. 
#if 1
        if (!internal->started) {
            internal->started = true;
            status = AudioDeviceStart(internal->outputDeviceID, audioDeviceIOProc);
            if (status) {
                fprintf(stderr, "ao_macosx_open: AudioDeviceStart returned %d\n", (int)status);
                
                // Can we do anything useful here?  The library doesn't expect this call
                // to be able to fail.
		return 0;
            }
        }
#endif
    }

    return 1;
}


int ao_plugin_close(ao_device *device)
{
    ao_macosx_internal *internal = (ao_macosx_internal *) device->internal;
    OSStatus status;

    // Only stop if we ever got started
    if (internal->started) {

        internal->isStopping = true;
        
        // Wait for any pending data to get flushed
        pthread_mutex_lock(&internal->mutex);
        while (internal->validByteCount)
            pthread_cond_wait(&internal->condition, &internal->mutex);
        pthread_mutex_unlock(&internal->mutex);
        
        status = AudioDeviceStop(internal->outputDeviceID, audioDeviceIOProc);
        if (status) {
            fprintf(stderr, "ao_macosx_close: AudioDeviceStop returned %d\n", (int)status);
            return 0;
        }
    }
    
    status = AudioDeviceRemoveIOProc(internal->outputDeviceID, audioDeviceIOProc);
    if (status) {
        fprintf(stderr, "ao_macosx_close: AudioDeviceRemoveIOProc returned %d\n", (int)status);
        return 0;
    }

    return 1;
}


void ao_plugin_device_clear(ao_device *device)
{
	ao_macosx_internal *internal = (ao_macosx_internal *) device->internal;

	free(internal->buffer);
	free(internal);
}


static OSStatus audioDeviceIOProc(AudioDeviceID inDevice, const AudioTimeStamp *inNow, const AudioBufferList *inInputData, const AudioTimeStamp *inInputTime, AudioBufferList *outOutputData, const AudioTimeStamp *inOutputTime, void *inClientData)
{
    ao_macosx_internal *internal = (ao_macosx_internal *)inClientData;
    short *sample;
    unsigned int validByteCount;
    float scale = (0.5f / SHRT_MAX), *outBuffer;
    unsigned int bytesToCopy, samplesToCopy;

    // Find the first valid frame and the number of valid frames
    pthread_mutex_lock(&internal->mutex);

    bytesToCopy = internal->outputBufferByteCount/2;
    validByteCount = internal->validByteCount;
    outBuffer = (float *)outOutputData->mBuffers[0].mData;
    
    if (validByteCount < bytesToCopy && !internal->isStopping) {
        // Not enough data ... let it build up a bit more before we start copying stuff over.
        // If we are stopping, of course, we should just copy whatever we have.
        memset(outBuffer, 0, bytesToCopy);
        pthread_mutex_unlock(&internal->mutex);
        return 0;
    }
    
    bytesToCopy = MIN(bytesToCopy, validByteCount);
    sample = internal->buffer + internal->firstValidByteOffset;
    samplesToCopy = bytesToCopy / sizeof(*sample);

    internal->bytesDequeued += bytesToCopy;

#if DEBUG_PIPE
    fprintf(stderr, "IO: outputTime=%f firstValid=%d valid=%d toCopy=%d queued=%d dequeued=%d sample=0x%08x\n",
            inOutputTime->mSampleTime,
            internal->firstValidByteOffset, internal->validByteCount, samplesToCopy, internal->bytesQueued, internal->bytesDequeued, sample);
#endif
    
    internal->validByteCount -= bytesToCopy;
    internal->firstValidByteOffset = (internal->firstValidByteOffset + bytesToCopy) % internal->bufferByteCount;
    
    // We don't have to deal with wrapping around in the buffer since the buffer is a
    // multiple of the output buffer size and we only copy on buffer at a time
    // (except on the last buffer when we may copy only a partial output buffer).
#warning On the last buffer, zero out the part of the buffer that does not have valid samples
    while (samplesToCopy--) {
        short x = *sample;
#warning The bytes in the buffer are currently in little endian, but we need big endian.  Supposedly these are going to be host endian at some point and the following line of code can go away.
/* They will go away now, I think. --- Stan */
/*        x = ((x & 0xff00) >> 8) | ((x & 0x00ff) << 8); */
        *outBuffer = x * scale;
        outBuffer++;
        sample++;
    }
    
    pthread_mutex_unlock(&internal->mutex);
    pthread_cond_signal(&internal->condition);
    
    return 0;
}


