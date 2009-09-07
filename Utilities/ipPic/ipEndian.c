
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
 *  COPYRIGHT (c) 1993 by DKFZ (Dept. MBI) Heidelberg, FRG
 */
#ifndef lint
  static char *what = { "@(#)ipEndian\t\tDKFZ (Dept. MBI)\t"__DATE__"\t$Revision$" };
#endif

#include <stdio.h>

#include "mitkIpPic.h"

_ipEndian_t _mitkIpEndian( void )
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
      fprintf( stderr, "%s:_mitkIpEndian: WARNING: unknown endian !!!\n", __FILE__ );
      return( _ipEndianUnknown );
    }
}

void _mitkIpCp( void *source, void *destination, unsigned long int len )
{    
  if( !source )
    return; 
  if( !destination )
    return; 

  memcpy( destination, source, len );
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
    memcpy( destination, source, len );
  else
    for( i=0; i<len; i+=bytes )
      for( i2=0; i2<bytes; i2++ )
        {
          ((char *)destination)[i+i2] = ((char *)source)[i+bytes-i2-1];
        }
}
