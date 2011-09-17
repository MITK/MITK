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
 *  searches for a tag
 */

#ifndef lint
  static char *what = { "@(#)mitkIpPicQueryTag\t\tDKFZ (Dept. MBI)\t"__DATE__"\t$Revision$" };
#endif

#include "mitkIpPic.h"

/** searches for a tag
 * 
 * @param pic	the pic
 * @param tag	the tag to search for
 * @param head	the head of the tags list
 *
 * AUTHOR & DATE
 *  Andre Schroeter	27.08.94
 *
 * UPDATES
 *  none
 */

mitkIpPicTSV_t *
mitkIpPicQueryTag( mitkIpPicDescriptor *pic, const char *tag )
{
  _mitkIpPicTagsElement_t *found;

  found = _mitkIpPicFindTag( pic->info->tags_head, tag );

  if( found == NULL )
    return( NULL );
  else
    return( found->tsv );
}
mitkIpPicTSV_t *
mitkIpPicQuerySubTag( mitkIpPicTSV_t *parent, const char *tag )
{
  _mitkIpPicTagsElement_t *found;

  found = _mitkIpPicFindTag( parent->value, tag );

  if( found == NULL )
    return( NULL );
  else
    return( found->tsv );
}

_mitkIpPicTagsElement_t *
_mitkIpPicFindTag( _mitkIpPicTagsElement_t *head, const char *tag )
{
  int i;
  char name[_mitkIpPicTAGLEN+1];
  _mitkIpPicTagsElement_t *current;

  if( !tag )
    return( NULL );

  sprintf( name, "%.*s", _mitkIpPicTAGLEN, tag );
  for( i=strlen(name); i<_mitkIpPicTAGLEN; i++ )
    name[i] = ' ';

  current = head;
  while( current != NULL )
    {
      if( strncmp( current->tsv->tag, name, _mitkIpPicTAGLEN) == 0 )
        return( current );

      current = current->next;
    }

  return( NULL );
}
