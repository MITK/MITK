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
 *   reads a slice from a PicFile
 *
 * $Log$
 * Revision 1.12  2006/03/03 14:07:58  maleike
 * try to fix warning "cast from pointer to integer of different size"
 *
 * Revision 1.11  2005/10/09 08:56:31  ivo
 * FIX: warning: comparison between pointer and integer
 *
 * Revision 1.10  2003/02/25 16:16:53  andre
 * *** empty log message ***
 *
 * Revision 1.9  2002/11/13 17:53:00  ivo
 * new ipPic added.
 *
 * Revision 1.7  2002/02/27 09:05:15  andre
 * zlib changes
 *
 * Revision 1.6  2002/02/27 09:02:56  andre
 * zlib changes
 *
 * Revision 1.5  2002/02/27 08:54:43  andre
 * zlib changes
 *
 * Revision 1.4  2000/05/04 12:52:38  ivo
 * inserted BSD style license
 *
 * Revision 1.3  2000/05/04 12:35:59  ivo
 * some doxygen comments.
 *
 * Revision 1.2  1997/09/15  13:21:14  andre
 * switched to new what string format
 *
 * Revision 1.1.1.1  1997/09/06  19:09:59  andre
 * initial import
 *
 * Revision 0.1  1993/04/02  16:18:39  andre
 * now works on PC, SUN and DECstation
 *
 * Revision 0.0  1993/03/31  11:26:16  andre
 * Initial revision, NO error checking
 *
 *
 *--------------------------------------------------------------------
 *  COPYRIGHT (c) 1993 by DKFZ (Dept. MBI) Heidelberg, FRG
 */
#ifndef lint
  static char *what = { "@(#)mitkIpPicGetSlice\t\tDKFZ (Dept. MBI)\t"__DATE__"\t$Revision$" };
#endif

#include "mitkIpPic.h"

mitkIpPicDescriptor *mitkIpPicGetSlice( const char *infile_name, mitkIpPicDescriptor *pic, ipUInt4_t slice )
{
  mitkIpPicFile_t infile;

  mitkIpPicTag_t tag_name;
  ipUInt4_t len;
  ipUInt4_t skip;

  int number = 1;

  unsigned long int picsize;

  if( infile_name == NULL )
    infile = stdin;
  else if( strcmp(infile_name, "stdin") == 0 )
    infile = stdin;
  else
    infile = _mitkIpPicOpenPicFileIn( infile_name );

  if( infile == NULL )
    {
      /*ipPrintErr( "mitkIpPicGetSlice: sorry, error opening infile\n" );*/
      return( NULL );
    }

  /* read infile */
  mitkIpPicFRead( &(tag_name[0]), 1, 4, infile );

  if( strncmp( mitkIpPicVERSION, tag_name, 4 ) != 0 )
    {
      if( pic == NULL )
        pic = _mitkIpPicOldGetSlice( infile,
                                 NULL,
                                 slice );
      else
        _mitkIpPicOldGetSlice( infile,
                           pic,
                           slice );
      if( infile != stdin )
        mitkIpPicFClose( infile );
      return( pic );
    }


  if( (void*)pic == (void*)3 )
    {
      pic = NULL;
      number = 3;
    }

  if( pic == NULL )
    pic = mitkIpPicNew();

  mitkIpPicClear( pic );

  mitkIpPicFRead( &(tag_name[4]), 1, sizeof(mitkIpPicTag_t)-4, infile );
  strncpy( pic->info->version, tag_name, _mitkIpPicTAGLEN );

  mitkIpPicFReadLE( &len, sizeof(ipUInt4_t), 1, infile );

  mitkIpPicFReadLE( &(pic->type), sizeof(ipUInt4_t), 1, infile );
  mitkIpPicFReadLE( &(pic->bpe), sizeof(ipUInt4_t), 1, infile );
  mitkIpPicFReadLE( &(pic->dim), sizeof(ipUInt4_t), 1, infile );

  mitkIpPicFReadLE( &(pic->n), sizeof(ipUInt4_t), pic->dim, infile );

  skip = len -        3 * sizeof(ipUInt4_t)
             - pic->dim * sizeof(ipUInt4_t);
  mitkIpPicFSeek( infile, skip, SEEK_CUR );

  picsize = _mitkIpPicSize(pic);

  pic->dim = 2;

  if( slice < 1
      || slice > picsize / _mitkIpPicSize(pic) )
    {
      mitkIpPicClear( pic );
      return( pic );
    }

  if( number < 1 )
    number = 1;

  if( slice + number - 1 > pic->n[2] )
    number = pic->n[2] - slice + 1;


  pic->info->write_protect = ipTrue;

  picsize = _mitkIpPicSize(pic);

  mitkIpPicFSeek( infile, picsize * (slice - 1), SEEK_CUR );

  if( number > 1 )
    {
      pic->dim = 3;
      pic->n[2] = number;
    }

  picsize = _mitkIpPicSize(pic);

  pic->data = malloc( picsize );

  mitkIpPicFReadLE( pic->data, pic->bpe / 8, _mitkIpPicElements(pic), infile );

  if( infile != stdin )
    mitkIpPicFClose( infile );

  return( pic );
}
