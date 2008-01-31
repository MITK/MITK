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
 *   reads big- and litle- endian from disk
 *   and swaps to the other endianess
 *
 * $Log$
 * Revision 1.14  2005/09/09 09:14:45  ivo
 * FIX: warnings on windows
 *
 * Revision 1.13  2003/03/06 10:53:08  andre
 * *** empty log message ***
 *
 * Revision 1.12  2002/11/13 17:52:59  ivo
 * new ipPic added.
 *
 * Revision 1.10  2002/02/27 08:54:43  andre
 * zlib changes
 *
 * Revision 1.9  2000/05/04 12:52:35  ivo
 * inserted BSD style license
 *
 * Revision 1.8  2000/01/17 18:31:59  andre
 * *** empty log message ***
 *
 * Revision 1.7  1999/12/09  19:12:09  andre
 * *** empty log message ***
 *
 * Revision 1.5  1999/11/29  09:34:34  andre
 * *** empty log message ***
 *
 * Revision 1.4  1999/11/28  16:27:20  andre
 * *** empty log message ***
 *
 * Revision 1.3  1999/11/27  19:15:08  andre
 * *** empty log message ***
 *
 * Revision 1.2  1997/09/15  13:21:08  andre
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
  static char *what = { "@(#)_ipFRead\t\tDKFZ (Dept. MBI)\t"__DATE__"\t$Revision$" };
#endif

#include <stdio.h>
#include <stdlib.h>
#ifdef WIN32
#include <io.h>
#endif

#include "ipPic.h"

size_t
ipFReadCvt( void *ptr, size_t size, size_t nitems, FILE *stream )
{
  size_t bytes_return;

  bytes_return = fread( ptr, size, nitems, stream );
  _ipCvtEndian( ptr, size*nitems, size );

  return( bytes_return );
}

#ifdef USE_ZLIB
ipBool_t _ipPicCanReadCompressed = ipTrue;
#else
ipBool_t _ipPicCanReadCompressed = ipFalse;
#endif

size_t
ipPicFReadCvt( void *ptr, size_t size, size_t nitems, ipPicFile_t stream )
{
  size_t bytes_return;

  bytes_return = ipPicFRead( ptr, size, nitems, stream );

  _ipCvtEndian( ptr, size*nitems, size );

  return( bytes_return );
}

int
ipPicAccess( const char *path, int mode )
{
  int status;

  status = access( path, mode );

  if( status == -1 )
    {
      char buff[1024];

      sprintf( buff, "%s.gz", path );

      status = access( buff, mode );
    }

  return( status );
}
int
ipPicRemoveFile( const char *path )
{
  int status;

  status = remove( path );

  if( status != 0 )
    {
      char buff[1024];

      sprintf( buff, "%s.gz", path );

      status = remove( buff );
    }

  return( status );
}
