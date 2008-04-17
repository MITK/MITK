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
 *  this function normalizes an image 
 */

/** @brief normalizes an image             
 *  @param pic_old    pointer to the image that should be inverted
 *  @param pic_return memory used to store return image ( if pic_return == NULL
 *                new memory is allocated )
 *
 * @return pointer to the normalized image
 *
 * AUTHOR & DATE
 */

/* include-Files                                                        */

#include "ipFuncP.h"   

ipPicDescriptor *ipFuncNorm ( ipPicDescriptor *pic_old,
                              ipPicDescriptor *pic_return );

#ifndef DOXYGEN_IGNORE

 
/* definition of macro for normalisation                                */

#define NORM( type, pic, min, max, min_gv, max_gv )                      \
{                                                                        \
  ipUInt4_t  i, no_elem;                                                 \
  ipFloat8_t a, b;                                                       \
                                                                         \
  a = ( ipFloat8_t ) ( max_gv - min_gv ) / ( max - min );                \
  b = ( ipFloat8_t ) ( max_gv ) - a * max;                               \
                                                                         \
  no_elem = _ipPicElements ( pic );                                      \
  for ( i = 0; i < no_elem; i++ )                                        \
    {                                                                    \
      (( type * ) pic_new->data ) [i] =                                  \
          ( type ) ( a  * (( type * ) pic->data ) [i] + b );             \
    }                                                                    \
} 

 

/* -------------------------------------------------------------------  */
/*
**  function picNorm: normalizes the greyvalues of the picture (pic_old)
**                    and returns the normalized picture (pic_new)
*/
/* -------------------------------------------------------------------  */

ipPicDescriptor *ipFuncNorm ( ipPicDescriptor *pic_old,
                              ipPicDescriptor *pic_return )
{

  ipPicDescriptor *pic_new;  /* inverted picture                        */
  ipFloat8_t      max, min;  /* extreme greyvalues in the image         */
  ipFloat8_t      max_gv;    /* max. possible greyvalue                 */
  ipFloat8_t      min_gv;    /* min. possible greyvalue                 */


  /* check image data                                                   */

  if ( _ipFuncError ( pic_old ) != mitkIpFuncOK ) return ( mitkIpFuncERROR );

  /* calculate extreme grevalues of the image                           */

  if ( ipFuncExtr ( pic_old, &min, &max ) != mitkIpFuncOK ) return ( mitkIpFuncERROR );

  /* calculate max. or min possible greyvalue for datatype              */

  if ( _ipFuncExtT ( pic_old->type, pic_old->bpe, &min_gv, &max_gv ) != mitkIpFuncOK ) 
    return ( mitkIpFuncERROR );

  /* test whether normalisation is necessary                            */
 
  if ( ( max_gv == max ) && ( min_gv == min ) )
    return ( pic_old );

  /* create a new picture, copy the header, allocate memory             */

  pic_new = _ipFuncMalloc ( pic_old, pic_return, mitkIpOVERWRITE );     
  if ( pic_new == NULL ) return ( mitkIpFuncERROR );

  /* macro to invert the picture (for all data types)                   */

  mitkIpPicFORALL_4( NORM, pic_old, min, max, min_gv, max_gv );
                        
  /* Copy Tags */

  ipFuncCopyTags(pic_new, pic_old);
  
  

  return pic_new;
}
#endif
