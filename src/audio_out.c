/*
 *
 *  audio_out.c
 *
 *      Original Copyright (C) Aaron Holtzman - May 1999
 *      Modifications Copyright (C) Stan Seibert - July 2000
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
 */

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <dlfcn.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <dirent.h>
#include <ao/ao.h>

/* These should have been set by the Makefile */
#ifndef AO_DEFAULT
#define AO_DEFAULT AO_NULL
#endif
#ifndef AO_PLUGIN_PATH
#define AO_PLUGIN_PATH "/usr/local/lib/ao"
#endif
#ifndef SHARED_LIB_EXT
#define SHARED_LIB_EXT ".so"
#endif

/* --- Driver Table --- */

typedef struct driver_tree_s {
	ao_functions_t *functions;
	void *handle;
	struct driver_tree_s *next;
} driver_tree_t;

extern ao_functions_t ao_null;
extern ao_functions_t ao_wav;

driver_tree_t *driver_head = NULL;

driver_tree_t *_get_plugin(char *plugin_file)
{
	driver_tree_t *dt;
	void *handle;
	
	handle = dlopen(plugin_file, RTLD_NOW);
	if (handle) {
		dt = (driver_tree_t *)malloc(sizeof(driver_tree_t));
		if (!dt) return NULL;

		dt->handle = handle;
		
		dt->functions = (ao_functions_t *)malloc(sizeof(ao_functions_t));
		if (!(dt->functions)) {
			free(dt);
			return NULL;
		}

		dt->functions->get_driver_info = dlsym(dt->handle, "plugin_get_driver_info");
		if (dlerror()) { free(dt->functions); free(dt); return NULL; }
		dt->functions->open = dlsym(dt->handle, "plugin_open");
		if (dlerror()) { free(dt->functions); free(dt); return NULL; }
		dt->functions->play = dlsym(dt->handle, "plugin_play");
		if (dlerror()) { free(dt->functions); free(dt); return NULL; }
		dt->functions->close = dlsym(dt->handle, "plugin_close");
		if (dlerror()) { free(dt->functions); free(dt); return NULL; }
	} else {
		return NULL;
	}

	return dt;
}

void ao_initialize(void)
{
	driver_tree_t *dnull;
	driver_tree_t *dwav;
	driver_tree_t *plugin;
	driver_tree_t *driver;
	DIR *plugindir;
	struct dirent *plugin_dirent;
	char *ext;
	struct stat statbuf;
	void *plughand;
	char fullpath[FILENAME_MAX];

	if (driver_head == NULL) {
		/* insert the null and wav drivers into the tree */
		dnull = (driver_tree_t *)malloc(sizeof(driver_tree_t));
		dnull->functions = &ao_null;
		dnull->handle = NULL;
		dwav = (driver_tree_t *)malloc(sizeof(driver_tree_t));
		dwav->functions = &ao_wav;
		dwav->handle = NULL;
		
		dnull->next = dwav;
		dwav->next = NULL;

		driver_head = dnull;		
		driver = dwav;

		/* now insert any plugins we find */
		plugindir = opendir(AO_PLUGIN_PATH);
		if (plugindir != NULL) {
			while ((plugin_dirent = readdir(plugindir)) != NULL) {
				snprintf(fullpath, FILENAME_MAX, "%s/%s", AO_PLUGIN_PATH, plugin_dirent->d_name);
				if (!stat(fullpath, &statbuf) && S_ISREG(statbuf.st_mode) && (ext = strrchr(plugin_dirent->d_name, '.')) != NULL) {
					if (strcmp(ext, SHARED_LIB_EXT) == 0) {
						plugin = _get_plugin(fullpath);
						if (plugin) {
							driver->next = plugin;
							plugin->next = NULL;
							driver = driver->next;
						}
					}
				}
			}
			
			closedir(plugindir);
		}
	}
}

