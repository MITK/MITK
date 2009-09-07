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
 *   reads a tag from a PicFile
 *
 * $Log$
 * Revision 1.12  2002/11/13 17:53:00  ivo
 * new ipPic added.
 *
 * Revision 1.10  2000/05/04 12:52:38  ivo
 * inserted BSD style license
 *
 * Revision 1.9  2000/05/04 12:35:59  ivo
 * some doxygen comments.
 *
 * Revision 1.8  1999/11/28  18:21:37  andre
 * *** empty log message ***
 *
 * Revision 1.7  1999/11/28  16:27:21  andre
 * *** empty log message ***
 *
 * Revision 1.6  1999/11/28  00:36:09  andre
 * *** empty log message ***
 *
 * Revision 1.5  1999/11/27  19:32:13  andre
 * *** empty log message ***
 *
 * Revision 1.2.2.2  1998/03/25  15:03:36  andre
 * added info->pixel_start_in_file
 *
 * Revision 1.2.2.1  1997/09/15  13:47:06  andre
 * added encryption
 *
 * Revision 1.2  1997/09/15  13:21:14  andre
 * switched to new what string format
 *
 * Revision 1.1.1.1  1997/09/06  19:09:59  andre
 * initial import
 *
 *
 *--------------------------------------------------------------------
 *  COPYRIGHT (c) 1993 by DKFZ (Dept. MBI) Heidelberg, FRG
 */
#ifndef lint
  static char *what = { "@(#)mitkIpPicGetTag\t\tDKFZ (Dept. MBI)\t"__DATE__"\t$Revision$" };
#endif

#include "mitkIpPic.h"

mitkIpPicDescriptor *
mitkIpPicGetTags( const char *infile_name, mitkIpPicDescriptor *pic )
{
  mitkIpPicFile_t infile;

  mitkIpPicTag_t tag_name;
  ipUInt4_t dummy;
  ipUInt4_t len;
  ipUInt4_t dim;
  ipUInt4_t n[_mitkIpPicNDIM];

  ipUInt4_t to_read;

  infile = _mitkIpPicOpenPicFileIn( infile_name );

  if( infile == NULL )
    {
      /*ipPrintErr( "mitkIpPicGetTags: sorry, error opening infile\n" );*/
      return( NULL );
    }

  if( pic != NULL )
    pic->info->write_protect = ipFalse;

  /* read infile */
  mitkIpPicFRead( &(tag_name[0]), 1, 4, infile );

  if( strncmp( mitkIpPicVERSION, tag_name, 4 ) != 0 )
    {
      if( infile != stdin )
        mitkIpPicFClose( infile );
      return( pic );
    }

  if( pic == NULL )
    pic = mitkIpPicNew();

  mitkIpPicFRead( &(tag_name[4]), 1, sizeof(mitkIpPicTag_t)-4, infile );
  /*strncpy( pic->info->version, tag_name, _mitkIpPicTAGLEN );*/

  mitkIpPicFReadLE( &len, sizeof(ipUInt4_t), 1, infile );

  mitkIpPicFReadLE( &dummy, sizeof(ipUInt4_t), 1, infile );
  mitkIpPicFReadLE( &dummy, sizeof(ipUInt4_t), 1, infile );
  mitkIpPicFReadLE( &dim, sizeof(ipUInt4_t), 1, infile );

  mitkIpPicFReadLE( n, sizeof(ipUInt4_t), dim, infile );


  to_read = len -        3 * sizeof(ipUInt4_t)
                -      dim * sizeof(ipUInt4_t);

  pic->info->tags_head = _mitkIpPicReadTags( pic->info->tags_head, to_read, infile, mitkIpPicEncryptionType(pic) );

  pic->info->pixel_start_in_file = mitkIpPicFTell( infile );

  if( infile != stdin )
    mitkIpPicFClose( infile );

  return( pic );
}
