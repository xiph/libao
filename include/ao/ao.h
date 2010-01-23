/*
 *
 *  ao.h 
 *
 *	Original Copyright (C) Aaron Holtzman - May 1999
 *      Modifications Copyright (C) Stan Seibert - July 2000, July 2001
 *      More Modifications Copyright (C) Jack Moffitt - October 2000
 *
 *  This file is part of libao, a cross-platform audio outputlibrary.  See
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
#ifndef __AO_H__
#define __AO_H__

#ifdef __cplusplus
extern "C"
{
#endif /* __cplusplus */

#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include "os_types.h"

/* --- Constants ---*/

#define AO_TYPE_LIVE 1
#define AO_TYPE_FILE 2


#define AO_ENODRIVER   1
#define AO_ENOTFILE    2
#define AO_ENOTLIVE    3
#define AO_EBADOPTION  4
#define AO_EOPENDEVICE 5
#define AO_EOPENFILE   6
#define AO_EFILEEXISTS 7

#define AO_EFAIL       100


#define AO_FMT_LITTLE 1
#define AO_FMT_BIG    2
#define AO_FMT_NATIVE 4

/* Specify the ordering channels will appear in the stream; not
   functionally necessary, but saves the application having to
   byte-twiddle streams into a single supported input order when AO
   will likelyhave to re-twiddle for specific hardware interfaces */

/* the native channel ordering used by OSS/ALSA/PULSE [L R BL BR C LFE SL SR] */
#define AO_CHANORDER_LINUX            0x00000100U
/* FLAC, WAV and WMA use Windows ordering [L R C LFE BL BR SL SR] */
#define AO_CHANORDER_WIN              0x00000200U
/* AC3, Vorbis use the classic 8-track theater order [L C R BL BR SL SR LFE] */
#define AO_CHANORDER_THEATER          0x00000300U
/* AAC and DTS use variant theater order [C LC RC L R BL BR LFE] */
#define AO_CHANORDER_AAC              0x00000400U
/* AIFF[-C] is yet different [L BL C R BR LFE [SL SR?]] */
#define AO_CHANORDER_AIF              0x00000500U

/* input channel masks; mark which channels are present */

/* not an exhaustive list, but what we will support for now */
#define AO_CHAN_LEFT                    0x00001000U
#define AO_CHAN_RIGHT                   0x00002000U
#define AO_CHAN_CENTER                  0x00004000U

#define AO_CHAN_REAR_SURROUND_LEFT      0x00008000U
#define AO_CHAN_REAR_SURROUND_RIGHT     0x00010000U
#define AO_CHAN_REAR_SURROUND_CENTER    0x00020000U

#define AO_CHAN_SIDE_SURROUND_LEFT      0x00040000U
#define AO_CHAN_SIDE_SURROUND_RIGHT     0x00080000U

#define AO_CHAN_LEFT_OF_CENTER          0x00100000U
#define AO_CHAN_RIGHT_OF_CENTER         0x00200000U

#define AO_CHAN_LFE                     0x40000000U


/* shortcuts */

#define AO_CHANMAP_2_1_STEREO         (AO_CHAN_LEFT |\
                                       AO_CHAN_RIGHT |\
                                       AO_CHAN_LFE)

#define AO_CHANMAP_3_0_WIDESTEREO     (AO_CHAN_LEFT |\
                                       AO_CHAN_RIGHT |\
                                       AO_CHAN_CENTER)

#define AO_CHANMAP_3_0_SURROUND_DOLBY (AO_CHAN_LEFT |\
                                       AO_CHAN_RIGHT |\
                                       AO_CHAN_REAR_SURROUND_CENTER)

#define AO_CHANMAP_4_0_QUAD           (AO_CHAN_LEFT |\
                                       AO_CHAN_RIGHT |\
                                       AO_CHAN_REAR_SURROUND_LEFT |\
                                       AO_CHAN_REAR_SURROUND_RIGHT)

#define AO_CHANMAP_4_0_SURROUND_DOLBY (AO_CHAN_LEFT |\
                                       AO_CHAN_RIGHT |\
                                       AO_CHAN_CENTER |\
                                       AO_CHAN_REAR_SURROUND_CENTER)

#define AO_CHANMAP_4_1_QUAD           (AO_CHAN_LEFT |\
                                       AO_CHAN_RIGHT |\
                                       AO_CHAN_REAR_SURROUND_LEFT |\
                                       AO_CHAN_REAR_SURROUND_RIGHT |\
                                       AO_CHAN_LFE)

