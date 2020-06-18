/*
 * audio_wav.cc
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
#include "audio_wav.h"
#include "message.h"

//
// Wave format:
//   "RIFF", len(4)
//   "WAVE"
//   "fmt ", len(4)
//   wFormatTag(2)        WAVE_FORMAT_PCM(0x0001), etc.
//   wChannels(2)         1: mono, 2: stereo
//   dwSamplesPerSec(4)   sampling rate
//   dwAvgBytesPerSec(4)
//   wBlockAlign(2)       
//   wBitsPerSample(2)    sampling bits per sample
//   ...
//   "data", len(4)
//   ...
//
int AudioWav::Play(char* file)
{
  FILE* fp;
  char id[4], format[8];
  WavHdr hdr;
  long len;
  int encoding;
  int ret;

  if ((fp = fopen(file, "r")) == NULL) {
    if (errno == ENOENT)
      return AUDIO_NOFILE;
    else {
      QvwmError("wave file open error: %s", strerror(errno));
      return AUDIO_ERROR;
    }
  }

  // check a magic
  if (fread(id, 4, 1, fp) < 1) {
    QvwmError("wave file read error: %s", strerror(errno));
    fclose(fp);
    return AUDIO_ERROR;
  }
  if (memcmp(id, "RIFF", 4) != 0) {
    QvwmError("wave file magic string is not 'RIFF'");
    fclose(fp);
    return AUDIO_ERROR;
  }

  // read a size; ignored
  skipWord(fp);
  
  // check a format
  if (fread(format, 8, 1, fp) < 1) {
    QvwmError("wave file read error: %s", strerror(errno));
    fclose(fp);
    return AUDIO_ERROR;
  }
  if (memcmp(format, "WAVEfmt ", 8) != 0) {
    QvwmError("wave file format does not start with 'WAVEfmt '");
    fclose(fp);
    return AUDIO_ERROR;
  }

  // read a size
  if (getLWord(fp, &len) < 0) {
    QvwmError("wave file read error: %s", strerror(errno));
    fclose(fp);
    return AUDIO_ERROR;
  }
  else if (len < 16) {
    QvwmError("wave file size is too small: %d", len);
    fclose(fp);
    return AUDIO_ERROR;
  }

  // read a format type
  if (getLHalf(fp, &hdr.m_wFormatTag) < 0) {
    QvwmError("wave file read error: %s", strerror(errno));
    fclose(fp);
    return AUDIO_ERROR;
  }

  // read stereo or mono
  if (getLHalf(fp, &hdr.m_wChannels) < 0) {
    QvwmError("wave file read error: %s", strerror(errno));
    fclose(fp);
    return AUDIO_ERROR;
  }
  else if (hdr.m_wChannels != 1 && hdr.m_wChannels != 2) {
    QvwmError("wave file channel is illegal: %d", hdr.m_wChannels);
    fclose(fp);
    return AUDIO_ERROR;
  }

  // read a sampling rate
  if (getLWord(fp, &hdr.m_dwSamplesPerSec) < 0) {
    QvwmError("wave file sampling rate is illegal: %d", hdr.m_dwSamplesPerSec);
    fclose(fp);
    return AUDIO_ERROR;
  }
  
  // read average bytes per second; ignored
  if (skipWord(fp) < 0) {
    QvwmError("wave file seek error: %s", strerror(errno));
    fclose(fp);
    return AUDIO_ERROR;
  }

  // read block alignment; ignored
  if (skipHalf(fp) < 0) {
    QvwmError("wave file seek error: %s", strerror(errno));
    fclose(fp);
    return AUDIO_ERROR;
  }

  // read sampling bits per sample
  if (getLHalf(fp, &hdr.m_wBitsPerSample) < 0) {
    QvwmError("wave file read error: %s", strerror(errno));
    fclose(fp);
    return AUDIO_ERROR;
  }

  if (fseek(fp, len - 16, SEEK_CUR) < 0) {
    QvwmError("wave file seek error: %s", strerror(errno));
    fclose(fp);
    return AUDIO_ERROR;
  }

#ifdef DEBUG
  printf("format type: %d\n", hdr.m_wFormatTag);
  printf("stereo: %d\n", (hdr.m_wChannels == 2) ? 1 : 0);
  printf("sampling rate: %d\n", hdr.m_dwSamplesPerSec);
  printf("sampling bits: %d\n", hdr.m_wBitsPerSample);
#endif

  // search a starting point of data area
  while (1) {
    if (fread(id, 4, 1, fp) < 1) {
      QvwmError("wave file read error: %s", strerror(errno));
      fclose(fp);
      return AUDIO_ERROR;
    }
    if (getLWord(fp, &len) < 0) {
      QvwmError("wave file read error: %s", strerror(errno));
      fclose(fp);
      return AUDIO_ERROR;
    }

    if (memcmp(id, "data", 4) == 0)
      break;
    else {
      if (fseek(fp, len, SEEK_CUR) < 0) {
	QvwmError("wave file seek error: %s", strerror(errno));
	fclose(fp);
	return AUDIO_ERROR;
      }
    }
  }
  
#ifdef DEBUG
  printf("data len: %d\n", len);
#endif

  // open an audio device
  if (openDevice() < 0) {
    fclose(fp);
    return AUDIO_ERROR;
  }

  // set the data format
  if (hdr.m_wBitsPerSample == 8)
    encoding = EN_ULINEAR;
  else
    encoding = EN_SLINEAR;

  if (setFormat(hdr.m_wBitsPerSample, encoding) < 0) {
    fclose(fp);
    closeDevice();
    return AUDIO_ERROR;
  }

  // set the number of channels
  if (setChannels(hdr.m_wChannels) < 0) {
    fclose(fp);
    closeDevice();
    return AUDIO_ERROR;
  }

  // set a sampling rate
  if (setSamplingRate(hdr.m_dwSamplesPerSec) < 0) {
    fclose(fp);
    closeDevice();
    return AUDIO_ERROR;
  }
  
  ret = outputStream(fp, len);

  fclose(fp);
  closeDevice();
  
  return ret;
}
