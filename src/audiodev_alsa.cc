/*
 * audiodev_alsa.cc
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

#include "audiodev_alsa.h"

#ifdef ALSA

#include <stdio.h>
#include <string.h>
#include "audio.h"
#include "message.h"

int AudiodevAlsa::open()
{
  int err;
    
  if ((err = snd_pcm_open(&m_handle, 0, 0, SND_PCM_OPEN_PLAYBACK)) < 0) {
    QvwmError("snd_pcm_open: %s\n", snd_strerror(err));
    return AUDIO_FATAL;
  }
    
  memset(&m_params, 0, sizeof(m_params));
    
  return 0;
}

int AudiodevAlsa::close()
{
  int err;

  if ((err = snd_pcm_close(m_handle)) < 0) {
    QvwmError("snd_pcm_close: %s", snd_strerror(err));
    return AUDIO_ERROR;
  }

  return 0;
}

int AudiodevAlsa::prepare()
{
  int err;

  snd_pcm_plugin_flush(m_handle, SND_PCM_CHANNEL_PLAYBACK);
  
  m_params.mode = SND_PCM_MODE_BLOCK;
  m_params.channel = SND_PCM_CHANNEL_PLAYBACK;
  m_params.start_mode = SND_PCM_START_FULL;
  m_params.stop_mode = SND_PCM_STOP_STOP;
  m_params.buf.block.frag_size = AUDIO_BSIZE;
  m_params.buf.block.frags_max = -1;
  m_params.buf.block.frags_min = 1;
  m_params.format.interleave = 1;
  
  if ((err = snd_pcm_plugin_params(m_handle, &m_params)) < 0) {
    QvwmError("snd_pcm_plugin_params: %s", snd_strerror(err));
    return AUDIO_ERROR;
  }

  if ((err = snd_pcm_plugin_prepare(m_handle, SND_PCM_CHANNEL_PLAYBACK)) < 0) {
    QvwmError("snd_pcm_plugin_prepare: %s", snd_strerror(err));
    return AUDIO_ERROR;
  }

  return 0;
}

// buf must have a space enough for AUDIO_BSIZE
int AudiodevAlsa::output(char* buf, int size)
{
  int err;

  if (size < AUDIO_BSIZE)
    memset(buf + size, m_silence, AUDIO_BSIZE - size);

  if ((err = snd_pcm_plugin_write(m_handle, buf, AUDIO_BSIZE)) < 0) {
    QvwmError("snd_pcm_plugin_write: %s", snd_strerror(err));
    return AUDIO_ERROR;
  }

  return 0;
}

int AudiodevAlsa::flush()
{
  snd_pcm_plugin_flush(m_handle, SND_PCM_CHANNEL_PLAYBACK);

  return 0;
}

int AudiodevAlsa::setFormat(int bits, int encoding)
{
  switch (encoding) {
  case Audio::EN_ULAW:
    m_params.format.format = SND_PCM_SFMT_MU_LAW;
    m_silence = 0x00;
    break;

  case Audio::EN_ALAW:
    m_params.format.format = SND_PCM_SFMT_A_LAW;
    m_silence = 0x00;
    break;

  case Audio::EN_ULINEAR:
    if (bits == 8)
      m_params.format.format = SND_PCM_SFMT_U8;
    else if (bits == 16)
      m_params.format.format = SND_PCM_SFMT_U16;
    else if (bits == 24)
      m_params.format.format = SND_PCM_SFMT_U24;
    else if (bits == 32)
      m_params.format.format = SND_PCM_SFMT_U32;
    else {
      outputFormatError("ALSA", bits, encoding);
      return AUDIO_ERROR;
    }
    m_silence = 0x80;
    break;

  case Audio::EN_SLINEAR:
    if (bits == 8)
      m_params.format.format = SND_PCM_SFMT_S8;
    else if (bits == 16)
      m_params.format.format = SND_PCM_SFMT_S16;
    else if (bits == 24)
      m_params.format.format = SND_PCM_SFMT_S24;
    else if (bits == 32) {
      m_params.format.format = SND_PCM_SFMT_S32;
    }
    else {
      outputFormatError("ALSA", bits, encoding);
      return AUDIO_ERROR;
    }
    m_silence = 0x00;
    break;

  default:
    outputFormatError("ALSA", bits, encoding);
    return AUDIO_ERROR;
  }

  return 0;
}

int AudiodevAlsa::setChannels(int channels)
{
  m_params.format.voices = channels;

  return 0;
}

int AudiodevAlsa::setSamplingRate(int rate)
{
  m_params.format.rate = rate;
  
  return 0;
}

#endif // ALSA
