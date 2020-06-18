/*
 * remote_cmd.cc
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

#include <stdio.h>
#include "audiodev.h"
#include "audio.h"
#include "message.h"

Audiodev::~Audiodev()
{
}

int Audiodev::prepare()
{
  return 0;
}

int Audiodev::flush()
{
  return 0;
}

void Audiodev::outputFormatError(char* dev, int bits, int encoding)
{
  char *strSign = "";
  char *strEncode = "unknown";

  if (encoding == Audio::EN_ULINEAR)
    strSign = "unsigned";
  else if (encoding == Audio::EN_SLINEAR)
    strSign = "signed";

  if (encoding == Audio::EN_ULAW)
    strEncode = "u-law";
  else if (encoding == Audio::EN_ALAW)
    strEncode = "A-law";
  else if (encoding == Audio::EN_ULINEAR || encoding == Audio::EN_SLINEAR)
    strEncode = "linear";

  QvwmError("%s: unsupported encoding: %s %d-bit %s",
	    dev, strSign, bits, strEncode);
}
