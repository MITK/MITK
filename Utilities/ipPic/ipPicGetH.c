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
 *   reads the Header of a  PicFile from disk
 *
 * $Log$
 * Revision 1.10  2002/11/13 17:53:00  ivo
 * new ipPic added.
 *
 * Revision 1.8  2000/05/04 12:52:37  ivo
 * inserted BSD style license
 *
 * Revision 1.7  2000/05/04 12:35:59  ivo
 * some doxygen comments.
 *
 * Revision 1.6  1999/11/28  16:27:21  andre
 * *** empty log message ***
 *
 * Revision 1.5  1999/11/28  00:36:09  andre
 * *** empty log message ***
 *
 * Revision 1.4  1999/11/27  20:02:41  andre
 * *** empty log message ***
 *
 * Revision 1.3  1999/11/27  19:15:26  andre
 * *** empty log message ***
 *
 * Revision 1.2  1997/09/15  13:21:13  andre
 * switched to new what string format
 *
 * Revision 1.1.1.1  1997/09/06  19:09:59  andre
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
  static char *what = { "@(#)mitkIpPicGetHeader\t\tDKFZ (Dept. MBI)\t"__DATE__"\t$Revision$" };
#endif

#include "mitkIpPic.h"

mitkIpPicDescriptor *
mitkIpPicGetHeader( const char *infile_name, mitkIpPicDescriptor *pic )
{
  mitkIpPicFile_t  infile;

  mitkIpPicTag_t tag_name;
  mitkIpUInt4_t len;

  infile = _mitkIpPicOpenPicFileIn( infile_name );

  if( infile == NULL )
    {
      /*ipPrintErr( "mitkIpPicGetHeader: sorry, error opening infile\n" );*/
      return( NULL );
    }

  /* read infile */
  mitkIpPicFRead( &(tag_name[0]), 1, 4, infile );

  if( strncmp( "\037\213", tag_name, 2 ) == 0 )
    {
      fprintf( stderr, "mitkIpPicGetHeader: sorry, can't read compressed file\n" );
      return( NULL );
    }
  else if( strncmp( mitkIpPicVERSION, tag_name, 4 ) != 0 )
    {
      if( pic == NULL )
        pic = _mitkIpPicOldGetHeader( infile,
                                  NULL );
      else
        _mitkIpPicOldGetHeader( infile,
                            pic );
      if( infile != stdin )
        mitkIpPicFClose( infile );
      return( pic );
    }

  if( pic == NULL )
    pic = mitkIpPicNew();

  mitkIpPicClear( pic );

  pic->info->write_protect = mitkIpTrue;

  mitkIpPicFRead( &(tag_name[4]), 1, sizeof(mitkIpPicTag_t)-4, infile );
  strncpy( pic->info->version, tag_name, _mitkIpPicTAGLEN );

  mitkIpPicFReadLE( &len, sizeof(mitkIpUInt4_t), 1, infile );

  mitkIpPicFReadLE( &(pic->type), sizeof(mitkIpUInt4_t), 1, infile );
  mitkIpPicFReadLE( &(pic->bpe), sizeof(mitkIpUInt4_t), 1, infile );
  mitkIpPicFReadLE( &(pic->dim), sizeof(mitkIpUInt4_t), 1, infile );

  mitkIpPicFReadLE( &(pic->n), sizeof(mitkIpUInt4_t), pic->dim, infile );


  if( infile != stdin )
    mitkIpPicFClose( infile );

  return( pic );
}
