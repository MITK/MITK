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

/**@file
*  this function inverts the greyvalues of an image
*/

/** @brief inverts the greyvalues of an image
 *
 *  @param pic_old  pointer to the image that should be inverted
 *  @param pic_return  memory which could be used for pic_new. If  
 *                pic_retrun == NULL new memory is allocated
 *
 *  @return  pointer to the inverted image
 *
 * AUTHOR & DATE
 */

/* include-Files                                                        */

#include "mitkIpFuncP.h"   

mitkIpPicDescriptor *mitkIpFuncInv ( mitkIpPicDescriptor *pic_old,
                             mitkIpPicDescriptor *pic_return );

#ifndef DOXYGEN_IGNORE

#ifndef lint
  static char *what = { "@(#)mitkIpFuncInv\t\tDKFZ (Dept. MBI)\t"__DATE__ };
#endif


 
/* definition of invert-macro                                           */

#define INV( type, pic )                                                 \
{                                                                        \
  mitkIpUInt4_t  i, no_elem;                                                 \
  type       help;                                                       \
                                                                         \
  /* calculate min. and max.                                           */\
                                                                         \
  type  max = ( ( type * ) pic->data ) [0];                              \
  type  min = ( ( type * ) pic->data ) [0];                              \
                                                                         \
  no_elem = _mitkIpPicElements ( pic );                                      \
  for ( i = 1; i < no_elem; i++ )                                        \
  {                                                                      \
    help = ( ( type * ) pic->data ) [i];                                 \
    max  = ( help > max ) ? help : max;                                  \
    min  = ( help < min ) ? help : min;                                  \
  }                                                                      \
                                                                         \
  /* invert greyvalues                                                 */\
                                                                         \
  for ( i = 0; i < no_elem; i++ )                                        \
  {                                                                      \
    (( type * ) pic_new->data ) [i] =                                    \
        max + min - (( type * ) pic->data ) [i];                         \
  }                                                                      \
} 
               
 

/* -------------------------------------------------------------------  */
/*
*/
/* -------------------------------------------------------------------  */

mitkIpPicDescriptor *mitkIpFuncInv ( mitkIpPicDescriptor *pic_old,
                             mitkIpPicDescriptor *pic_return )
{

  mitkIpPicDescriptor *pic_new;  /* inverted picture                        */

  /* check data                                                         */

  if ( _mitkIpFuncError ( pic_old ) != mitkIpFuncOK ) return ( mitkIpFuncERROR );

  /* create a new picture, copy the header, allocate memory             */

  pic_new = _mitkIpFuncMalloc ( pic_old, pic_return, mitkIpOVERWRITE );     
  if ( pic_new == NULL ) return ( mitkIpFuncERROR );

  /* macro to invert the picture (for all data types)                   */

  mitkIpPicFORALL ( INV, pic_old );

  mitkIpFuncCopyTags(pic_new, pic_old);
  
     

  return pic_new;
}
#endif

