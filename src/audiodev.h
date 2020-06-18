/*
 * audiodev.h
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

#ifndef AUDIODEV_H_
#define AUDIODEV_H_

class Audiodev {
public:
  Audiodev() {}
  virtual ~Audiodev();

  virtual int open() = 0;
  virtual int close() = 0;
  virtual int prepare();
  virtual int output(char* buf, int size) = 0;
  virtual int flush();

  virtual int setFormat(int bits, int encoding) = 0;
  virtual int setChannels(int channels) = 0;
  virtual int setSamplingRate(int rate) = 0;

  void outputFormatError(char* dev, int bits, int encoding);
};

#endif // AUDIODEV_H_
