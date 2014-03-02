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
 *  creates the clone of a pic
 */
#ifndef lint
  static char *what = { "@(#)mitkIpPicClone\t\tDKFZ (Dept. MBI)\t"__DATE__"\t$Revision$" };
#endif

#include "mitkIpPic.h"

/**
 * creates the clone of a pic
 * @param pic		the original pic
 *
 * @return the exact copy
 *
 *
 * AUTHOR & DATE
 *
 */

mitkIpPicDescriptor *mitkIpPicClone( mitkIpPicDescriptor *pic )
{
  mitkIpPicDescriptor *clone;

  clone = mitkIpPicCopyHeader( pic,
                           NULL );

  strncpy( clone->info->version, pic->info->version, _mitkIpPicTAGLEN );

  clone->info->tags_head = _mitkIpPicCloneTags( pic->info->tags_head );

  clone->info->write_protect = pic->info->write_protect;

  if( !pic->data )
    {
      clone->data = NULL;
    }
  else
    {
      mitkIpUInt4_t size;

      size = _mitkIpPicSize( pic );

      clone->data = malloc( size );
      if ( ! ( clone->data ) )
          return NULL;
      memmove( clone->data, pic->data, size );
    }

  return( clone );
}

_mitkIpPicTagsElement_t *_mitkIpPicCloneTags( _mitkIpPicTagsElement_t *head )
{
  _mitkIpPicTagsElement_t *new;
  _mitkIpPicTagsElement_t *current;

  new = NULL;
  current = head;
  while( current != NULL )
    {
      new = _mitkIpPicInsertTag( new, _mitkIpPicCloneTag(current->tsv) );

      current = current->next;
    }
  return( new );
}

mitkIpPicTSV_t *_mitkIpPicCloneTag( mitkIpPicTSV_t *source )
{
  mitkIpPicTSV_t *new;

  mitkIpUInt4_t i;

  new = malloc( sizeof(mitkIpPicTSV_t) );

  strncpy( new->tag, source->tag, _mitkIpPicTAGLEN );

  new->type = source->type;
  new->bpe = source->bpe;
  new->dim = source->dim;

  for( i = 0; i < source->dim; i++ )
    new->n[i] = source->n[i];

  if( source->type == mitkIpPicTSV )
    {
      new->value = _mitkIpPicCloneTags( source->value );
    }
  else
    {
      mitkIpUInt4_t  size;

      size = _mitkIpPicTSVSize( new );

      new->value = malloc( size );
      memmove( new->value, source->value, size );
    }

  return( new );
}
