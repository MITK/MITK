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
 *--------------------------------------------------------------------
 * DESCRIPTION
 *   writes big- and litle- endian to disk
 *   and swaps to the other endianess
 *
 * $Log$
 * Revision 1.8  2003/02/18 12:28:23  andre
 * write compressed pic files
 *
 * Revision 1.7  2002/11/13 17:52:59  ivo
 * new ipPic added.
 *
 * Revision 1.5  2000/05/04 12:52:35  ivo
 * inserted BSD style license
 *
 * Revision 1.4  2000/05/04 12:35:57  ivo
 * some doxygen comments.
 *
 * Revision 1.3  1997/09/15  13:22:32  andre
 * *** empty log message ***
 *
 * Revision 1.2  1997/09/15  13:21:09  andre
 * switched to new what string format
 *
 * Revision 1.1.1.1  1997/09/06  19:09:58  andre
 * initial import
 *
 * Revision 0.1  1993/04/02  16:18:39  andre
 * now works on PC, SUN and DECstation
 *
 * Revision 0.0  1993/03/26  12:56:26  andre
 * Initial revision, NO error checking
 *
 *
 *--------------------------------------------------------------------
 *  COPYRIGHT (c) 1993 by DKFZ (Dept. MBI) Heidelberg, FRG
 */
#ifndef lint
  static char *what = { "@(#)_mitkIpFWrite\t\tDKFZ (Dept. MBI)\t"__DATE__"\t$Revision$" };
#endif

#include <stdio.h>
#include <stdlib.h>

#include "mitkIpPic.h"

#ifdef USE_ZLIB
mitkIpBool_t _mitkIpPicCanWriteCompressed = mitkIpTrue;
#else
mitkIpBool_t _mitkIpPicCanWriteCompressed = mitkIpFalse;
#endif


static mitkIpBool_t mitk_write_compressed = mitkIpFalse;

mitkIpBool_t
mitkIpPicGetWriteCompression( void )
{
  return( mitk_write_compressed );
}

mitkIpBool_t
mitkIpPicSetWriteCompression( mitkIpBool_t compression )
{
  mitkIpBool_t compression_old = mitk_write_compressed;

  mitk_write_compressed = compression;

  return( compression_old );
}

size_t
mitkIpFWriteCvt( void *ptr, size_t size, size_t nitems, FILE *stream )
{
  size_t bytes_return;

  void *buff;
  buff = malloc( size * nitems );
  _mitkIpCpCvtEndian( ptr, buff, size*nitems, size );
  bytes_return = fwrite( buff, size, nitems, stream);
  free( buff );

  return( bytes_return );
}

size_t
_mitkIpPicFWrite( const void *ptr, size_t size, size_t nitems, mitkIpPicFile_t stream)
{
  size_t bytes_return;

#ifdef USE_ZLIB
  if( mitk_write_compressed )
    bytes_return = gzwrite( stream, (void *)ptr, size*nitems);
  else
#endif
    bytes_return = fwrite( ptr, size, nitems, (FILE *)stream );

  return( bytes_return );
}

size_t
mitkIpPicFWriteCvt( void *ptr, size_t size, size_t nitems, mitkIpPicFile_t stream )
{
  size_t bytes_return;

  void *buff;
  buff = malloc( size * nitems );
  _mitkIpCpCvtEndian( ptr, buff, size*nitems, size );
  bytes_return = mitkIpPicFWrite( buff, size, nitems, stream);
  free( buff );

  return( bytes_return );
}
