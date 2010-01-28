/***
  This file is part of libao-pulse.

  libao-pulse is free software; you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published
  by the Free Software Foundation; either version 2 of the License,
  or (at your option) any later version.

  libao-pulse is distributed in the hope that it will be useful, but
  WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
  General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with libao-pulse; if not, write to the Free Software
  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307
  USA.

 ********************************************************************

 last mod: $Id$

 ********************************************************************/


#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <assert.h>
#include <string.h>
#include <signal.h>
#include <limits.h>

#include <pulse/simple.h>
#include <pulse/util.h>

#include <ao/ao.h>
#include <ao/plugin.h>

/* Unfortunately libao doesn't allow "const" for these structures... */
static char * ao_pulse_options[] = {
    "server",
    "sink",
    "verbose",
    "quiet",
    "matrix",
    "debug"
};

static ao_info ao_pulse_info = {
    AO_TYPE_LIVE,
    "PulseAudio Output",
    "pulse",
    PACKAGE_BUGREPORT,
    "Outputs to the PulseAudio Sound Server",
    AO_FMT_NATIVE,
    50,
    ao_pulse_options,
    6
};

typedef struct ao_pulse_internal {
    struct pa_simple *simple;
    char *server, *sink;
} ao_pulse_internal;

/* Yes, this is very ugly, but required nonetheless... */
static void disable_sigpipe(void) {
    struct sigaction sa;

    sigaction(SIGPIPE, NULL, &sa);
    if (sa.sa_handler != SIG_IGN) {
        memset(&sa, 0, sizeof(sa));
        sa.sa_handler = SIG_IGN;
        sa.sa_flags = SA_RESTART;
        sigaction(SIGPIPE, &sa, NULL);
    }
}

int ao_plugin_test(void) {
    char *p=NULL, t[256], t2[256];
    const char *fn;
    struct pa_simple *s;
    static const struct pa_sample_spec ss = {
        .format = PA_SAMPLE_S16LE,
        .rate = 44100,
        .channels = 2
    };
    size_t allocated = 128;

    disable_sigpipe();

    if (getenv("PULSE_SERVER") || getenv("PULSE_SINK"))
        return 1;

    while (1) {
      p = pa_xmalloc(allocated);

      if (!(fn = pa_get_binary_name(p, allocated))) {
        pa_xfree(p);
        break;
      }

      if (fn != p || strlen(p) < allocated - 1) {
        snprintf(t, sizeof(t), "libao[%s]", fn);
        snprintf(t2, sizeof(t2), "libao[%s] test", fn);
        break;
      }

      pa_xfree(p);
      allocated *= 2;
    }
    pa_xfree(p);
    p = NULL;

    if (!(s = pa_simple_new(NULL, fn ? t : "libao", PA_STREAM_PLAYBACK, NULL, fn ? t2 : "libao test", &ss, NULL, NULL, NULL)))
        return 0;

    pa_simple_free(s);
    return 1;
}

ao_info *ao_plugin_driver_info(void) {
    return &ao_pulse_info;
}

int ao_plugin_device_init(ao_device *device) {
    ao_pulse_internal *internal;
    assert(device);

    internal = (ao_pulse_internal *) malloc(sizeof(ao_pulse_internal));

    if (internal == NULL)
        return 0;

    internal->simple = NULL;
    internal->server = NULL;
    internal->sink = NULL;
    device->internal = internal;

    return 1;
}

int ao_plugin_set_option(ao_device *device, const char *key, const char *value) {
    ao_pulse_internal *internal;
    assert(device && device->internal && key && value);
    internal = (ao_pulse_internal *) device->internal;

    if (!strcmp(key, "server")) {
        free(internal->server);
        internal->server = strdup(value);
    } else if (!strcmp(key, "sink")) {
        free(internal->sink);
        internal->sink = strdup(value);
    } else
        return 0;

    return 1;
}

pa_channel_position_t default_map[]={
  PA_CHANNEL_POSITION_FRONT_LEFT,
  PA_CHANNEL_POSITION_FRONT_RIGHT,
  PA_CHANNEL_POSITION_REAR_LEFT,
  PA_CHANNEL_POSITION_REAR_RIGHT,
  PA_CHANNEL_POSITION_FRONT_CENTER,
  PA_CHANNEL_POSITION_LFE,
  PA_CHANNEL_POSITION_SIDE_LEFT,
  PA_CHANNEL_POSITION_SIDE_RIGHT,
  PA_CHANNEL_POSITION_AUX0,
  PA_CHANNEL_POSITION_AUX1,
  PA_CHANNEL_POSITION_AUX2,
  PA_CHANNEL_POSITION_AUX3,
  PA_CHANNEL_POSITION_AUX4,
  PA_CHANNEL_POSITION_AUX5,
  PA_CHANNEL_POSITION_AUX6,
  PA_CHANNEL_POSITION_AUX7,
  PA_CHANNEL_POSITION_AUX8,
  PA_CHANNEL_POSITION_AUX9,
  PA_CHANNEL_POSITION_AUX10,
  PA_CHANNEL_POSITION_AUX11,
  PA_CHANNEL_POSITION_AUX12,
  PA_CHANNEL_POSITION_AUX13,
  PA_CHANNEL_POSITION_AUX14,
  PA_CHANNEL_POSITION_AUX15,
  PA_CHANNEL_POSITION_AUX16,
  PA_CHANNEL_POSITION_AUX17,
  PA_CHANNEL_POSITION_AUX18,
  PA_CHANNEL_POSITION_AUX19,
  PA_CHANNEL_POSITION_AUX20,
  PA_CHANNEL_POSITION_AUX21,
  PA_CHANNEL_POSITION_AUX22,
  PA_CHANNEL_POSITION_AUX23,
  PA_CHANNEL_POSITION_AUX23};

