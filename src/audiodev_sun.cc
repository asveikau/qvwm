/*
 * audiodev_sun.cc
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

#include "audiodev_sun.h"

#ifdef SUN_AUDIO

#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include "audio.h"
#include "message.h"

int AudiodevSun::open()
{
  if ((m_fd = ::open(m_audiodev, O_WRONLY)) < 0) {
    QvwmError("SUNaudio open device %s: %s", m_audiodev, strerror(errno));
    return AUDIO_FATAL;
  }

  AUDIO_INITINFO(&m_ainfo);
  m_amask = 0;

  return 0;
}

int AudiodevSun::close()
{
  if (::close(m_fd) < 0) {
    QvwmError("SUNaudio close device %s: %s", m_audiodev, strerror(errno));
    return AUDIO_ERROR;
  }

  return 0;
}

int AudiodevSun::prepare()
{
  if (ioctl(m_fd, m_amask, &m_ainfo) < 0) {
    QvwmError("SUNaudio set audio_info: %s", strerror(errno));
    return AUDIO_ERROR;
  }

  return 0;
}

int AudiodevSun::output(char* buf, int size)
{
  int err;

  if ((err = write(m_fd, buf, size)) < 0) {
    QvwmError("SUNaudio write: %s", strerror(errno));
    return AUDIO_ERROR;
  }

  return 0;
}

int AudiodevSun::setFormat(int bits, int encoding)
{
  switch (encoding) {
  case Audio::EN_ULAW:
    m_ainfo.play.encoding = AUDIO_ENCODING_ULAW;
    break;

  case Audio::EN_ALAW:
    m_ainfo.play.encoding = AUDIO_ENCODING_ALAW;
    break;

  case Audio::EN_ULINEAR:
    if (bits == 8)
      m_ainfo.play.encoding = AUDIO_ENCODING_LINEAR8;
    else {
      outputFormatError("SUNaudio", bits, encoding);
      return AUDIO_ERROR;
    }
    break;

  case Audio::EN_SLINEAR:
    m_ainfo.play.encoding = AUDIO_ENCODING_LINEAR;
    break;

  default:
    outputFormatError("SUNaudio", bits, encoding);
    return AUDIO_ERROR;
  }

  m_ainfo.play.precision = bits;
  m_amask |= AUDIO_SETINFO;

  return 0;
}

int AudiodevSun::setChannels(int channels)
{
  m_ainfo.play.channels = channels;
  m_amask |= AUDIO_SETINFO;

  return 0;
}

int AudiodevSun::setSamplingRate(int rate)
{
  m_ainfo.play.sample_rate = rate;
  m_amask |= AUDIO_SETINFO;

  return 0;
}

#endif // SUN_AUDIO
