/*
 *
 *  ao_private.c
 *
 *       Copyright (C) Stan Seibert - July 2001
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

#ifndef __AO_PRIVATE_H__
#define __AO_PRIVATE_H__

/* --- Operating System Compatibility --- */

/* 
  OpenBSD systems with a.out binaries require dlsym()ed symbols to be
  prepended with an underscore, so we need the following nasty #ifdef
  hack.
*/
#if defined(__OpenBSD__) && !defined(__ELF__)
#define dlsym(h,s) dlsym(h, "_" s)
#endif

/* RTLD_NOW is the preferred symbol resolution behavior, but
 * some platforms do not support it.  The autoconf script will have
 * already defined DLOPEN_FLAG if the default is unacceptable on the
 * current platform.
 *
 * ALSA requires RTLD_GLOBAL.
 */
#if !defined(DLOPEN_FLAG)
#define DLOPEN_FLAG (RTLD_NOW | RTLD_GLOBAL)
#endif

/* --- Constants --- */

#ifndef AO_SYSTEM_CONFIG
#define AO_SYSTEM_CONFIG "/etc/libao.conf"
#endif
#ifndef AO_USER_CONFIG
#define AO_USER_CONFIG   "/.libao"
#endif

/* --- Structures --- */

typedef struct ao_config {
	char *default_driver;
} ao_config;

/* --- Functions --- */

void read_config_files (ao_config *config);
int read_config_file(ao_config *config, const char *config_file);

#endif /* __AO_PRIVATE_H__ */
