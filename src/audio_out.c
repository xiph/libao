/*
 *
 *  audio_out.c
 *
 *      Original Copyright (C) Aaron Holtzman - May 1999
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
#include <stdio.h>
#include <string.h>
#include <assert.h>
#include <ao/ao.h>

/* --- Function Tables --- */

extern ao_functions_t ao_null;

/* Okay, so this is messy.  I'm open to ideas of how to clean this
   up.  - Stan */

#ifdef AO_COMPILE_OSS
extern ao_functions_t ao_oss;
#define AO_FUNC_OSS &ao_oss
#else
#define AO_FUNC_OSS NULL
#endif

#ifdef AO_COMPILE_IRIX
extern ao_functions_t ao_irix;
#define AO_FUNC_IRIX &ao_irix
#else
#define AO_FUNC_IRIX NULL
#endif

#ifdef AO_COMPILE_SOLARIS
extern ao_functions_t ao_solaris;
#define AO_FUNC_SOLARIS &ao_solaris
#else
#define AO_FUNC_SOLARIS NULL
#endif

#ifdef AO_COMPILE_WIN32
extern ao_functions_t ao_win32;
#define AO_FUNC_WIN32 &ao_win32
#else
#define AO_FUNC_WIN32 NULL
#endif

#ifdef AO_COMPILE_BEOS
extern ao_functions_t ao_beos;
#define AO_FUNC_BEOS &ao_beos
#else
#define AO_FUNC_BEOS NULL
#endif

#ifdef AO_COMPILE_ESD
extern ao_functions_t ao_esd;
#define AO_FUNC_ESD &ao_esd
#else
#define AO_FUNC_ESD NULL
#endif

#ifdef AO_COMPILE_ALSA
extern ao_functions_t ao_alsa;
#define AO_FUNC_ALSA &ao_alsa
#else
#define AO_FUNC_ALSA NULL
#endif

extern ao_functions_t ao_wav;



/* --- Driver Table --- */

ao_functions_t* ao_drivers[AO_DRIVERS] = 
{
	&ao_null,        /* 0: Null Device */
        AO_FUNC_OSS,     /* 1: Linux, *BSD */ 
	AO_FUNC_IRIX,    /* 2: IRIX */
	AO_FUNC_SOLARIS, /* 3: Solaris */
	AO_FUNC_WIN32,   /* 4: Win32 */
	AO_FUNC_BEOS,    /* 5: BeOS */
	AO_FUNC_ESD,     /* 6: EsounD */
	AO_FUNC_ALSA,    /* 7: ALSA */
	NULL,            /* 8: Unassigned */
	NULL,            /* 9: Unassigned */
	&ao_wav,         /* 10: .WAV output */
	NULL,            /* 11: RAW output */
};



/* --- Driver Functions --- */

/* This should have been set by the Makefile */
#ifndef AO_DEFAULT
#define AO_DEFAULT AO_NULL
#endif

int ao_get_driver_id (const char *short_name)
{
	int i;

	if (short_name == NULL) 
		return AO_DEFAULT;
	else 
	{
		i = 0;
		while (i < AO_DRIVERS)
		{
			/* Skip empty driver slots */
			if (ao_drivers[i] != NULL
			    && !strcmp(short_name, 
				       ao_drivers[i]->get_driver_info()->short_name))
				return i;

			i++;
		}
		
		return -1; /* No driver by that name */
	}
}


int ao_check_driver_id (int driver_id)
{
	return driver_id >= 0 && driver_id < AO_DRIVERS && 
		ao_drivers[driver_id] != NULL;	
}	

ao_info_t *ao_get_driver_info (int driver_id)
{
	if (ao_check_driver_id(driver_id))
		return ao_drivers[driver_id]->get_driver_info();
	else
		return NULL;
}



/* -- Audio Functions --- */

ao_device_t* ao_open (int driver_id, uint_32 bits, uint_32 rate, uint_32 channels, 
	     ao_option_t *options)
{
	ao_functions_t *funcs;
	ao_internal_t *state;
	ao_device_t *device;

	if (ao_check_driver_id(driver_id))
	{
		funcs = ao_drivers[driver_id];
		state = funcs->open(bits, rate, channels, options);
		if (state != NULL)
		{
			device = malloc(sizeof(ao_device_t));
			device->funcs = funcs;
			device->state = state;
			return device;
		}
	}
	
	return NULL;
}	

void ao_play (ao_device_t *device, void* output_samples, uint_32 num_bytes)
{
  device->funcs->play(device->state, output_samples,
		      num_bytes);
}


void ao_close (ao_device_t *device)
{
	device->funcs->close(device->state);
	free(device);
}



/* --- Option Functions --- */

ao_option_t* ao_parse_option (const char* op_str)
{
	char *copy;
	char *value_ptr;
	char *colon;
        ao_option_t *op = NULL;
	
        copy = strdup(op_str);
	
        colon = strchr(copy, ':');
        if (colon != NULL) 
        {
                value_ptr = colon + 1;
                *colon = 0x00; // Null terminate the key part
                
                // Allocate the option structure
                op = malloc(sizeof(ao_option_t));
                if (op != NULL)
                {
                        op->key = strdup(copy);
                        op->value = strdup(value_ptr);
                        op->next = NULL;
                }
        }
	
        free(copy);
        return op;
}


int ao_append_option (ao_option_t **options, const char *op_str)
{
	ao_option_t *temp;

	temp = ao_parse_option(op_str);

	if (temp == NULL)
		return 0; //Bad option format

	if (*options != NULL)
	{
		while ((*options)->next != NULL)
		{
			*options = (*options)->next;
		}
		(*options)->next = temp;
	}
	else
	{
		*options = temp;
	}

	return 1;
}


void ao_free_options (ao_option_t* options)
{
	ao_option_t *rest;

	while (options != NULL)
	{
		rest = options->next;
		free(options->key);
		free(options->value);
		free(options);
		options = rest;
	}
}

/* Helper function lifted from lib/vorbisfile.c */
int ao_is_big_endian() {
	uint_16 pattern = 0xbabe;
	unsigned char *bytewise = (unsigned char *)&pattern;
	if (bytewise[0] == 0xba) return 1;
	
	assert(bytewise[0] == 0xbe);
	return 0;
}
