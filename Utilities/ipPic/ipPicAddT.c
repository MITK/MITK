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
 *  inserts a tsv into the pic
 */
#ifndef lint
  static char *what = { "@(#)ipPicAddTag\t\tDKFZ (Dept. MBI)\t"__DATE__"\t$Revision$" };
#endif

#include "ipPic.h"


void
ipPicAddTag( ipPicDescriptor *pic, ipPicTSV_t *tsv )
{
  pic->info->tags_head = _ipPicInsertTag( pic->info->tags_head, tsv );
}

void
ipPicAddSubTag( ipPicTSV_t *parent, ipPicTSV_t *tsv )
{
  if( parent->type != ipPicTSV )
    return;

  parent->n[0]++;
  parent->value = _ipPicInsertTag( parent->value, tsv );
}

_ipPicTagsElement_t *
_ipPicInsertTag( _ipPicTagsElement_t *head, ipPicTSV_t *tsv )
{
  int i;
  _ipPicTagsElement_t *new;
  _ipPicTagsElement_t *current;

  tsv->tag[_mitkIpPicTAGLEN] = '\0';
  for( i=strlen(tsv->tag); i<_mitkIpPicTAGLEN; i++ )
    tsv->tag[i] = ' ';

  new = malloc( sizeof(_ipPicTagsElement_t) );

  new->tsv = tsv;
  new->prev = NULL;
  new->next = NULL;

  if( head == NULL )
    {
      head = new;
    }
  else
    {
      current = head;

      while( current->next != NULL )
        {
          current = current->next;
        }

      new->prev = current;
      current->next = new;
    }

  return( head );
}
