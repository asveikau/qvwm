/*
 * audio_wav.h
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

#ifndef AUDIO_WAV_H_
#define AUDIO_WAV_H_

#include "audio.h"

//
// Class for wave file
//
class AudioWav : public Audio {
public:
  AudioWav(const char* audiodev) : Audio(audiodev) {}

  int Play(const char* file);
};

struct WavHdr {
  short m_wFormatTag;
  short m_wChannels;
  long m_dwSamplesPerSec;
  long m_dwAvgBytesPerSec;
  short m_wBlockAlign;
  short m_wBitsPerSample;
};

#endif // AUDIO_WAV_H_
