/*
 *
 *  ao_arts.c
 *
 *      Copyright (C) Rik Hemsley (rikkus) <rik@kde.org 2000
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

#include <stdio.h>
#include <errno.h>

#include <artsc.h>
#include <ao/ao.h>

typedef struct ao_arts_internal_s
{
  arts_stream_t stream;
	uint_32 bits;
	uint_32 rate;
	uint_32 channels;
} ao_arts_internal_t;

ao_info_t ao_arts_info =
{
  "aRts output",
  "arts",
  "Rik Hemsley (rikkus) <rik@kde.org>",
  "Outputs to the aRts soundserver."
};

  ao_internal_t *
plugin_open
(
 uint_32 bits,
 uint_32 rate,
 uint_32 channels,
 ao_option_t * options
)
{
  ao_arts_internal_t * state;
  int errorcode;

  state = malloc(sizeof(ao_arts_internal_t));

  if (NULL == state)
  {
    fprintf(stderr, "libao: Can't initialise aRts driver. Out of memory.\n");
    return NULL;
  }

  errorcode = arts_init();

  if (0 != errorcode)
  {
    fprintf(stderr, "libao: Can't initialise aRts driver.\n");
    fprintf(stderr, "libao: Error: %s\n", arts_error_text(errorcode));
    free(state);
    return NULL;
  }

  state->stream = arts_play_stream(rate, bits, channels, "ao stream");

	state->bits = bits;
	state->rate = rate;
	state->channels = channels;

  return state;
}

  void
plugin_close(ao_internal_t * state)
{
  arts_close_stream(((ao_arts_internal_t *)state)->stream);
  arts_free();
}

  void
plugin_play
(
 ao_internal_t * state,
 void * buf,
 uint_32 count
)
{
  int bytes_written;

  bytes_written = arts_write(((ao_arts_internal_t *)state)->stream, buf, count);

  if (bytes_written != count)
  {
    fprintf(stderr, "libao: aRts driver would not write all data !\n");
  }
}

  int
plugin_get_latency(ao_internal_t * state)
{
  ao_arts_internal_t * s = (ao_arts_internal_t *)state;
	int ms = arts_stream_get(s->stream, ARTS_P_TOTAL_LATENCY);
	int sample_rate = (s->bits / 8) * s->rate * s->channels;
	return (sample_rate * ms) / 1000;
}

  ao_info_t *
plugin_get_driver_info(void)
{
  return &ao_arts_info;
}

