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
 *  This function sets all border pixels to a spezified value.
 */

/** @brief sets all border pixels to a spezified value.
 *
 *  The size of the border is calculated using the size of the single
 *  dimensions of the mask (dim/2)
 *
 * 
 *  @param pic_old input image
 *  @param mask mask which is used to calculate the border size
 *  @param value border value
 *  @param pic_rtn memory used for result image (if pic_rtn == NULL new
 *               memory is allocated.
 *
 *  @return pic_new result image
 *
 * AUTHOR & DATE
 */

/* include files                                                                  */

#include "mitkIpFuncP.h"

mitkIpPicDescriptor *mitkIpFuncBorderX ( mitkIpPicDescriptor *pic_old,
                                 mitkIpPicDescriptor *mask,
                                 mitkIpFloat8_t      value,
                                 mitkIpPicDescriptor *pic_rtn );


#ifndef DOXYGEN_IGNORE

#ifndef lint
  static char *what = { "@(#)????????\t\tDKFZ (Dept. MBI)\t"__DATE__ };
#endif



/* definition of macros                                                           */

#define BORDER( type, pic, size, value )                                           \
{                                                                                  \
  mitkIpUInt4_t      i;                /* loop index                                */ \
  mitkIpUInt4_t      anf[_mitkIpPicNDIM];  /*                                           */ \
  mitkIpUInt4_t      end[_mitkIpPicNDIM];  /*                                           */ \
  mitkIpUInt4_t      ind[_mitkIpPicNDIM];  /* loop index vector                         */ \
  mitkIpUInt4_t      off[_mitkIpPicNDIM];  /* used to calculate offset of image pixels  */ \
                                                                                   \
                                                                                   \
  for ( i = 0; i < pic->dim; i++ )                                                 \
    {                                                                              \
       anf[i] = 0;                                                                 \
       end[i] = pic->n[i];                                                         \
    }                                                                              \
  for ( i = pic->dim; i < _mitkIpPicNDIM; i++ )                                        \
    {                                                                              \
       anf[i] = 0;                                                                 \
       end[i] = 1;                                                                 \
       pic->n[i] = 1;                                                              \
    }                                                                              \
  for ( i = 0; i < pic->dim; i++ )                                                 \
    {                                                                              \
       anf[i] = pic->n[i] - mask->n[i]/2;                                          \
       end[i] = mask->n[i]/2;                                                      \
       for ( ind[7] = 0; ind[7] < end[7]; ind[7]++ )                               \
       {                                                                           \
         off[7] = size[7] * ind[7];                                                \
         for ( ind[6] = 0; ind[6] < end[6]; ind[6]++ )                             \
         {                                                                         \
           off[6] = size[6] * ind[6] + off[7];                                     \
           for ( ind[5] = 0; ind[5] < end[5]; ind[5]++ )                           \
           {                                                                       \
             off[5] = size[5] * ind[5] + off[6];                                   \
             for ( ind[4] = 0; ind[4] < end[4]; ind[4]++ )                         \
             {                                                                     \
               off[4] = size[4] * ind[4] + off[5];                                 \
               for ( ind[3] = 0; ind[3] < end[3]; ind[3]++ )                       \
               {                                                                   \
                 off[3] = size[3] * ind[3] + off[4];                               \
                 for ( ind[2] = 0; ind[2] < end[2]; ind[2]++ )                     \
                 {                                                                 \
                   off[2] = size[2] * ind[2] + off[3];                             \
                   for ( ind[1] = 0; ind[1] < end[1]; ind[1]++ )                   \
                   {                                                               \
                     off[1] = size[1] * ind[1] + off[2];                           \
                     for ( ind[0] = 0; ind[0] < end[0]; ind[0]++ )                 \
                     {                                                             \
                       (( type * ) pic_new->data )[off[1]] = ( type ) value;       \
                       off[1]++;                                                   \
                     }                                                             \
                   }                                                               \
                 }                                                                 \
               }                                                                   \
             }                                                                     \
           }                                                                       \
         }                                                                         \
       }                                                                           \
       for ( ind[7] = anf[7]; ind[7] < pic->n[7]; ind[7]++ )                       \
       {                                                                           \
         off[7] = size[7] * ind[7];                                                \
         for ( ind[6] = anf[6]; ind[6] < pic->n[6]; ind[6]++ )                     \
         {                                                                         \
           off[6] = size[6] * ind[6] + off[7];                                     \
           for ( ind[5] = anf[5]; ind[5] < pic->n[5]; ind[5]++ )                   \
           {                                                                       \
             off[5] = size[5] * ind[5] + off[6];                                   \
             for ( ind[4] = anf[4]; ind[4] < pic->n[4]; ind[4]++ )                 \
             {                                                                     \
               off[4] = size[4] * ind[4] + off[5];                                 \
               for ( ind[3] = anf[3]; ind[3] < pic->n[3]; ind[3]++ )               \
               {                                                                   \
                 off[3] = size[3] * ind[3] + off[4];                               \
                 for ( ind[2] = anf[2]; ind[2] < pic->n[2]; ind[2]++ )             \
                 {                                                                 \
                   off[2] = size[2] * ind[2] + off[3];                             \
                   for ( ind[1] = anf[1]; ind[1] < pic->n[1]; ind[1]++ )           \
                   {                                                               \
                     off[1] = size[1] * ind[1] + off[2];                           \
                     off[0] = off[1] + anf[0];                                     \
                     for ( ind[0] = anf[0]; ind[0] < pic->n[0]; ind[0]++ )         \
                     {                                                             \
                       (( type * ) pic_new->data )[off[0]] = ( type ) value;       \
                       off[0]++;                                                   \
                     }                                                             \
                   }                                                               \
                 }                                                                 \
               }                                                                   \
             }                                                                     \
           }                                                                       \
         }                                                                         \
       }                                                                           \
     end[i] = pic->n[i];                                                           \
     anf[i] = 0;                                                                   \
    }\
}

