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
 *   This Function calculates the center of gravity of an region of
 *   interest in an binary image
 */

/**  @brief calculates the center of gravity of an region of
 *   interest in an binary image
 *
 *   @param pic     binary image: contains the region of interest
 *
 *   @return pointer to the vector which contains the coordinates 
 *              of the center of gravity
 *
 * AUTHOR & DATE
 */



/* include Files                                                        */

#include "mitkIpFuncP.h"

ipFloat8_t *mitkIpFuncGrav ( mitkIpPicDescriptor *pic_old );

#ifndef DOXYGEN_IGNORE 

/* definition of reflect-macro                                          */

#define GRAV( type, pic, index )                                          \
{                                                                         \
  ipUInt4_t   i;                                                          \
  ipUInt4_t   no;                                                         \
  ipUInt4_t   offset_refl;                                                \
  ipInt4_t    n[_mitkIpPicNDIM];                                              \
                                                                          \
  for ( i = 0; i < pic_old->dim; i++ )                                    \
    n[i] = pic_old->n[i];                                                 \
                                                                          \
  for ( i = pic_old->dim; i < _mitkIpPicNDIM; i++ )                           \
    n[i] = 1;                                                             \
                                                                          \
  no          = 0;                                                        \
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
                  if ( (( type * )pic->data )[offset_refl] )              \
                    {                                                     \
                       for ( i = 0; i < pic->dim; i++ )                   \
                          {                                               \
                             gravity[i] = gravity[i] + index[i];          \
                          }                                               \
                        no++;                                             \
                    }                                                     \
                  offset_refl++;                                          \
                }                                                         \
                                                                          \
      for ( i = 0; i < pic->dim; i++ ) gravity[i] = gravity[i]/no;        \
}



/* -------------------------------------------------------------------  */
/*
*/
/* -------------------------------------------------------------------  */

ipFloat8_t *mitkIpFuncGrav ( mitkIpPicDescriptor *pic_old )            
{

  ipUInt4_t       index_vect[_mitkIpPicNDIM]; /* loopindex-vector           */
  ipInt4_t        n[_mitkIpPicNDIM];          /* number of pixels in each   */
                                          /* dimension                  */
  ipUInt4_t       i;                      /* loop index                 */
  ipFloat8_t      *gravity;

  /* check data                                                         */

  if ( _mitkIpFuncError ( pic_old ) != mitkIpFuncOK ) return ( mitkIpFuncERROR );
 
  /* initialisation of vectors                                          */

  for ( i = 0; i < pic_old->dim; i++ )
    n[i] = pic_old->n[i];
  
  for ( i = pic_old->dim; i < _mitkIpPicNDIM; i++ )
    n[i] = 1;

  for ( i = 0; i < _mitkIpPicNDIM; i++ )
    index_vect[i] = 0;

  gravity = ( ipFloat8_t * ) malloc ( 8 * sizeof ( ipFloat8_t ) );
  if ( gravity == NULL ) 
    {
       _mitkIpFuncSetErrno ( mitkIpFuncMALLOC_ERROR );
       return ( mitkIpFuncERROR );
    }

  for ( i = 0; i < _mitkIpPicNDIM; i++ )
    gravity[i] = 0.;
  
  mitkIpPicFORALL_1 ( GRAV, pic_old, index_vect ) 

  return gravity;
}
#endif
