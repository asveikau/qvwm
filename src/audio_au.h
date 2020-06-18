/*
 * audio_au.h
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

#ifndef AUDIO_AU_H_
#define AUDIO_AU_H_

#include "audio.h"

#define AUDIO_FILE_ENCODING_MULAW_8	1	// 8-bit ISDN u-law
#define AUDIO_FILE_ENCODING_LINEAR_8	2	// 8-bit linear PCM
#define AUDIO_FILE_ENCODING_LINEAR_16	3	// 16-bit linear PCM
#define AUDIO_FILE_ENCODING_LINEAR_24	4	// 24-bit linear PCM
#define AUDIO_FILE_ENCODING_LINEAR_32	5	// 32-bit linear PCM
#define AUDIO_FILE_ENCODING_ALAW_8	27	// 8-bit ISDN A-law

class AudioAu : public Audio {
public:
  AudioAu(char* audiodev) : Audio(audiodev) {}

  int Play(char* file);
};

struct AuHdr {
  char m_magic[4];
  long m_hdrsize;
  long m_datasize;
  long m_encoding;
  long m_rate;
  long m_channels;
};

#endif // AUDIO_AU_H_