/* ------------------------------------------------------------------------------ */
/*
*/
/* ------------------------------------------------------------------------------ */

mitkIpPicDescriptor *mitkIpFuncBorderX ( mitkIpPicDescriptor *pic_old,
                                 mitkIpPicDescriptor *mask,
                                 mitkIpFloat8_t      value,
                                 mitkIpPicDescriptor *pic_rtn )
{
  mitkIpPicDescriptor *pic_new;            /* pointer to transformed image            */
  mitkIpUInt4_t       i;                   /* loop index                              */
  mitkIpUInt4_t      size[_mitkIpPicNDIM];     /*                                         */
  mitkIpFloat8_t     max_gv, min_gv;

  /* check whEther data are correct                                               */

  if ( _mitkIpFuncError ( pic_old ) != mitkIpFuncOK ) return ( mitkIpFuncERROR );
  if ( _mitkIpFuncError ( mask ) != mitkIpFuncOK ) return ( mitkIpFuncERROR );
  if ( ( pic_old->dim != mask->dim ) || ( mask->dim > 3 ) ) 
    { 
       _mitkIpFuncSetErrno ( mitkIpFuncDIM_ERROR );
       return ( mitkIpFuncERROR );
    }

  for ( i = 0; i < pic_old->dim; i++ )
    {
       if ( pic_old->n[i] <= mask->n[i]  ) 
         { 
            _mitkIpFuncSetErrno ( mitkIpFuncUNFIT_ERROR );
            return ( mitkIpFuncERROR );
         }
    }

  if ( _mitkIpFuncExtT ( pic_old->type, pic_old->bpe, &min_gv, &max_gv ) != mitkIpFuncOK )
    {
       _mitkIpFuncSetErrno ( mitkIpFuncDATA_ERROR );
       return ( mitkIpFuncERROR );
    }
  if ( ( value > max_gv ) ||  ( value < min_gv ) )
    {
       _mitkIpFuncSetErrno ( mitkIpFuncDATA_ERROR );
       return ( mitkIpFuncERROR );
    }
    
  /* create a new picture, copy the header, allocate memory             */

  /*  pic_new = _mitkIpFuncMalloc ( pic_old, pic_rtn, mitkIpOVERWRITE );*/
  if ( pic_rtn == NULL )
     pic_new = mitkIpPicClone ( pic_old );
  else 
     pic_new = pic_old;

  if ( pic_new == NULL ) return ( mitkIpFuncERROR );

  /* initialize vectors and variables                                             */

  size[0] = 1;
  for ( i = 1; i < _mitkIpPicNDIM; i++ )
    size[i] = size[i-1] * pic_old->n[i-1];
  size[pic_old->dim] = 0;

  mitkIpPicFORALL_2 ( BORDER, pic_old, size, value )    

  /* Copy Tags */

  mitkIpFuncCopyTags(pic_new, pic_old);
  
  

  return ( pic_new );
}
#endif

