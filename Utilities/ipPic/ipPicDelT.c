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
 *   deletes a tsv from the pic
 */
#ifndef lint
  static char *what = { "@(#)ipPicDelTag\t\tDKFZ (Dept. MBI)\t"__DATE__"\t$Revision$" };
#endif

#include "mitkIpPic.h"
/**  deletes a tsv from the pic
 */



static ipPicTSV_t *_ipPicRemoveTag( _ipPicTagsElement_t **head,
                                    _ipPicTagsElement_t *which,
                                    const char *tag );

ipPicTSV_t *
ipPicDelTag( ipPicDescriptor *pic, const char *tag )
{
  _ipPicTagsElement_t *which;

  which = _ipPicFindTag( pic->info->tags_head, tag );

  return( _ipPicRemoveTag( &(pic->info->tags_head), which, tag ) );
}

ipPicTSV_t *
ipPicDelSubTag( ipPicTSV_t *parent, const char *tag )
{
  _ipPicTagsElement_t *which;

  if( parent->type != ipPicTSV )
    return( NULL );

  which = _ipPicFindTag( parent->value, tag );

  if( !which )
    return( NULL );

  if( which->tsv->type == ipPicTSV )
    return( NULL );

  parent->n[0]--;
  return( _ipPicRemoveTag( (_ipPicTagsElement_t **)&(parent->value), which, tag ) );
}

ipPicTSV_t *
_ipPicRemoveTag( _ipPicTagsElement_t **head, _ipPicTagsElement_t *which, const char *tag )
{
  ipPicTSV_t *tsv = NULL;

  if( which != NULL )
    {
      tsv = which->tsv;

      if( which->prev == NULL )              /* which is the current head */
        {
          *head = which->next;

          if( *head )
            (*head)->prev = NULL;
        }
      else if( which->next == NULL )         /* which is the current tail */
        {
          which->prev->next = NULL;
        }
      else                                   /* which is somewhere if the list */
        {
          which->prev->next = which->next;
          which->next->prev = which->prev;
        }

      free( which );
    }

  return( tsv );
}
