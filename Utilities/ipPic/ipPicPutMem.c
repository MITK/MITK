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
 *  writes a PicStructure to memory block
 */

#ifndef lint
  static char *what = { "@(#)mitkIpPicPutMem\t\tDKFZ (Dept. MBI)\t"__DATE__"\t$Revision$" };
#endif

#include "mitkIpPic.h" 

 /** FUNCTION DECLARATION
 *  
 *
 * PARAMETERS
 *  
 *
 * AUTHOR & DATE
 *  Harald Evers   15.7.98
 *
 * UPDATES
 *
 *--------------------------------------------------------------------
 *  COPYRIGHT (c) 1998 by DKFZ (Dept. MBI) Heidelberg, FRG
 */



void _mitkIpPicWriteTagsMem( _mitkIpPicTagsElement_t *head, ipUInt1_t **mem_ptr )
{
  _mitkIpPicTagsElement_t *current = head;

  while( current != NULL )
    {
      ipUInt4_t  elements;
      ipUInt4_t  len;

      elements = _mitkIpPicTSVElements( current->tsv );

      if( current->tsv->type == mitkIpPicTSV )
        {
          if( current->tsv->dim == 0 )
            {
              current->tsv->n[0] = _mitkIpPicTagsNumber(current->tsv->value);

              if( current->tsv->n[0] > 0 )
                current->tsv->dim = 1;
            }

          assert( elements == _mitkIpPicTagsNumber(current->tsv->value) );

          len = _mitkIpPicTagsSize( current->tsv->value );
        }
      else
        {
          if( current->tsv->type == mitkIpPicASCII
              || current->tsv->type == mitkIpPicNonUniform )
            current->tsv->bpe = 8;

          len = elements * current->tsv->bpe / 8;

         }

      len +=                   3 * sizeof(ipUInt4_t)  /* type, bpe, dim */
             + current->tsv->dim * sizeof(ipUInt4_t); /* n[]            */

      memcpy( *mem_ptr, current->tsv->tag, sizeof(mitkIpPicTag_t) );
      *mem_ptr += sizeof(mitkIpPicTag_t);
      memcpy( *mem_ptr, &len, sizeof(ipUInt4_t) );
      *mem_ptr += sizeof(ipUInt4_t);
      memcpy( *mem_ptr, &(current->tsv->type), sizeof(ipUInt4_t) );
      *mem_ptr += sizeof(ipUInt4_t);
      memcpy( *mem_ptr, &(current->tsv->bpe), sizeof(ipUInt4_t) );
      *mem_ptr += sizeof(ipUInt4_t);
      memcpy( *mem_ptr, &(current->tsv->dim), sizeof(ipUInt4_t) );
      *mem_ptr += sizeof(ipUInt4_t);
      memcpy( *mem_ptr, &(current->tsv->n), 
	       current->tsv->dim*sizeof(ipUInt4_t) );
      *mem_ptr += current->tsv->dim*sizeof(ipUInt4_t);

      if( current->tsv->type == mitkIpPicTSV )
        {
          _mitkIpPicWriteTagsMem( current->tsv->value, mem_ptr );
        }
      else
        {
	  /* if( encryption_type == 'e' ... */

	  memcpy( *mem_ptr,current->tsv->value, current->tsv->bpe/8*elements );
	  *mem_ptr += current->tsv->bpe/8*elements;
	}
      current = current->next;
    }
  
}

ipUInt1_t *
mitkIpPicPutMem( mitkIpPicDescriptor *pic, int *mem_size )
{
  ipUInt4_t  len;
  ipUInt4_t  tags_len;
  ipUInt1_t *mem_pic = NULL;
  ipUInt1_t *mem_ptr;

  tags_len = _mitkIpPicTagsSize( pic->info->tags_head );

  len = tags_len +        3 * sizeof(ipUInt4_t)
                 + pic->dim * sizeof(ipUInt4_t);

  *mem_size = 
    sizeof(mitkIpPicTag_t) +               /* pic version     */
    sizeof(ipUInt4_t) +                /* pic data offset */
    len +                              /* header + tags   */
    _mitkIpPicSize( pic );                 /* size of data    */

  mem_pic = malloc( *mem_size );
  if( !mem_pic )
    return( NULL );

  mem_ptr = mem_pic;
  memcpy( mem_ptr, mitkIpPicVERSION, sizeof(mitkIpPicTag_t) );
  mem_ptr += sizeof(mitkIpPicTag_t);
  memcpy( mem_ptr, &len, sizeof(ipUInt4_t) );
  mem_ptr += sizeof(ipUInt4_t);
  memcpy( mem_ptr, &(pic->type), sizeof(ipUInt4_t) );
  mem_ptr += sizeof(ipUInt4_t);
  memcpy( mem_ptr, &(pic->bpe), sizeof(ipUInt4_t) );
  mem_ptr += sizeof(ipUInt4_t);
  memcpy( mem_ptr, &(pic->dim), sizeof(ipUInt4_t) );
  mem_ptr += sizeof(ipUInt4_t);
  memcpy( mem_ptr, &(pic->n), pic->dim*sizeof(ipUInt4_t) );
  mem_ptr += pic->dim*sizeof(ipUInt4_t);

  _mitkIpPicWriteTagsMem( pic->info->tags_head, &mem_ptr );

  memcpy( mem_ptr, pic->data, pic->bpe/8*_mitkIpPicElements(pic) );

  return( mem_pic );
}
