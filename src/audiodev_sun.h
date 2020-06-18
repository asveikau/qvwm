/*
 * audiodev_sun.h
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

#ifndef AUDIODEV_SUN_H_
#define AUDIODEV_SUN_H_

#if defined(sun) || defined(__NetBSD__) || defined(__OpenBSD__)
#define SUN_AUDIO
#endif

#ifdef SUN_AUDIO

// SUNaudio header
#if defined(sun) && !defined(__SVR4)
#include <sun/audioio.h>
#endif

#if (defined(sun) && defined(__SVR4)) || defined(__NetBSD__) || defined(__OpenBSD__)
#include <sys/audioio.h>
#endif

#include "audiodev.h"

class AudiodevSun : public Audiodev {
private:
  char* m_audiodev;
  int m_fd;
  audio_info_t m_ainfo;
  int m_amask;

public:
  AudiodevSun(char* audiodev) : m_audiodev(audiodev) {}

  int open();
  int close();
  int prepare();
  int output(char* buf, int size);

  int setFormat(int bits, int encoding);
  int setChannels(int channels);
  int setSamplingRate(int rate);
};

#endif // SUN_AUDIO

#endif // AUDIODEV_SUN_H_
