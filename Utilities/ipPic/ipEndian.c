/*****************************************************************************

 Copyright (c) 1993-2000,  Div. Medical and Biological Informatics, 
 Deutsches Krebsforschungszentrum, Heidelberg, Germany
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
          
     "This product includes software developed by the Div. Medical and 
      Biological Informatics, Deutsches Krebsforschungszentrum, Heidelberg, 
      Germany."

 - Neither the name of the Deutsches Krebsforschungszentrum nor the names of 
   its contributors may be used to endorse or promote products derived from 
   this software without specific prior written permission. 

   THIS SOFTWARE IS PROVIDED BY THE DIVISION MEDICAL AND BIOLOGICAL
   INFORMATICS AND CONTRIBUTORS ``AS IS'' AND ANY EXPRESS OR IMPLIED
   WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
   OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
   IN NO EVENT SHALL THE DIVISION MEDICAL AND BIOLOGICAL INFORMATICS,
   THE DEUTSCHES KREBSFORSCHUNGSZENTRUM OR CONTRIBUTORS BE LIABLE FOR 
   ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL 
   DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE 
   GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS 
   INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER 
   IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR 
   OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN 
   IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE. 

 Send comments and/or bug reports to:
   mbi-software@dkfz-heidelberg.de

*****************************************************************************/


/*
 * $RCSfile$
 *---------------------------------------------------------------
 * DESCRIPTION
 *   Routines for the conversion of Data
 *   between different machine representations
 *
 * $Log$
 * Revision 1.9  2002/11/13 17:52:59  ivo
 * new ipPic added.
 *
 * Revision 1.7  2000/05/04 12:52:34  ivo
 * inserted BSD style license
 *
 * Revision 1.6  2000/05/04 12:35:57  ivo
 * some doxygen comments.
 *
 * Revision 1.5  1999/11/27  23:59:45  andre
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
 *  COPYRIGHT (c) 1993 by DKFZ (Dept. MBI) Heidelberg, FRG
 */
#ifndef lint
  static char *what = { "@(#)ipEndian\t\tDKFZ (Dept. MBI)\t"__DATE__"\t$Revision$" };
#endif

#include <stdio.h>

#include "ipPic.h"

_ipEndian_t _ipEndian( void )
{
  static union
  { ipUInt2_t s;
    ipUInt1_t b;
  } t = { 0x1234 };

  if( t.b == 0x12 )
    return( _ipEndianBig );
  if( t.b == 0x34 )
    return( _ipEndianLittle );
  else
    {
      fprintf( stderr, "%s:_ipEndian: WARNING: unknown endian !!!\n", __FILE__ );
      return( _ipEndianUnknown );
    }
}

void _ipCp( void *source, void *destination, unsigned long int len )
{    
  if( !source )
    return; 
  if( !destination )
    return; 

  memcpy( destination, source, len );
}

void _ipCvtEndian( void *data, unsigned long int len, unsigned char bytes )
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

void _ipCpCvtEndian( void *source, void *destination, unsigned long int len, unsigned char bytes )
{
  unsigned long int i;
  unsigned char i2;

  if( !source )
    return;
  if( !destination )
    return;

  for( i=0; i<len; i+=bytes )
    for( i2=0; i2<bytes; i2++ )
      {
        ((char *)destination)[i+i2] = ((char *)source)[i+bytes-i2-1];
      }
}
