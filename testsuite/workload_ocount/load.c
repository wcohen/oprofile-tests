/*  load.c
 *   Copyright (C) 2013 IBM
 *
 *   Author: Carl Love  <carll@us.ibm.com>
 *
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

#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/time.h>
#include <sched.h>
#include <link.h>

#define ARRAY_SIZE      7000
#define ARRAY_STEP      128/sizeof(int)

#define REPORT_FREQ     100
#define NUM_ITER        20000000
#define UPPER_LIMIT     10000
#define LOWER_LIMIT     10

int main(int argc, char *argv[])
{

  int array[ARRAY_SIZE];
  int i, j;
  int sum=0;
  int direction=1;

  int sum_add(int sum, int value);
  int sum_sub(int sum, int value);

  for (i=0; i<ARRAY_SIZE; i++) {
    array[i] = i;
  }

  for (i=1; i<=NUM_ITER; i++) {
    for (j=0; j<ARRAY_SIZE; j=j+ARRAY_STEP) {

      if (sum > UPPER_LIMIT) {
        direction = 0;
      }

      if (sum < LOWER_LIMIT) {
        direction = 1;
      }

      if (direction == 1) {
        sum=sum_add(sum, array[j]);
      }

      if (direction == 0) {
        sum=sum_sub(sum, array[j]);
      }
    }
  }

  return (EXIT_SUCCESS);
}

int sum_add(int sum, int value)
{
  return(sum+value);
}

int sum_sub(int sum, int value)
{
  return(sum-value);
}