void ao_shutdown(void)
{
	driver_tree_t *driver = driver_head;
	driver_tree_t *next_driver;

	if (!driver_head) return;

	/* unload and free all the plugins */
	driver = driver->next->next;
	while (driver) {
		if (driver->functions) free(driver->functions);
		if (driver->handle) dlclose(driver->handle);
		next_driver = driver->next;
		free(driver);
		driver = next_driver;
	}

	/* free the standard drivers */
	if (driver_head->next) free(driver_head->next);
	if (driver_head->next) free(driver_head);
}

int ao_get_driver_id(const char *short_name)
{
	int i;
	driver_tree_t *driver = driver_head;

	if (short_name == NULL) {
		return AO_NULL;
	} else {
		i = 0;
		while (driver) {
			if (strcmp(short_name, driver->functions->get_driver_info()->short_name) == 0)
				return i;
			driver = driver->next;
			i++;
		}
		
		return -1; /* No driver by that name */
	}
}

driver_tree_t *_get_driver(int driver_id) {
	int i = 0;
	driver_tree_t *driver = driver_head;

	if (driver_id < 0) return NULL;

	while (driver && (i < driver_id)) {
		i++;
		driver = driver->next;
	}

	if (i == driver_id) 
		return driver;

	return NULL;
}

int _check_driver_id(int driver_id)
{
	int i = 0;
	driver_tree_t *driver = driver_head;

	if (driver_id < 0) return 0;

	while (driver && (i <= driver_id)) {
		driver = driver->next;
		i++;
	}
	
	if (i == (driver_id + 1))
		return 1;

	return 0;
}	

ao_info_t *ao_get_driver_info(int driver_id)
{
	driver_tree_t *driver;

	if (driver = _get_driver(driver_id))
		return driver->functions->get_driver_info();
	else
		return NULL;
}



/* -- Audio Functions --- */

ao_device_t* ao_open(int driver_id, uint_32 bits, uint_32 rate, uint_32 channels, ao_option_t *options)
{
	ao_functions_t *funcs;
	ao_internal_t *state;
	ao_device_t *device;
	driver_tree_t *driver = driver_head;

	if (driver = _get_driver(driver_id)) {
		funcs = driver->functions;
		state = funcs->open(bits, rate, channels, options);
		if (state != NULL) {
			device = malloc(sizeof(ao_device_t));
			device->funcs = funcs;
			device->state = state;
			return device;
		}
	}
	
	return NULL;
}	

void ao_play(ao_device_t *device, void* output_samples, uint_32 num_bytes)
{
	device->funcs->play(device->state, output_samples, num_bytes);
}


void ao_close(ao_device_t *device)
{
	device->funcs->close(device->state);
	free(device);
}



/* --- Option Functions --- */

ao_option_t* _parse_option(const char* op_str)
{
	char *copy;
	char *value_ptr;
	char *colon;
        ao_option_t *op = NULL;
	
        copy = strdup(op_str);
	
        colon = strchr(copy, ':');
        if (colon != NULL) {
                value_ptr = colon + 1;
                *colon = 0x00; // Null terminate the key part
                
                /* Allocate the option structure */
                op = malloc(sizeof(ao_option_t));
                if (op != NULL) {
                        op->key = strdup(copy);
                        op->value = strdup(value_ptr);
                        op->next = NULL;
                }
        }
	
        free(copy);
        return op;
}


int ao_append_option(ao_option_t **options, const char *op_str)
{
	ao_option_t *temp;

	temp = _parse_option(op_str);

	if (temp == NULL)
		return 0; //Bad option format

	if (*options != NULL) {
		while ((*options)->next != NULL)
			*options = (*options)->next;
		(*options)->next = temp;
	} else {
		*options = temp;
	}

	return 1;
}


void ao_free_options(ao_option_t *options)
{
	ao_option_t *rest;

	while (options != NULL) {
		rest = options->next;
		free(options->key);
		free(options->value);
		free(options);
		options = rest;
	}
}

/* Helper function lifted from Vorbis' lib/vorbisfile.c */
int ao_is_big_endian(void) 
{
	uint_16 pattern = 0xbabe;
	unsigned char *bytewise = (unsigned char *)&pattern;

	if (bytewise[0] == 0xba) return 1;
	return 0;
}
