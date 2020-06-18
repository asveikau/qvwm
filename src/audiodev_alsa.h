/*
 * audiodev_alsa.h
 *
 * Copyright (C) 1995-2001 Kenichi Kourai
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2, or (at your option)
 * any later version.
 * 
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with qvwm; see the file COPYING.  If not, write to
 * the Free Software Foundation, 675 Mass Ave, Cambridge, MA 02139, USA.
 */

#ifndef AUDIODEV_ALSA_H_
#define AUDIODEV_ALSA_H_

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#if defined(USE_ALSA) && defined(__linux__)
#define ALSA
#endif

#ifdef ALSA

extern "C" {
#include <sys/asoundlib.h>
}
#include "audiodev.h"

class AudiodevAlsa : public Audiodev {
private:
  snd_pcm_t *m_handle;
  snd_pcm_channel_params_t m_params;
  char m_silence;
  
public:
  AudiodevAlsa() {}

  int open();
  int close();
  int prepare();
  int output(char* buf, int size);
  int flush();

  int setFormat(int bits, int encoding);
  int setChannels(int channels);
  int setSamplingRate(int rate);
};

#endif // ALSA

#endif // AUDIODEV_ALSA_H_