typedef struct {
  char *from;
  pa_channel_position_t to;
} translate;

translate trans[]={
  {"M",PA_CHANNEL_POSITION_MONO},
  {"L",PA_CHANNEL_POSITION_FRONT_LEFT},
  {"R",PA_CHANNEL_POSITION_FRONT_RIGHT},
  {"C",PA_CHANNEL_POSITION_FRONT_CENTER},
  {"BL",PA_CHANNEL_POSITION_REAR_LEFT},
  {"BR",PA_CHANNEL_POSITION_REAR_RIGHT},
  {"BC",PA_CHANNEL_POSITION_REAR_CENTER},
  {"SL",PA_CHANNEL_POSITION_SIDE_LEFT},
  {"SR",PA_CHANNEL_POSITION_SIDE_RIGHT},
  {"LFE",PA_CHANNEL_POSITION_LFE},
  {"U",PA_CHANNEL_POSITION_INVALID},
  {"X",PA_CHANNEL_POSITION_INVALID},
  {"CL",PA_CHANNEL_POSITION_FRONT_LEFT_OF_CENTER},
  {"CR",PA_CHANNEL_POSITION_FRONT_RIGHT_OF_CENTER},
  {NULL,PA_CHANNEL_POSITION_INVALID}
};

int ao_plugin_open(ao_device *device, ao_sample_format *format) {
    char *p=NULL, t[256], t2[256];
    const char *fn = NULL;
    ao_pulse_internal *internal;
    struct pa_sample_spec ss;
    struct pa_channel_map map;
    int usemap=0;
    size_t allocated = 128;

    assert(device && device->internal && format);

    internal = (ao_pulse_internal *) device->internal;

    if (format->bits == 8)
        ss.format = PA_SAMPLE_U8;
    else if (format->bits == 16)
        ss.format = PA_SAMPLE_S16NE;
    else
        return 0;

    if (format->channels <= 0 || format->channels > PA_CHANNELS_MAX)
        return 0;

    ss.channels = format->channels;
    ss.rate = format->rate;

    disable_sigpipe();

    while (1) {
        p = pa_xmalloc(allocated);

        if (!(fn = pa_get_binary_name(p, allocated))) {
            pa_xfree(p);
            break;
        }

        if (fn != p || strlen(p) < allocated - 1) {
            fn = pa_path_get_filename(fn);
            snprintf(t, sizeof(t), "libao[%s]", fn);
            snprintf(t2, sizeof(t2), "libao[%s] playback stream", fn);
            break;
        }

        pa_xfree(p);
        allocated *= 2;
    }
    pa_xfree(p);
    p = NULL;

    if(!device->output_matrix){
      if(format->matrix){
        /* request a matrix similar/identical to the input format; let
           Pulse do the conversion work */
        int i;
        char *p=format->matrix,*h;
        char buffer[129]={0};
        usemap=1;
        pa_channel_map_init(&map);
        map.channels=format->channels;

        for(i=0;i<format->channels;i++){
          int m=0;
          h=p;
          while(*h && *h!=',')h++;
          while(trans[m].from){
            if(h-p && !strncmp(trans[m].from,p,h-p) &&
               strlen(trans[m].from)==h-p)
              break;
            m++;
          }
          if(i)strcat(buffer,",");
          if(trans[m].from){
            map.map[i] = trans[m].to;
            strcat(buffer,trans[m].from);
          }else{
            map.map[i] = PA_CHANNEL_POSITION_INVALID;
            strcat(buffer,"X");
          }

          p=h;
          if(*h)p++;
        }

        device->output_matrix = strdup(buffer);

      }else{
        if(format->channels <= 32){
          /* set up a channel mapping similar to ALSA */
          if(format->channels == 1 ){
            usemap=1;
            pa_channel_map_init(&map);
            map.channels=1;
            map.map[0] = PA_CHANNEL_POSITION_MONO;
            device->output_matrix=strdup("M");
          }else{
            int i;
            usemap=1;
            pa_channel_map_init(&map);
            map.channels=format->channels;
            for(i=0;i<format->channels;i++)
              map.map[i] = default_map[i];
            device->output_matrix=strdup("L,R,BL,BR,C,LFE,SL,SR,"
                                  "A1,A2,A3,A4,A5,A6,A7,A8,A9,A10,"
                                  "A11,A12,A13,A14,A15,A16,A17,A18,A19,A20,"
                                  "A21,A22,A23,A4");
          }
        }
      }
    }

    if (!(internal->simple = pa_simple_new(internal->server, fn ? t : "libao", PA_STREAM_PLAYBACK, internal->sink, fn ? t2 : "libao playback stream", &ss, &map, NULL, NULL)))
        return 0;

    device->driver_byte_format = AO_FMT_NATIVE;

    return 1;
}

int ao_plugin_play(ao_device *device, const char* output_samples, uint_32 num_bytes) {
    assert(device && device->internal);
    ao_pulse_internal *internal = (ao_pulse_internal *) device->internal;

    return pa_simple_write(internal->simple, output_samples, num_bytes, NULL) >= 0;
}


int ao_plugin_close(ao_device *device) {
    assert(device && device->internal);
    ao_pulse_internal *internal = (ao_pulse_internal *) device->internal;

    pa_simple_drain(internal->simple, NULL);
    pa_simple_free(internal->simple);
    internal->simple = NULL;

    return 1;
}

void ao_plugin_device_clear(ao_device *device) {
    assert(device && device->internal);
    ao_pulse_internal *internal = (ao_pulse_internal *) device->internal;

    free(internal->server);
    free(internal->sink);
    free(internal);
    device->internal = NULL;
}
