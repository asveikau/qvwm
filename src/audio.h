/*
 * audio.h
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

#ifndef AUDIO_H_
#define AUDIO_H_

#ifndef SEEK_CUR
#define SEEK_CUR 1
#endif

// transfer size at a time
#define AUDIO_BSIZE 4096

// return value
#define AUDIO_SUCCESS  0
#define AUDIO_ERROR   -1
#define AUDIO_NOFILE  -2
#define AUDIO_FATAL   -3

class Audiodev;

class Audio {
private:
  Audiodev* m_audioDev;

protected:
  int getLWord(FILE* fp, long *word);	// 4 bytes of little endian
  int getBWord(FILE* fp, long *word);	// 4 bytes of big endian
  int skipWord(FILE* fp);

  int getLHalf(FILE* fp, short *half);	// 2 bytes of little endian
  int getBHalf(FILE* fp, short *half);	// 2 bytes of big endian
  int skipHalf(FILE* fp);

  int outputStream(FILE* fp, int size);

public:
  enum {
    EN_ULAW,     // u-law
    EN_ALAW,     // A-law
    EN_ULINEAR,  // unsigned linear
    EN_SLINEAR   // signed linear
  };

public:
  Audio(char* audiodev);
  ~Audio();

  int openDevice();
  int closeDevice();

  int setFormat(int bits, int encoding);
  int setChannels(int channels);
  int setSamplingRate(int rate);

  virtual int Play(char* file);
};

#endif // AUDIO_H_
