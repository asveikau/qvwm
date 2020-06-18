/*
 * audiodev_esd.h
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

#ifndef AUDIODEV_ESD_H_
#define AUDIODEV_ESD_H_

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#ifdef USE_ESD
#define ESD
#endif

#ifdef ESD

#include <esd.h>
#include "audiodev.h"

class AudiodevEsd : public Audiodev {
private:
  char* m_host;			// ESD host
  int m_fd;			// socket for communicating with ESD
  esd_format_t m_format;
  int m_rate;

public:
  AudiodevEsd(char* host) : m_host(host) {}

  int open();
  int close();
  int prepare();
  int output(char* buf, int size);

  int setFormat(int bits, int encoding);
  int setChannels(int channels);
  int setSamplingRate(int rate);
};

#endif // ESD

#endif // AUDIODEV_ESD_H_
