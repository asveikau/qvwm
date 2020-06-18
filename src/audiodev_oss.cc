/*
 * audiodev_oss.cc
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

#include "audiodev_oss.h"

#ifdef OSS

#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include "audio.h"
#include "message.h"

int AudiodevOss::open()
{
  if ((m_fd = ::open(m_audiodev, O_WRONLY)) < 0) {
    QvwmError("OSS open device %s: %s", m_audiodev, strerror(errno));
    return AUDIO_FATAL;
  }

  return 0;
}

int AudiodevOss::close()
{
  if (::close(m_fd) < 0) {
    QvwmError("OSS close device %s: %s", m_audiodev, strerror(errno));
    return AUDIO_ERROR;
  }

  return 0;
}

int AudiodevOss::output(char* buf, int size)
{
  int err;

  if ((err = write(m_fd, buf, size)) < 0) {
    QvwmError("OSS write: %s", strerror(errno));
    return AUDIO_ERROR;
  }

  return 0;
}

int AudiodevOss::setFormat(int bits, int encoding)
{
  int format;

  switch (encoding) {
  case Audio::EN_ULAW:
    format = AFMT_MU_LAW;
    break;
    
  case Audio::EN_ALAW:
    format = AFMT_A_LAW;
    break;
    
  case Audio::EN_ULINEAR:
    if (bits == 8)
      format = AFMT_U8;
    else if (bits == 16) {
#ifdef SND_LITTLE_ENDIAN
      format = AFMT_U16_LE;
#else
      format = AFMT_U16_BE;
#endif
    }
    else {
      outputFormatError("OSS", bits, encoding);
      return AUDIO_ERROR;
    }
    break;

  case Audio::EN_SLINEAR:
    if (bits == 8)
      format = AFMT_S8;
    else if (bits == 16) {
#ifdef SND_LITTLE_ENDIAN
      format = AFMT_S16_LE;
#else
      format = AFMT_S16_BE;
#endif
    }
    else {
      outputFormatError("OSS", bits, encoding);
      return AUDIO_ERROR;
    }
    break;

  default:
    outputFormatError("OSS", bits, encoding);
    return AUDIO_ERROR;
  }

  if (ioctl(m_fd, SNDCTL_DSP_SETFMT, &format) < 0) {
    QvwmError("OSS: SNDCTL_DSP_SETFMT: %s", strerror(errno));
    return AUDIO_ERROR;
  }

  return 0;
}

int AudiodevOss::setChannels(int channels)
{
  if (ioctl(m_fd, SNDCTL_DSP_CHANNELS, &channels) < 0) {
    QvwmError("SNDCTL_DSP_CHANNELS: %s", strerror(errno));
    return AUDIO_ERROR;
  }

  return 0;
}

int AudiodevOss::setSamplingRate(int rate)
{
  if (ioctl(m_fd, SNDCTL_DSP_SPEED, &rate) < 0) {
    QvwmError("SNDCTL_DSP_SPEED: %s", strerror(errno));
    return AUDIO_ERROR;
  }

  return 0;
}

#endif // OSS
