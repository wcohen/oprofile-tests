/* memcpyt.c
 *  Copyright (C) 2012 IBM

 * This file is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
 */

#include <string.h>

main()
{
  int j;
  long a4k, b4k;
  char a[8192], b[8192];
  int source_offset, dest_offset;
  char *source_address, *dest_address;
  int iterations, copy_length;

  iterations = 50000000;
  copy_length = 65;
  source_offset = 3;
  dest_offset = 7;

  a4k = (long)(a+4095) & 0xfff;
  b4k = (long)(b+4095) & 0xfff;
  a4k = (4095 - a4k) & 0xfff;
  b4k = (4095 - b4k) & 0xfff;

  source_address=a+a4k+source_offset;
  dest_address=b+b4k+dest_offset;

  for(j=0;j<iterations;j++)
      memcpy(dest_address, source_address, copy_length);
}
