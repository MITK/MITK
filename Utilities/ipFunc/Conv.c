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
 *  this function convolves an image with a mask  
 */

/**  this function convolves an image with a mask 
 *  @param pic_old pointer to the image that should be convolved   
 *  @param mask pointer to the mask that is used for convolution
 *  @param border tells how the edge is transformed              
 *  @arg @c mitkIpFuncFlagBorderOld  : original greyvalues         
 *  @arg @c mitkIpFuncFlagBorderZero : edge is set to minimal greyvalue
 *
 *  @return pointer to the convolved image 
 *
 * AUTHOR & DATE
 */

/* include-Files                                                        */

#include "mitkIpFuncP.h"   
 
mitkIpPicDescriptor *mitkIpFuncConv( mitkIpPicDescriptor *pic_old,           
                             mitkIpPicDescriptor *pic_mask, 
                             mitkIpFuncFlagI_t   border );

#ifndef DOXYGEN_IGNORE

#ifndef lint
  static char *what = { "@(#)mitkIpFuncConv\t\tDKFZ (Dept. MBI)\t"__DATE__ };
#endif



/* definition of macros                                                 */

#define CONV( typ, pic, size, mask, beg, end, m)                         \
{                                                                        \
   mitkIpInt4_t         ind[_mitkIpPicNDIM];                                     \
   mitkIpInt4_t         off[_mitkIpPicNDIM];                                     \
   mitkIpUInt4_t        i;                                                   \
   mitkIpInt4_t         offset;                                              \
   mitkIpFloat8_t       help, min_gv, max_gv;                                \
   typ              *data, *data_new;                                    \
                                                                         \
   /* calculate max. and min. possible greyvalues                     */ \
                                                                         \
   _mitkIpFuncExtT ( pic->type, pic->bpe, &min_gv, &max_gv );                \
   data = pic->data;                                                     \
   data_new = pic_new->data;                                             \
                                                                         \
   /* convolve image with compressed mask                             */ \
                                                                         \
   for ( ind[7] = beg[7]; ind[7] < end[7]; ind[7]++ )                    \
   {                                                                     \
     off[7] = size[7] * ind[7];                                          \
     for ( ind[6] = beg[6]; ind[6] < end[6]; ind[6]++ )                  \
     {                                                                   \
       off[6] = size[6] * ind[6] + off[7];                               \
       for ( ind[5] = beg[5]; ind[5] < end[5]; ind[5]++ )                \
       {                                                                 \
         off[5] = size[5] * ind[5] + off[6];                             \
         for ( ind[4] = beg[4]; ind[4] < end[4]; ind[4]++ )              \
         {                                                               \
           off[4] = size[4] * ind[4] + off[5];                           \
           for ( ind[3] = beg[3]; ind[3] < end[3]; ind[3]++ )            \
           {                                                             \
             off[3] = size[3] * ind[3] + off[4];                         \
             for ( ind[2] = beg[2]; ind[2] < end[2]; ind[2]++ )          \
             {                                                           \
               off[2] = size[2] * ind[2] + off[3];                       \
               for ( ind[1] = beg[1]; ind[1] < end[1]; ind[1]++ )        \
               {                                                         \
                 off[1] = size[1] * ind[1] + off[2];                     \
                 offset = off[1] + beg[0];                               \
                 for ( ind[0] = beg[0]; ind[0] < end[0]; ind[0]++ )      \
                   {                                                     \
                                                                         \
                      /* calculate new greyvalue                      */ \
                                                                         \
                      help = 0;                                          \
                      for ( i = 0; i < m->length; i++ )                  \
                        help  = help + m->mask_vekt[i] *                 \
                                       data[offset + m->off_vekt[i]];    \
                                                                         \
                      data_new[offset] =                                 \
                         ( help < max_gv )?                              \
                         ( ( help < min_gv ) ? ( typ )min_gv : help ) :  \
                         ( typ ) max_gv;                                 \
                      /*                                                 \
                        help = help + m->mask_vekt[i] *                  \
                          (( typ * )pic->data)[offset + m->off_vekt[i]]; \
                                                                         \
                      (( typ * )pic_new->data )[offset] =                \
                         ( help < max_gv )?                              \
                         ( ( help < min_gv ) ? ( typ )min_gv : help ) :  \
                         ( typ ) max_gv;                                 \
                      */                                                 \
                      offset++;                                          \
                    }                                                    \
               }                                                         \
             }                                                           \
           }                                                             \
         }                                                               \
       }                                                                 \
     }                                                                   \
   }                                                                     \
}

