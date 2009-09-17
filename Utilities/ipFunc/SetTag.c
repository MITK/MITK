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
 *  this function sets a tag into an image
 */

/** @brief sets a tag into an image
 *  If the tag does not exist, it is created. Otherwise the existing
 *  tag is replaced by the new one. If type==mitkIpPicASCII and the last
 *  byte is not \0, a \0 is added.
 *
 *  @param pic image into which the tag is set
 *  @param name name of this tag
 *  @param size size of the whole data
 *  @param el_size size of one element of the data
 *  @param data pointer to the data
 *  @return 
 *  @arg 0 : error (pic==NULL or data==NULL)
 *  @arg 1 : new tag added
 *  @arg 2 : tag replaced
 *
 *  @b Examples:
 *	@code 
 *  mitkIpFuncSetTag(pic, "OUTSIDE_VALUE", mitkIpPicInt, sizeof(mitkIpInt4_t), 1, &outside_value);
 *
 *  char startpoint[11];
 *  memset(startpoint, 0, 11);
 *  sprintf(startpoint,"(%u/%u)",*x,*y);
 *  mitkIpFuncSetTag(p, "STARTPOINT", mitkIpPicASCII, 11, 1, startpoint);
 *  @endcode
 *  @warning not yet extensively tested.
 *
 *  @author Ivo (based on Gerald's NewTag)
 *  @date 26.06.2000
 * COPYRIGHT (c) 1995 by DKFZ (Dept. MBI) HEIDELBERG, FRG
 */

#include "mitkIpFuncP.h"

int mitkIpFuncSetTag( mitkIpPicDescriptor *pic, char *name, int type, int size, 
                  int el_size, void *data )
{
  mitkIpPicTSV_t *tsv;
  int alloc_size;
  int add_new;

  if((pic==NULL) || (data==NULL)) return 0;

  alloc_size=size;
  if((type==mitkIpPicASCII) && (((char*)data)[size-1]!=0)) alloc_size++;

  tsv = mitkIpPicQueryTag(pic, name);
  if (tsv != NULL)
  {
    tsv->value = realloc(tsv->value, alloc_size * el_size );
    add_new = 0;
  }
  else
  {
    tsv = malloc( sizeof(mitkIpPicTSV_t) );
    tsv->value = malloc( alloc_size * el_size );
	strcpy( tsv->tag, name);
    add_new = 1;
  }
  tsv->type = type;
  tsv->bpe = 8*el_size;
  tsv->dim = 1;
  tsv->n[0] = alloc_size;
  tsv->value = memcpy( tsv->value, data, size * el_size );
  if(alloc_size!=size) ((char*)tsv->value)[size]=0;

  if(add_new==1)
    mitkIpPicAddTag( pic, tsv );

  return add_new+1;
}
