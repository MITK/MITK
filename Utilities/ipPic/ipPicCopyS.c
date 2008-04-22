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


/** @file
 *  copys a slice from a pic into a new pic
 */

 /*$Log$
  *Revision 1.7  2002/11/13 17:53:00  ivo
  *new ipPic added.
  *
 /*Revision 1.5  2002/04/23 11:06:25  ivo
 /*ipPicCopySlice behaves now in the same way as ipPicGetSlice:
 /*2D-slices are copied, no longer hyper-planes of dimension dim-1.
 /*
 /*Revision 1.4  2000/05/04 12:52:36  ivo
 /*inserted BSD style license
 /*
  *Revision 1.3  2000/05/04 12:35:58  ivo
  *some doxygen comments.
  *
 * Revision 1.2  1997/09/15  13:21:11  andre
 * switched to new what string format
 *
 * Revision 1.1.1.1  1997/09/06  19:09:58  andre
 * initial import
 *
 * Revision 0.0  1993/06/07  19:19:19  andre
 * Initial revision, NO error checking
 *
 *
 *--------------------------------------------------------------------
 *  COPYRIGHT (c) 1993 by DKFZ (Dept. MBI) Heidelberg, FRG
 */
#ifndef lint
  static char *what = { "@(#)ipPicCopySlice\t\tDKFZ (Dept. MBI)\t"__DATE__"\t$Revision$" };
#endif

#include "mitkIpPic.h"

ipPicDescriptor *_ipPicCopySlice( ipPicDescriptor *pic, ipPicDescriptor *pic_in, ipUInt4_t slice )
{
  ipUInt4_t picsize;

  if( pic == NULL )
    pic = ipPicNew();

  ipPicClear( pic );

  pic->type = pic_in->type;
  pic->bpe = pic_in->bpe;
  pic->dim = 2;
  pic->n[0] = pic_in->n[0];
  pic->n[1] = pic_in->n[1];
  pic->data = NULL;

  picsize = _ipPicSize(pic);

  if( slice < 1
      || slice > _ipPicSize(pic_in) / picsize )
    {
	  ipPicClear(pic);
      return( pic );
    }

  pic->data = malloc( picsize );

  memcpy( pic->data,
	  &((char *)pic_in->data)[(slice-1) * picsize],
	  picsize );

  return( pic );
}
