/*
 *
 *  audio_out.h 
 *    
 *	Original Copyright (C) Aaron Holtzman - May 1999
 *      Modifications Copyright (C) Stan Seibert - July 2000
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

#include <stdlib.h>

#include "os_types.h"

// Type sizes
#include "config.h"

/* --- Structures --- */

typedef struct ao_option_s {
	char *key;
	char *value;
	struct ao_option_s *next;
} ao_option_t;

typedef struct ao_info_s
{
	/* driver name (Ex: "OSS Audio driver") */
	const char *name;
	/* short name (for config strings) (Ex: "oss") */
	const char *short_name;
	/* author (Ex: "Aaron Holtzman <aholtzma@ess.engr.uvic.ca>") */
	const char *author;
	/* any additional comments (Ex: "Needs work.") */
	const char *comment;
} ao_info_t;

typedef void ao_internal_t;

typedef struct ao_functions_s
{
	ao_info_t* (*get_driver_info) (void);
	ao_internal_t*     (*open)  (uint_32 bits, uint_32 rate, 
				     uint_32 channels, ao_option_t *options);
	void             (*play)  (ao_internal_t *state, 
				   void* output_samples, uint_32 num_bytes);
	void             (*close) (ao_internal_t *state);
} ao_functions_t;

typedef struct ao_device_s
{
	ao_functions_t *funcs;
	ao_internal_t *state;
} ao_device_t;



/* --- Driver id numbers --- */

#define AO_NULL     0

#define AO_OSS      1
#define AO_IRIX     2
#define AO_SOLARIS  3
#define AO_WIN32    4
#define AO_BEOS     5
#define AO_ESD      6
#define AO_ALSA     7

#define AO_WAV      10
#define AO_RAW      11

/* Total number of drivers */
#define AO_DRIVERS 12


/* --- Functions --- */

int ao_get_driver_id (const char *short_name);

ao_info_t *ao_get_driver_info (int driver_id);

ao_device_t *ao_open (int driver_id, uint_32 bits, uint_32 rate, uint_32 channels, 
	     ao_option_t *options);

void ao_play (ao_device_t *device, void* output_samples, uint_32 num_bytes);

void ao_close (ao_device_t *device);

/* Returns 1 if options successfully appended, 0 if error */
int ao_append_option (ao_option_t **options, const char* op_str);

void ao_free_options (ao_option_t* options);

int ao_is_big_endian();
