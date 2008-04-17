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
 *  this function reflects an image along one axis
 */

/** @brief reflects an image along one axis
 *
 *  @param pic_old   pointer to the image that should be inverted
 *  @param axis      reflection axis: 1 => x-axis
 *                                 2 => y-axis
 *                                 3 => z-axis 
 *                                 4 => t-axis
 *
 * @return pointer to the reflected image
 *
 * AUTHOR & DATE
 */
/* include Files                                                        */

#include "ipFuncP.h"

ipPicDescriptor *ipFuncRefl ( ipPicDescriptor *pic_old, int axis );

#ifndef DOXYGEN_IGNORE

/* definition of reflect-macro                                          */

#define ForLoop( type, pic, pic_new, index, offset_orig, offset_func )    \
{                                                                         \
  ipUInt4_t   i;                                                          \
  ipUInt4_t   offset_refl;                                                \
  ipInt4_t    n[_mitkIpPicNDIM];                                              \
                                                                          \
  for ( i = 0; i < pic_old->dim; i++ )                                    \
    n[i] = pic_old->n[i];                                                 \
                                                                          \
  for ( i = pic_old->dim; i < _mitkIpPicNDIM; i++ )                           \
    n[i] = 1;                                                             \
                                                                          \
  offset_refl = 0;                                                        \
  for ( index[7] = 0; index[7] < n[7]; index[7]++ )                       \
                                                                          \
    for ( index[6] = 0; index[6] < n[6]; index[6]++ )                     \
                                                                          \
      for ( index[5] = 0; index[5] < n[5]; index[5]++ )                   \
                                                                          \
        for ( index[4] = 0; index[4] < n[4]; index[4]++ )                 \
                                                                          \
          for ( index[3] = 0; index[3] < n[3]; index[3]++ )               \
                                                                          \
            for ( index[2] = 0; index[2] < n[2]; index[2]++ )             \
                                                                          \
              for ( index[1] = 0; index[1] < n[1]; index[1]++ )           \
                                                                          \
                for ( index[0] = 0; index[0] < n[0]; index[0]++ )         \
                {                                                         \
                  offset_func;                                            \
                  (( type * ) pic_new->data )  [offset_refl] =            \
                     (( type * ) pic->data ) [offset_orig];               \
                  offset_refl++;                                          \
                }                                                         \
}



/* -------------------------------------------------------------------  */
/*
**  function picRefl : reflects a picture (pic_old) along one axis
**                     (axis)
**                     axis:  1 => x-axis 
**                            2 => y-axis
**                            3 => z-axis
**                            4 => t-axis
*/
/* -------------------------------------------------------------------  */

ipPicDescriptor *ipFuncRefl ( ipPicDescriptor *pic_old, int axis )
{

  ipPicDescriptor *pic_new;               /* inverted picture           */
  ipUInt4_t       index_vect[_mitkIpPicNDIM]; /* loopindex-vector           */
  ipUInt4_t       length_vect[_mitkIpPicNDIM];
  ipUInt4_t       axis_vect[_mitkIpPicNDIM];               
  ipInt4_t        n[_mitkIpPicNDIM];          /* number of pixels in each   */
                                          /* dimension                  */
  ipUInt4_t       i, j;                   /* loop index                 */
  ipUInt4_t       offset_orig;

  /* check data                                                         */

  if ( _ipFuncError ( pic_old ) != mitkIpFuncOK ) return ( mitkIpFuncERROR );
  if ( axis < 1 || axis > pic_old->dim ) 
     {
       _ipFuncSetErrno ( mitkIpFuncDATA_ERROR );
       return ( mitkIpFuncERROR );
     }
 
  /* initialisation of vectors                                          */

  for ( i = 0; i < pic_old->dim; i++ )
    n[i] = pic_old->n[i];
  
  for ( i = pic_old->dim; i < _mitkIpPicNDIM; i++ )
    n[i] = 1;

 
  for ( i = 0; i < _mitkIpPicNDIM; i++ )
  {
    index_vect[i] = 0;
    axis_vect[i] = 0;
  }

  /* change index for x and y axis                                      */

  if ( axis == 1 )
    axis = 2;
  else if ( axis == 2 )
    axis = 1; 
  axis_vect[axis - 1] = n[axis - 1] - 1;

  length_vect[0]  = 1;
  for ( i = 1; i < pic_old->dim; i++ )
  {
    length_vect[i] = n[i-1] * length_vect[i-1];
  } 

  /* create a new picture, copy the header, allocate memory             */

  pic_new = ipPicCopyHeader ( pic_old, 0 );
  if ( pic_new == NULL )
    {
       _ipFuncSetErrno ( mitkIpFuncPICNEW_ERROR );
       return ( mitkIpFuncERROR );
    }
  pic_new->data = malloc ( _ipPicSize ( pic_new ) );
  if ( pic_new->data == NULL )
    {
       ipPicFree ( pic_new );
       _ipFuncSetErrno ( mitkIpFuncPICNEW_ERROR );
       return ( mitkIpFuncERROR );
    }

  /* macro to reflect image (for all image data types)                  */
  mitkIpPicFORALL_4 ( ForLoop, pic_old, pic_new, index_vect, offset_orig, 
                  offset_orig = 0;
                  for ( i = 0; i < pic_old->dim; i++ )
                  {
                    offset_orig = offset_orig + length_vect [i] * 
                                  (( axis_vect [i] == 0 ) ? 
                                     index_vect [i] : 
                                     ( axis_vect [i] - index_vect[i] ));
                  }
                ) 

  /* Copy Tags */

  ipFuncCopyTags(pic_new, pic_old);
  
  
                        
  return pic_new;
}
#endif
