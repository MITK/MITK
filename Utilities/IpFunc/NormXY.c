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
 *  this function normalizes an image to a specified range 
*/

/** @brief normalizes an image to a specified range
 *
 *  @param pic_old   pointer to the image to normalize
 *  @param low       lower border of range
 *  @param up       upper border of range
 *  @param pic_return  memory used to store return image ( if pic_return == NULL
 *                new memory is allocated )
 *
 * @return pointer to the noRmalized image
 *
 * AUTHOR & DATE
 */

/* include-Files                                                        */

#include "mitkIpFuncP.h"   
 
mitkIpPicDescriptor *mitkIpFuncNormXY ( mitkIpPicDescriptor *pic_old, 
                                mitkIpFloat8_t      low,   
                                mitkIpFloat8_t      up,
                                mitkIpPicDescriptor *pic_return );

#ifndef DOXYGEN_IGNORE

/* definition of macro for normalisation                                */

#define NORM( type, pic, min, max, min_gv, max_gv )                      \
{                                                                        \
  mitkIpUInt4_t  i, no_elem;                                                 \
  mitkIpFloat8_t a, b;                                                       \
                                                                         \
  a = ( mitkIpFloat8_t ) ( max_gv - min_gv ) / ( max - min );                \
  b = ( mitkIpFloat8_t ) ( max_gv ) - a * max;                               \
                                                                         \
  no_elem = _mitkIpPicElements ( pic );                                      \
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

mitkIpPicDescriptor *mitkIpFuncNormXY ( mitkIpPicDescriptor *pic_old, 
                                mitkIpFloat8_t      low,   
                                mitkIpFloat8_t      up,
                                mitkIpPicDescriptor *pic_return )
{

  mitkIpPicDescriptor *pic_new;  /* inverted picture                        */
  mitkIpFloat8_t      max, min;  /* extreme greyvalues in the image         */
  mitkIpFloat8_t      max_gv;    /* max. possible greyvalue                 */
  mitkIpFloat8_t      min_gv;    /* min. possible greyvalue                 */


  /* check whether data are ok                                          */

  if ( _mitkIpFuncError ( pic_old ) != mitkIpFuncOK ) return ( mitkIpFuncERROR );

  /* calculate extreme grevalues of the image                           */

  if ( mitkIpFuncExtr ( pic_old, &min, &max ) != mitkIpFuncOK ) return ( mitkIpFuncERROR );

  /* calculate max and min. possible greyvalues                         */

  if ( _mitkIpFuncExtT ( pic_old->type, pic_old->bpe, &min_gv, &max_gv ) != mitkIpFuncOK ) 
    return ( mitkIpFuncERROR );

  /* check data                                                         */

  if ( max == min )
    {
       _mitkIpFuncSetErrno ( mitkIpFuncDATA_ERROR );
       return ( mitkIpFuncERROR );
    }

  if ( low >= up ) 
    {
       _mitkIpFuncSetErrno ( mitkIpFuncDATA_ERROR );
       return ( mitkIpFuncERROR );
    }

  if ( low < min_gv || up > max_gv ) 
    {
       _mitkIpFuncSetErrno ( mitkIpFuncDATA_ERROR );
       return ( mitkIpFuncERROR );
    }

  /* test whether normalisation is necessary                            */
 
  if ( ( up == max ) && ( low == min ) )
    return ( pic_old );

  /* create a new picture, copy the header, allocate memory             */

  pic_new = _mitkIpFuncMalloc ( pic_old, pic_return, mitkIpOVERWRITE );     
  if ( pic_new == NULL ) return ( mitkIpFuncERROR );

  /* macro to invert the picture (for all data types)                   */

  mitkIpPicFORALL_4( NORM, pic_old, min, max, low, up );

  /* Copy Tags */

  mitkIpFuncCopyTags(pic_new, pic_old);
  
  
                        
  return pic_new;
}
#endif

