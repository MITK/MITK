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
 * creates a PicStructure from memory block
 */
#ifndef lint
  static char *what = { "@(#)ipPicGetMem\t\tDKFZ (Dept. MBI)\t"__DATE__"\t$Revision$" };
#endif

#include "ipPic.h"
/** creates a PicStructure from memory block
 *
 * FUNCTION DECLARATION
 *  
 *
 * PARAMETERS
 *  
 *
 * AUTHOR & DATE
 *  Harald Evers   15.7.98
 *
 */



ipPicDescriptor *ipPicGetMem( ipUInt1_t *mem_pic )
{
  ipPicDescriptor *pic;
  ipUInt4_t        len;
  ipUInt4_t        to_read;
  ipUInt1_t       *mem_ptr = mem_pic;
  
  pic = ipPicNew();
  ipPicClear( pic );
  memcpy( &(pic->info->version), mem_ptr, sizeof(ipPicTag_t) );
  mem_ptr += sizeof(ipPicTag_t); 
  if( strncmp( ipPicVERSION, pic->info->version, 4 ) != 0 )
    {      
      ipPicFree( pic );
      return( NULL );
    }

  memcpy( &len,  mem_ptr, sizeof(ipUInt4_t) );
  mem_ptr += sizeof(ipUInt4_t);
  memcpy( &(pic->type),  mem_ptr, sizeof(ipUInt4_t) );
  mem_ptr += sizeof(ipUInt4_t);
  memcpy( &(pic->bpe),  mem_ptr, sizeof(ipUInt4_t) );
  mem_ptr += sizeof(ipUInt4_t);
  memcpy( &(pic->dim),  mem_ptr, sizeof(ipUInt4_t) );
  mem_ptr += sizeof(ipUInt4_t);
  memcpy( &(pic->n),  mem_ptr, pic->dim * sizeof(ipUInt4_t) );
  mem_ptr += pic->dim*sizeof(ipUInt4_t);

  to_read = len -        3 * sizeof(ipUInt4_t)
                - pic->dim * sizeof(ipUInt4_t);
  pic->info->tags_head = _ipPicReadTagsMem( pic->info->tags_head, 
					    to_read, &mem_ptr );

  pic->info->write_protect = ipFalse;
  pic->data = malloc( _ipPicSize(pic) );

  /*
    pic->info->pixel_start_in_file = ftell( infile );
  */
  
  memcpy( pic->data,  mem_ptr, pic->bpe/8*_ipPicElements(pic) );

  return( pic );
}

_ipPicTagsElement_t *
_ipPicReadTagsMem( _ipPicTagsElement_t *head, ipUInt4_t bytes_to_read, 
		   ipUInt1_t **mem_ptr )
{
  while( bytes_to_read > 0 )
    {
      ipPicTSV_t *tsv;
      ipUInt4_t   len;

      tsv = malloc( sizeof(ipPicTSV_t) );

      memcpy( tsv->tag, *mem_ptr, _ipPicTAGLEN );
      tsv->tag[_ipPicTAGLEN] = '\0';
      *mem_ptr += _ipPicTAGLEN;

      memcpy( &len,  *mem_ptr, sizeof(ipUInt4_t) );
      *mem_ptr += sizeof(ipUInt4_t);

      memcpy( &(tsv->type), *mem_ptr, sizeof(ipUInt4_t) );
      *mem_ptr += sizeof(ipUInt4_t);
      memcpy( &(tsv->bpe), *mem_ptr, sizeof(ipUInt4_t) );
      *mem_ptr += sizeof(ipUInt4_t);
      memcpy( &(tsv->dim), *mem_ptr, sizeof(ipUInt4_t) );
      *mem_ptr += sizeof(ipUInt4_t);
      memcpy( &(tsv->n), *mem_ptr, tsv->dim*sizeof(ipUInt4_t) );
      *mem_ptr += tsv->dim*sizeof(ipUInt4_t);

      if( tsv->type == ipPicTSV )
        {
          tsv->value = NULL;
          tsv->value = _ipPicReadTagsMem( tsv->value,
					  len -        3 * sizeof(ipUInt4_t)
  					      - tsv->dim * sizeof(ipUInt4_t),
					  mem_ptr );
        }
      else
        {
          ipUInt4_t  elements;

          elements = _ipPicTSVElements( tsv );

          if( tsv->type == ipPicASCII
              || tsv->type == ipPicNonUniform )
            tsv->bpe = 8;

          if( tsv->type == ipPicASCII )
            tsv->value = malloc( elements+1 * tsv->bpe / 8 );
          else
            tsv->value = malloc( elements * tsv->bpe / 8 );

	  memcpy( tsv->value,  *mem_ptr, tsv->bpe/8*_ipPicTSVElements(tsv) );
	  *mem_ptr += tsv->bpe/8*_ipPicTSVElements(tsv);
	  
          if( tsv->type == ipPicASCII )
            ((char *)(tsv->value))[elements] = '\0';

          /*
	    if( encryption_type == 'e' ..
	  */
        }

      head = _ipPicInsertTag( head, tsv );

      bytes_to_read -= 32                  /* name      */
                       + sizeof(ipUInt4_t) /* len       */
                       + len;              /* type + bpe + dim + n[] + data */
    }
  return( head );
}
