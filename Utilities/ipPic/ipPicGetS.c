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
  static char *what = { "@(#)ipPicGetSlice\t\tDKFZ (Dept. MBI)\t"__DATE__"\t$Revision$" };
#endif

#include "ipPic.h"

ipPicDescriptor *ipPicGetSlice( char *infile_name, ipPicDescriptor *pic, ipUInt4_t slice )
{
  FILE *infile;

  ipPicTag_t tag_name;
  ipUInt4_t len;
  ipUInt4_t skip;

  unsigned long int picsize;

  if( infile_name == NULL )
    infile = stdin;
  else if( strcmp(infile_name, "stdin") == 0 )
    infile = stdin;
  else
    infile = fopen( infile_name, "rb" );

  if( infile == NULL )
    {
      /*ipPrintErr( "ipPicGetSlice: sorry, error opening infile\n" );*/
      return( NULL );
    }

  /* read infile */
  fread( &(tag_name[0]), 1, 4, infile );

  if( strncmp( ipPicVERSION, tag_name, 4 ) != 0 )
    {
      if( pic == NULL )
        pic = _ipPicOldGetSlice( infile,
                                 NULL,
                                 slice );
      else
        _ipPicOldGetSlice( infile,
                           pic,
                           slice );
      if( infile != stdin )
        fclose( infile );
      return( pic );
    }


  if( pic == NULL )
    pic = ipPicNew();

  ipPicClear( pic );

  fread( &(tag_name[4]), 1, sizeof(ipPicTag_t)-4, infile );
  strncpy( pic->info->version, tag_name, _ipPicTAGLEN );

  ipFReadLE( &len, sizeof(ipUInt4_t), 1, infile );

  ipFReadLE( &(pic->type), sizeof(ipUInt4_t), 1, infile );
  ipFReadLE( &(pic->bpe), sizeof(ipUInt4_t), 1, infile );
  ipFReadLE( &(pic->dim), sizeof(ipUInt4_t), 1, infile );

  ipFReadLE( &(pic->n), sizeof(ipUInt4_t), pic->dim, infile );


  skip = len -        3 * sizeof(ipUInt4_t)
             - pic->dim * sizeof(ipUInt4_t);
  fseek( infile, skip, SEEK_CUR );

  picsize = _ipPicSize(pic);

  pic->dim = 2;

  if( slice < 1
      || slice > picsize / _ipPicSize(pic) )
    {
      ipPicClear( pic );
      return( pic );
    }

  pic->info->write_protect = ipTrue;

  picsize = _ipPicSize(pic);

  fseek( infile, picsize * (slice - 1), SEEK_CUR );

  pic->data = malloc( picsize );

  ipFReadLE( pic->data, pic->bpe / 8, _ipPicElements(pic), infile );

  if( infile != stdin )
    fclose( infile );

  return( pic );
}
