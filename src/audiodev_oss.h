/*
 * audiodev_oss.h
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

#ifndef AUDIODEV_OSS_H_
#define AUDIODEV_OSS_H_

#if defined(__linux__) || defined(__FreeBSD__)
#define OSS
#endif

#ifdef OSS

// OSS header
#ifdef __linux__
#include <linux/soundcard.h>
#endif

#ifdef __FreeBSD__
#include <machine/soundcard.h>
#endif

// set endian
#if !defined(SND_LITTLE_ENDIAN) && !defined(SND_BIG_ENDIAN)
#ifdef __linux__
#include <endian.h>

#if __BYTE_ORDER == __LITTLE_ENDIAN
#define SND_LITTLE_ENDIAN
#elif __BYTE_ORDER == __BIG_ENDIAN
#define SND_BIG_ENDIAN
#else
#error "Unsupported endian"
#endif
#endif // __linux__

#ifdef __FreeBSD__
#define SND_LITTLE_ENDIAN  // XXX
#endif // __FreeBSD__
#endif // !SND_LITTLE_ENDIAN && !SND_BIG_ENDIAN

// set extra definition
#ifndef SNDCTL_DSP_CHANNELS
#define SNDCTL_DSP_CHANNELS SOUND_PCM_WRITE_CHANNELS
#endif

#include "audiodev.h"

class AudiodevOss : public Audiodev {
private:
  char* m_audiodev;
  int m_fd;

public:
  AudiodevOss(char* audiodev) : m_audiodev(audiodev) {}

  int open();
  int close();
  int output(char* buf, int size);

  int setFormat(int bits, int encoding);
  int setChannels(int channels);
  int setSamplingRate(int rate);
};

#endif // OSS

#endif // AUDIODEV_OSS_H_
