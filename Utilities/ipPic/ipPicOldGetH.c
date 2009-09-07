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
 *   reads the header of a oldstyle PicFile from disk
 *
 * $Log$
 * Revision 1.7  2002/11/13 17:53:00  ivo
 * new ipPic added.
 *
 * Revision 1.5  2000/05/04 12:52:39  ivo
 * inserted BSD style license
 *
 * Revision 1.4  2000/05/04 12:36:00  ivo
 * some doxygen comments.
 *
 * Revision 1.3  2000/01/13  14:03:07  andre
 * *** empty log message ***
 *
 * Revision 1.2  1997/09/15  13:21:17  andre
 * switched to new what string format
 *
 * Revision 1.1.1.1  1997/09/06  19:09:59  andre
 * initial import
 *
 * Revision 0.2  1993/04/26  17:40:09  andre
 * 8 bit images from old pic format are now unsigned
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
  static char *what = { "@(#)_mitkIpPicOldGetHeader\tDKFZ (Dept. MBI)\t"__DATE__"\t$Revision$" };
#endif

#include "mitkIpPic.h"

#ifdef DOS
#include "mitkIpPicOP.h"
#else
#include "mitkIpPicOldP.h"
#endif

mitkIpPicDescriptor * _mitkIpPicOldGetHeader( FILE *infile, mitkIpPicDescriptor *pic )
{
  _mitkIpPicOldHeader old_pic;

  /*unsigned char *text;*/

  /* read infile */
  size_t ignored = mitkIpFReadLE( &(old_pic.dummy1), sizeof(ipUInt4_t), 4, infile );
  if( old_pic.conv <= 0 || old_pic.conv > 6 )
    {
      old_pic.conv = 3;
      old_pic.rank = 2;
    }

  ignored = mitkIpFReadLE( &(old_pic.n1), sizeof(ipUInt4_t),old_pic.rank, infile );
  if( old_pic.rank == 3 && old_pic.n3 == 1 )
    old_pic.rank = 2;

  ignored = mitkIpFReadLE( &(old_pic.type), sizeof(ipUInt4_t), 3, infile );
  if( old_pic.ntxt )
    {
      /*text = (unsigned char *)malloc( old_pic.ltxt );
      ignored = mitkIpFReadLE( text, 1, old_pic.ltxt, infile );*/
      fseek( infile, old_pic.ltxt, SEEK_CUR );
    }

  /* convert to the new pic3 format */

  if( pic == NULL )
    pic = mitkIpPicNew();

  mitkIpPicClear( pic );

  pic->data = NULL;
  if( old_pic.type == 1 )
    pic->type = mitkIpPicUInt;
  else
    pic->type = (mitkIpPicType_t)old_pic.conv;
  pic->bpe = old_pic.type*8;
  pic->dim = old_pic.rank;
  pic->n[0] = old_pic.n1;
  pic->n[1] = old_pic.n2;
  pic->n[2] = old_pic.n3;
  pic->n[3] = old_pic.n4;
  pic->n[4] = old_pic.n5;
  pic->n[5] = old_pic.n6;
  pic->n[6] = old_pic.n7;
  pic->n[7] = old_pic.n8;

  return( pic );
}