/* -------------------------------------------------------------------  */
/*
*/
/* -------------------------------------------------------------------  */

mitkIpPicDescriptor *mitkIpFuncConv( mitkIpPicDescriptor *pic_old,           
                             mitkIpPicDescriptor *pic_mask, 
                             mitkIpFuncFlagI_t   border )
{

  mitkIpPicDescriptor *pic_new;           /* convolved  image               */
  mitkIpUInt4_t       i;                  /* loopindex                      */
  mitkIpInt4_t        beg[_mitkIpPicNDIM];
  mitkIpInt4_t        end[_mitkIpPicNDIM];
  mitkIpFuncMasc_t    *m;                 /* length of mask and offsets     */
  mitkIpUInt4_t       size[_mitkIpPicNDIM];                                    


  /* check whether data are correct                                     */

  if ( _mitkIpFuncError ( pic_old  ) != mitkIpFuncOK ) return ( mitkIpFuncERROR );
  if ( _mitkIpFuncError ( pic_mask ) != mitkIpFuncOK ) return ( mitkIpFuncERROR );

  if ( ( pic_mask->dim > pic_old->dim ) || ( pic_mask->dim < 1 ) )
    {
       _mitkIpFuncSetErrno ( mitkIpFuncDIMMASC_ERROR );
       return ( mitkIpFuncERROR );
    }
  
  for ( i = 0; i < pic_old->dim; i++ )
    {
       if ( pic_mask->n[i] > pic_old->n[i] )
         {
            _mitkIpFuncSetErrno ( mitkIpFuncUNFIT_ERROR );
            return ( mitkIpFuncERROR );
         }
    }

  /* create a new picture, copy the header, allocate memory             */

  if ( border == mitkIpFuncBorderOld ) 
    {
     /*pic_new = _mitkIpFuncBorder ( pic_old, pic_mask, mitkIpFuncBorderOld );*/
       pic_new = mitkIpPicClone ( pic_old );          
    }
  else if ( border == mitkIpFuncBorderZero )
    { 
   /*  pic_new = _mitkIpFuncBorder ( pic_old, pic_mask, mitkIpFuncBorderZero ); */ 
       pic_new = mitkIpPicCopyHeader ( pic_old, 0 );
       pic_new->data = calloc ( _mitkIpPicElements ( pic_new ), pic_new->bpe/8  );    
    }
  else 
    {
       _mitkIpFuncSetErrno ( mitkIpFuncFLAG_ERROR );
       return ( mitkIpFuncERROR );
    }

  if ( pic_new == NULL )
    {
       _mitkIpFuncSetErrno ( mitkIpFuncMALLOC_ERROR );
       return ( mitkIpFuncERROR );
    }
 
  /* initialize vectors                                                 */

  size[0] = 1;
  for ( i = 1; i < _mitkIpPicNDIM; i++ ) size[i] = size[i-1] * pic_old->n[i-1];

  /* compress mask                                                      */

  m =  _mitkIpFuncCompressM ( pic_mask, pic_old, mitkIpFuncReflect, beg, end );  
  if ( m == NULL ) return ( mitkIpFuncERROR );

  /* macro to convolve  an image (for all data types)                   */

  mitkIpPicFORALL_5 ( CONV, pic_old, size, pic_mask, beg, end, m );                   
  /* free memory                                                        */

  free ( m->off_vekt );
  free ( m->mask_vekt );
  free ( m );

  mitkIpFuncCopyTags(pic_new, pic_old);
  
     

  return pic_new;
}
#endif
