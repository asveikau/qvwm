/*
 * audiodev_esd.cc
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

#include "audiodev_esd.h"

#ifdef ESD

#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include "audio.h"
#include "message.h"

int AudiodevEsd::open()
{
  m_fd = -1;

  return 0;
}

int AudiodevEsd::close()
{
  if (esd_close(m_fd) < 0) {
    QvwmError("esd_close: %s", strerror(errno));
    return AUDIO_ERROR;
  }

  return 0;
}

int AudiodevEsd::prepare()
{
  m_fd = esd_play_stream_fallback(m_format | ESD_STREAM | ESD_PLAY,
				  m_rate, NULL, "qvwm");
  if (m_fd < 0) {
    QvwmError("esd_play_stream_fallback: %s", strerror(errno));
    return AUDIO_ERROR;
  }
  
  return 0;
}

int AudiodevEsd::output(char* buf, int size)
{
  int err;

  if ((err = write(m_fd, buf, size)) < 0) {
    QvwmError("ESD write: %s", strerror(errno));
    return AUDIO_ERROR;
  }

  return 0;
}

int AudiodevEsd::setFormat(int bits, int encoding)
{
  switch (encoding) {
  case Audio::EN_ULINEAR:
  case Audio::EN_SLINEAR:
    m_format &= ~ESD_MASK_BITS;
    if (bits == 8)
      m_format |= ESD_BITS8;
    else if (bits == 16)
      m_format |= ESD_BITS16;
    else {
      outputFormatError("ESD", bits, encoding);
      return AUDIO_ERROR;
    }
    break;

  default:
    outputFormatError("ESD", bits, encoding);
    return AUDIO_ERROR;
  }

  return 0;
}

int AudiodevEsd::setChannels(int channels)
{
  m_format &= ~ESD_MASK_CHAN;
  if (channels == 1)
    m_format |= ESD_MONO;
  else
    m_format |= ESD_STEREO;

  return 0;
}

int AudiodevEsd::setSamplingRate(int rate)
{
  m_rate = rate;

  return 0;
}

#endif // ESD
