/*
 * audio.cc
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

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif
#include <errno.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include "main.h"
#include "qvwmrc.h"
#include "audio.h"
#include "audiodev.h"
#include "audiodev_alsa.h"
#include "audiodev_esd.h"
#include "audiodev_oss.h"
#include "audiodev_sun.h"

Audio::Audio(char* audiodev)
{
#ifdef ALSA
  if (EnableAlsa) {
    m_audioDev = new AudiodevAlsa();
    return;
  }
#endif

#ifdef ESD
  if (EnableEsd) {
    m_audioDev = new AudiodevEsd(NULL);
    return;
  }
#endif

#ifdef OSS
  m_audioDev = new AudiodevOss(audiodev);
  return;
#endif

#ifdef SUN_AUDIO
  m_audioDev = new AudiodevSun(audiodev);
  return;
#endif

  QvwmError("qvwm does not support sound facility in your architecture");
  EnableSound = False;

  return;
}

Audio::~Audio()
{
}

int Audio::Play(char* file)
{
  // dummy play
  return 0;
}

// get a word (4 bytes) of little endian
int Audio::getLWord(FILE* fp, long *word)
{
  unsigned char data[4];

  if (fread(data, 4, 1, fp) < 1)
    return AUDIO_ERROR;
  
  *word = data[0] + ((int)data[1] << 8) + ((int)data[2] << 16)
    + ((int)data[3] << 24);
  return 0;
}

// get a word (4 bytes) of big endian
int Audio::getBWord(FILE* fp, long *word)
{
  unsigned char data[4];

  if (fread(data, 4, 1, fp) < 1)
    return AUDIO_ERROR;

  *word = data[3] + ((int)data[2] << 8) + ((int)data[1] << 16)
    + ((int)data[0] << 24);
  return 0;
}

// skip a word (4 bytes)
int Audio::skipWord(FILE* fp)
{
  if (fseek(fp, 4L, SEEK_CUR) < 0)
    return AUDIO_ERROR;

  return 0;
}

// get a half word (2 bytes) of little endian
int Audio::getLHalf(FILE* fp, short *half)
{
  unsigned char data[2];

  if (fread(data, 2, 1, fp) < 1)
    return AUDIO_ERROR;

  *half = data[0] + ((short)data[1] << 8);
  return 0;
}

// get a half word (2 bytes) of big endian
int Audio::getBHalf(FILE* fp, short *half)
{
  unsigned char data[2];

  if (fread(data, 2, 1, fp) < 1)
    return AUDIO_ERROR;

  *half = data[1] + ((short)data[0] << 8);
  return 0;
}

// skip a half word (2 bytes)
int Audio::skipHalf(FILE* fp)
{
  if (fseek(fp, 2L, SEEK_CUR) < 0)
    return AUDIO_ERROR;

  return 0;
}

// open an audio device
int Audio::openDevice()
{
  if (!EnableSound)
    return AUDIO_FATAL;

  return m_audioDev->open();
}

// close an audio device
int Audio::closeDevice()
{
  if (!EnableSound)
    return AUDIO_FATAL;

  return m_audioDev->close();
}

// set the data format
int Audio::setFormat(int bits, int encoding)
{
  if (!EnableSound)
    return AUDIO_FATAL;

  return m_audioDev->setFormat(bits, encoding);
}

// set the number of channels; mono = 1, stereo = 2
int Audio::setChannels(int channels)
{
  if (!EnableSound)
    return AUDIO_FATAL;

  return m_audioDev->setChannels(channels);
}

// set a sampling rate per second
int Audio::setSamplingRate(int rate)
{
  if (!EnableSound)
    return AUDIO_FATAL;

  return m_audioDev->setSamplingRate(rate);
}

// output audio stream from the file
int Audio::outputStream(FILE* fp, int size)
{
  int bytes, err;
  char buf[AUDIO_BSIZE];

  if (!EnableSound)
    return AUDIO_FATAL;

  if (m_audioDev->prepare())
    return AUDIO_ERROR;

  // output audio data
  while (size > 0) {
    if (size > AUDIO_BSIZE)
      bytes = AUDIO_BSIZE;
    else
      bytes = size;

    if (fread(buf, bytes, 1, fp) < 1) {
      QvwmError("sound file read error: %s", strerror(errno));
      return AUDIO_ERROR;
    }

    err = m_audioDev->output(buf, bytes);
    if (err == AUDIO_ERROR)
      return err;

    size -= bytes;
  }

  m_audioDev->flush();

  return 0;
}