#define AO_CHANMAP_4_1_SURROUND_DOLBY (AO_CHAN_LEFT |\
                                       AO_CHAN_RIGHT |\
                                       AO_CHAN_CENTER |\
                                       AO_CHAN_REAR_SURROUND_CENTER |\
                                       AO_CHAN_LFE)

#define AO_CHANMAP_5_0_SURROUND       (AO_CHAN_LEFT |\
                                       AO_CHAN_CENTER |\
                                       AO_CHAN_RIGHT |\
                                       AO_CHAN_REAR_SURROUND_LEFT |\
                                       AO_CHAN_REAR_SURROUND_RIGHT)

#define AO_CHANMAP_5_1_SURROUND       (AO_CHAN_LEFT |\
                                       AO_CHAN_CENTER |\
                                       AO_CHAN_RIGHT |\
                                       AO_CHAN_REAR_SURROUND_LEFT |\
                                       AO_CHAN_REAR_SURROUND_RIGHT |\
                                       AO_CHAN_LFE)

#define AO_CHANMAP_6_0_SURROUND_HEX   (AO_CHAN_LEFT |\
                                       AO_CHAN_RIGHT |\
                                       AO_CHAN_SIDE_SURROUND_LEFT |\
                                       AO_CHAN_SIDE_SURROUND_RIGHT |\
                                       AO_CHAN_REAR_SURROUND_LEFT |\
                                       AO_CHAN_REAR_SURROUND_RIGHT)

#define AO_CHANMAP_6_1_SURROUND       (AO_CHAN_LEFT |\
                                       AO_CHAN_RIGHT |\
                                       AO_CHAN_CENTER |\
                                       AO_CHAN_SIDE_SURROUND_LEFT |\
                                       AO_CHAN_SIDE_SURROUND_RIGHT |\
                                       AO_CHAN_REAR_SURROUND_CENTER |\
                                       AO_CHAN_LFE)

#define AO_CHANMAP_7_1_SURROUND       (AO_CHAN_LEFT |\
                                       AO_CHAN_CENTER |\
                                       AO_CHAN_RIGHT |\
                                       AO_CHAN_REAR_SURROUND_LEFT |\
                                       AO_CHAN_REAR_SURROUND_RIGHT |\
                                       AO_CHAN_SIDE_SURROUND_LEFT |\
                                       AO_CHAN_SIDE_SURROUND_RIGHT |\
                                       AO_CHAN_LFE)

#define AO_CHANMAP_7_1_WIDEDTS        (AO_CHAN_LEFT |\
                                       AO_CHAN_CENTER |\
                                       AO_CHAN_RIGHT |\
                                       AO_CHAN_LEFT_OF_CENTER |\
                                       AO_CHAN_RIGHT_OF_CENTER |\
                                       AO_CHAN_REAR_SURROUND_LEFT |\
                                       AO_CHAN_REAR_SURROUND_RIGHT |\
                                       AO_CHAN_LFE)

/* --- Structures --- */

typedef struct ao_info {
	int  type; /* live output or file output? */
	char *name; /* full name of driver */
	char *short_name; /* short name of driver */
        char *author; /* driver author */
	char *comment; /* driver comment */
	int  preferred_byte_format;
	int  priority;
	char **options;
	int  option_count;
} ao_info;

typedef struct ao_functions ao_functions;
typedef struct ao_device ao_device;

typedef struct ao_sample_format {
	int bits; /* bits per sample */
	int rate; /* samples per second (in a single channel) */
	int channels; /* number of audio channels */
	int byte_format; /* Byte ordering in sample, see constants below */
} ao_sample_format;

typedef struct ao_option {
	char *key;
	char *value;
	struct ao_option *next;
} ao_option;

#if defined(AO_BUILDING_LIBAO)
#include "ao_private.h"
#endif

/* --- Functions --- */

/* library setup/teardown */
void ao_initialize(void);
void ao_shutdown(void);

/* device setup/playback/teardown */
int ao_append_option(ao_option **options, const char *key, 
		     const char *value);
void ao_free_options(ao_option *options);
ao_device* ao_open_live(int driver_id, ao_sample_format *format,
				ao_option *option);
ao_device* ao_open_file(int driver_id, const char *filename, int overwrite,
			ao_sample_format *format, ao_option *option);

int ao_play(ao_device *device, char *output_samples, uint_32 num_bytes);
int ao_close(ao_device *device);

/* driver information */
int ao_driver_id(const char *short_name);
int ao_default_driver_id(void);
ao_info *ao_driver_info(int driver_id);
ao_info **ao_driver_info_list(int *driver_count);
char *ao_file_extension(int driver_id);

/* miscellaneous */
int ao_is_big_endian(void);


#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif  /* __AO_H__ */
