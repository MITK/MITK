/*============================================================================

 Copyright (c) German Cancer Research Center (DKFZ)
 All rights reserved.

 Redistribution and use in source and binary forms, with or without
 modification, are permitted provided that the following conditions are met:

 - Redistributions of source code must retain the above copyright notice, this
   list of conditions and the following disclaimer.

 - Redistributions in binary form must reproduce the above copyright notice,
   this list of conditions and the following disclaimer in the documentation
   and/or other materials provided with the distribution.

 - All advertising materials mentioning features or use of this software must
   display the following acknowledgement:

     "This product includes software developed by the German Cancer Research
      Center (DKFZ)."

 - Neither the name of the German Cancer Research Center (DKFZ) nor the names
   of its contributors may be used to endorse or promote products derived from
   this software without specific prior written permission.

   THIS SOFTWARE IS PROVIDED BY THE GERMAN CANCER RESEARCH CENTER (DKFZ) AND
   CONTRIBUTORS ``AS IS'' AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING,
   BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS
   FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE GERMAN
   CANCER RESEARCH CENTER (DKFZ) OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT,
   INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
   (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
   SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
   CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
   LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
   OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH
   DAMAGE.

============================================================================*/

/*
 * $RCSfile$
 *---------------------------------------------------------------
 * DESCRIPTION
 *   Routines for the conversion of Data
 *   between different machine representations
 *
 * $Log$
 * Revision 1.10  2004/01/17 22:58:18  andre
 * big endian fixes
 *
 * Revision 1.6  2004/01/17 22:56:18  uid9277
 * big endian fixes
 *
 * Revision 1.5  1999/11/27 23:59:45  andre
 * *** empty log message ***
 *
 * Revision 1.4  1999/11/27  19:15:08  andre
 * *** empty log message ***
 *
 * Revision 1.3  1998/11/11  11:11:26  andre
 * *** empty log message ***
 *
 * Revision 1.2  1997/09/15  13:21:07  andre
 * switched to new what string format
 *
 * Revision 1.1.1.1  1997/09/06  19:09:58  andre
 * initial import
 *
 * Revision 0.0  1993/03/26  12:56:26  andre
 * Initial revision, NO error checking
 *
 *
 *---------------------------------------------------------------
 *
 */
#ifndef lint
  static char *what = { "@(#)ipEndian\t\tGerman Cancer Research Center (DKFZ)\t"__DATE__"\t$Revision$" };
#endif

#include <stdio.h>

#include "mitkIpPic.h"

_mitkIpEndian_t _mitkIpEndian( void )
{
  static union
  { mitkIpUInt2_t s;
    mitkIpUInt1_t b;
  } t = { 0x1234 };

  if( t.b == 0x12 )
    return( _mitkIpEndianBig );
  if( t.b == 0x34 )
    return( _mitkIpEndianLittle );
  else
    {
      fprintf( stderr, "%s:_mitkIpEndian: WARNING: unknown endian !!!\n", __FILE__ );
      return( _mitkIpEndianUnknown );
    }
}

void _mitkIpCp( void *source, void *destination, unsigned long int len )
{
  if( !source )
    return;
  if( !destination )
    return;

  memmove( destination, source, len );
}

void _mitkIpCvtEndian( void *data, unsigned long int len, unsigned char bytes )
{
  unsigned long int i;
  unsigned char i2;
  char buff;

  if( !data )
    return;

  for( i=0; i<len; i+=bytes )
    for( i2=0; i2<bytes/2; i2++ )
      {
        buff = ((char *)data)[i+i2];
        ((char *)data)[i+i2] = ((char *)data)[i+bytes-i2-1];
        ((char *)data)[i+bytes-i2-1] = buff;
      }
}

void _mitkIpCpCvtEndian( void *source, void *destination, unsigned long int len, unsigned char bytes )
{
  unsigned long int i;
  unsigned char i2;

  if( !source )
    return;
  if( !destination )
    return;

  if( bytes == 1 )
    memmove( destination, source, len );
  else
    for( i=0; i<len; i+=bytes )
      for( i2=0; i2<bytes; i2++ )
        {
          ((char *)destination)[i+i2] = ((char *)source)[i+bytes-i2-1];
        }
}
