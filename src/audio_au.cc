/*
 * audio_au.cc
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
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include "audio_au.h"
#include "message.h"

int AudioAu::Play(char* file)
{
  FILE* fp;
  AuHdr hdr;
  struct stat sb;
  int len, ret;
  int encoding, bits;

  if (stat(file, &sb) < 0) {
    if (errno == ENOENT)
      return AUDIO_NOFILE;
    else {
      QvwmError("au file check error: %s", strerror(errno));
      return AUDIO_ERROR;
    }
  }

  if ((fp = fopen(file, "r")) == NULL) {
    QvwmError("au file open error: %s", strerror(errno));
    return AUDIO_ERROR;
  }

  if (fread(hdr.m_magic, 4, 1, fp) < 1) {
    QvwmError("au file read error: %s", strerror(errno));
    fclose(fp);
    return AUDIO_ERROR;
  }
  if (memcmp(hdr.m_magic, ".snd", 4) != 0) {
    QvwmError("au format magic string is not '.snd'");
    fclose(fp);
    return AUDIO_ERROR;
  }

  if (getBWord(fp, &hdr.m_hdrsize) < 0) {
    QvwmError("au file read error: %s", strerror(errno));
    fclose(fp);
    return AUDIO_ERROR;
  }
  if (getBWord(fp, &hdr.m_datasize) < 0) {
    QvwmError("au file read error: %s", strerror(errno));
    fclose(fp);
    return AUDIO_ERROR;
  }
  if (getBWord(fp, &hdr.m_encoding) < 0) {
    QvwmError("au file read error: %s", strerror(errno));
    fclose(fp);
    return AUDIO_ERROR;
  }
  if (getBWord(fp, &hdr.m_rate) < 0) {
    QvwmError("au file read error: %s", strerror(errno));
    fclose(fp);
    return AUDIO_ERROR;
  }
  if (getBWord(fp, &hdr.m_channels) < 0) {
    QvwmError("au file read error: %s", strerror(errno));
    fclose(fp);
    return AUDIO_ERROR;
  }

  if (fseek(fp, hdr.m_hdrsize - 24, SEEK_CUR) < 0) {
    QvwmError("au file seek error: %s", strerror(errno));
    fclose(fp);
    return AUDIO_ERROR;
  }

  // open an audio device
  if (openDevice() < 0) {
    fclose(fp);
    return AUDIO_ERROR;
  }

  switch (hdr.m_encoding) {
  case AUDIO_FILE_ENCODING_MULAW_8:
    encoding = EN_ULAW;
    bits = 8;
    break;

  case AUDIO_FILE_ENCODING_ALAW_8:
    encoding = EN_ALAW;
    bits = 8;
    break;

  case AUDIO_FILE_ENCODING_LINEAR_8:
    encoding = EN_ULINEAR;
    bits = 8;
    break;

  case AUDIO_FILE_ENCODING_LINEAR_16:
    encoding = EN_ULINEAR;
    bits = 16;
    break;

  case AUDIO_FILE_ENCODING_LINEAR_24:
    encoding = EN_ULINEAR;
    bits = 24;
    break;

  case AUDIO_FILE_ENCODING_LINEAR_32:
    encoding = EN_ULINEAR;
    bits = 32;
    break;

  default:
    QvwmError("au file encoding is not supported");
    fclose(fp);
    closeDevice();
    return AUDIO_ERROR;
  }

  // set sampling bits per sample
  if (setFormat(bits, encoding) < 0) {
    fclose(fp);
    closeDevice();
    return AUDIO_ERROR;
  }

  // set the number of channels
  if (setChannels(hdr.m_channels) < 0) {
    fclose(fp);
    closeDevice();
    return AUDIO_ERROR;
  }

  // set a sampling rate
  if (setSamplingRate(hdr.m_rate) < 0) {
    fclose(fp);
    closeDevice();
    return AUDIO_ERROR;
  }
  
  len = sb.st_size - hdr.m_hdrsize;

  ret = outputStream(fp, len);

  fclose(fp);
  closeDevice();
  
  return ret;
}
