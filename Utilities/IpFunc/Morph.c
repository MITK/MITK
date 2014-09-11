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
 *  this function performs a morphological operation
 */

/** @brief performs a morphological operation. 
 *
 *  Depending on the parameter kind a erosion or dilation is performed. If the original
 *  image is an binary image each pixel is connected with the surrounding
 *  pixels which are non zero in the kernel by using logical operations 
 *  ( and for erosion, or for dilation ). Greylevel images are transformed
 *  by taking the minimum (erosion) of the surrounding pixels or the      
 *  maximum (dilation).
 *
 *  @param pic_old   pointer to the original image
 *  @param mask         pointer to the kernel
 *  @param kind         tells whether erosion or dilation is performed
 *                 0 (mitkIpERO)  :   erosion
 *                 1 (mitkIpDILA) :   dilation
 *  @param border      tells how the edge is transformed
 *  @arg @c     mitkIpFuncBorderOld  :   original greyvalues
 *  @arg @c     mitkIpFuncBorderZero :   edge is set to zero
 *
 * @return pointer to the transformed image
 *
 * @par Uses
 *  @arg _mitkIpFuncError()     - check the image data          
 *  @arg mitkIpFuncHist()      - calculate the greylevel histogram
 *  @arg  _mitkIpFuncCompressM() - compress filtering mask
 *
 * AUTHOR & DATE
 */

/* include files                                                        */

#include "mitkIpFuncP.h"

mitkIpPicDescriptor *_mitkIpFuncMorph ( mitkIpPicDescriptor *pic_old,
                               mitkIpPicDescriptor *mask,
                               _mitkIpFuncFlagF_t  kind, 
                               mitkIpFuncFlagI_t   border );
#ifndef DOXYGEN_IGNORE

#ifndef lint
  static char *what = { "@(#)mitkIpFuncMorph\t\tDKFZ (Dept. MBI)\t"__DATE__ };
#endif

/* include files                                                        */

#include "mitkIpFuncP.h"

/* definition of macros and constants                                     */

#define ERO_B  &&
#define DILA_B ||
#define ERO_G  <
#define DILA_G >

#define INIT( type, pic_new )                                              \
{                                                                          \
  mitkIpUInt4_t     i;                   /* loop variable                   */ \
  mitkIpUInt4_t     no_elem;                                                   \
                                                                           \
  no_elem = _mitkIpPicElements ( pic_new );                                    \
  for ( i = 0; i < no_elem; i++ )                                          \
    (( type * )pic_new->data )[i] = 1;                                     \
}


#define MORPH_B( type, pic_old, pic_new, m, beg, end, OP )                 \
{                                                                          \
  mitkIpUInt4_t     i;                   /* loop index                      */ \
  mitkIpUInt4_t     offset;              /* offset                          */ \
  mitkIpInt4_t      ind[_mitkIpPicNDIM];     /* loop index vector               */ \
  mitkIpUInt4_t     off[_mitkIpPicNDIM];     /* offset vector                   */ \
                                                                           \
  /* transformation of image                                            */ \
                                                                           \
  for ( i = 0; i < m->length; i++ )                                        \
  {                                                                        \
    offset = m->off_vekt[i];                                               \
    for ( ind[7] = beg[7]; ind[7] < end[7]; ind[7]++ )                     \
    {                                                                      \
      off[7] = ind[7] * size[7];                                           \
      for ( ind[6] = beg[6]; ind[6] < end[6]; ind[6]++ )                   \
      {                                                                    \
        off[6] = ind[6] * size[6] + off[7];                                \
        for ( ind[5] = beg[5]; ind[5] < end[5]; ind[5]++ )                 \
        {                                                                  \
          off[5] = ind[5] * size[5] + off[6];                              \
          for ( ind[4] = beg[4]; ind[4] < end[4]; ind[4]++ )               \
          {                                                                \
            off[4] = ind[4] * size[4] + off[5];                            \
            for ( ind[3] = beg[3]; ind[3] < end[3]; ind[3]++ )             \
            {                                                              \
              off[3] = ind[3] * size[3] + off[4];                          \
              for ( ind[2] = beg[2]; ind[2] < end[2]; ind[2]++ )           \
              {                                                            \
                off[2] = ind[2] * size[2] + off[3];                        \
                for ( ind[1] = beg[1]; ind[1] < end[1]; ind[1]++ )         \
                {                                                          \
                  off[1] = ind[1] * size[1] + off[2];                      \
                  off[0] = beg[0] + off[1];                                \
                  for ( ind[0] = beg[0]; ind[0] < end[0]; ind[0]++ )       \
                  {                                                        \
                    (( type * )pic_new->data )[off[0]] =                   \
                        (( type * )pic_new->data)[off[0]] OP               \
                        (( type * )pic_old->data)[off[0] + offset];        \
                    off[0]++;                                              \
                  }                                                        \
                }                                                          \
              }                                                            \
            }                                                              \
          }                                                                \
        }                                                                  \
      }                                                                    \
    }                                                                      \
  }                                                                        \
}

#define MORPH_G( type, pic_old, pic_new, m, beg, end, OP )                 \
{                                                                          \
  mitkIpUInt4_t     i;                   /* loop index                      */ \
  mitkIpInt4_t      ind[_mitkIpPicNDIM];     /* loop index vector               */ \
  mitkIpUInt4_t     off[_mitkIpPicNDIM];     /* offset vector                   */ \
  type          help, help2;                                               \
                                                                           \
                                                                           \
  /* transformation of image                                            */ \
                                                                           \
  for ( ind[7] = beg[7]; ind[7] < end[7]; ind[7]++ )                       \
  {                                                                        \
    off[7] = ind[7] * size[7];                                             \
    for ( ind[6] = beg[6]; ind[6] < end[6]; ind[6]++ )                     \
    {                                                                      \
      off[6] = ind[6] * size[6] + off[7];                                  \
      for ( ind[5] = beg[5]; ind[5] < end[5]; ind[5]++ )                   \
      {                                                                    \
        off[5] = ind[5] * size[5] + off[6];                                \
        for ( ind[4] = beg[4]; ind[4] < end[4]; ind[4]++ )                 \
        {                                                                  \
          off[4] = ind[4] * size[4] + off[5];                              \
          for ( ind[3] = beg[3]; ind[3] < end[3]; ind[3]++ )               \
          {                                                                \
            off[3] = ind[3] * size[3] + off[4];                            \
            for ( ind[2] = beg[2]; ind[2] < end[2]; ind[2]++ )             \
            {                                                              \
              off[2] = ind[2] * size[2] + off[3];                          \
              for ( ind[1] = beg[1]; ind[1] < end[1]; ind[1]++ )           \
              {                                                            \
                off[1] = ind[1] * size[1] + off[2];                        \
                off[0] = beg[0] + off[1];                                  \
                for ( ind[0] = beg[0]; ind[0] < end[0]; ind[0]++ )         \
                {                                                          \
                  help = (( type* )pic_old->data)[off[0]];                 \
                  for ( i = 0; i < m->length; i++ )                        \
                  {                                                        \
                    help2 = (( type * )pic_old->data)[m->off_vekt[i]+off[0]];\
                    help  = ( help OP help2 ) ? help : help2;              \
                  }                                                        \
                  (( type * )pic_new->data)[off[0]] = help;                \
                  off[0]++;                                                \
                }                                                          \
              }                                                            \
            }                                                              \
          }                                                                \
        }                                                                  \
      }                                                                    \
    }                                                                      \
  }                                                                        \
                                                                           \
}

/* ---------------------------------------------------------------------- */
/* 
** function _mitkIpFuncMorph
*/
/* ---------------------------------------------------------------------- */

mitkIpPicDescriptor *_mitkIpFuncMorph ( mitkIpPicDescriptor *pic_old,
                               mitkIpPicDescriptor *mask,
                               _mitkIpFuncFlagF_t  kind, 
                               mitkIpFuncFlagI_t   border )
{
  mitkIpPicDescriptor *pic_new;        /* pointer to transformed image         */
  mitkIpFuncMasc_t    *m;              /* compressed mask and belonging offsets*/
  mitkIpInt4_t        beg[_mitkIpPicNDIM];
  mitkIpInt4_t        end[_mitkIpPicNDIM];
  mitkIpUInt4_t       size[_mitkIpPicNDIM];    /*                                  */  
  mitkIpUInt4_t       no_gv;           /* number of different greyvalues       */
  mitkIpUInt4_t       i;               /* loop index                           */
  mitkIpUInt4_t       size_hist;       /* number of elements in histogram      */
  mitkIpUInt4_t       *hist;           /* pointer to greyvalue histogram       */
  mitkIpFloat8_t      min, max;        /* extreme greyvalues in image          */

  /* check image data                                                      */

  if ( _mitkIpFuncError ( pic_old ) != mitkIpFuncOK ) return ( mitkIpFuncERROR );
  if ( _mitkIpFuncError ( mask ) != mitkIpFuncOK ) return ( mitkIpFuncERROR );
  if ( mask->dim > pic_old->dim ) 
    {
       _mitkIpFuncSetErrno ( mitkIpFuncDIMMASC_ERROR );
       return ( mitkIpFuncERROR );
    }
 
  for ( i = 0; i < mask->dim; i++ )
    if ( mask->n[i] > pic_old->n[i] ) 
      {
         _mitkIpFuncSetErrno ( mitkIpFuncDATA_ERROR );
         return ( mitkIpFuncERROR );
      }

  pic_new = NULL;

  /* initialisation of vectors                                             */  
                                                                            
  size [0] = 1;                                                             
  for ( i = 1; i < _mitkIpPicNDIM; i++ )                                        
    size[i] = size[i-1] * pic_old->n[i-1];                                  
  size[pic_old->dim] = 0;                                                   

  /* compress filtering mask                                               */

  m = _mitkIpFuncCompressM ( mask, pic_old, mitkIpFuncNoReflect, beg, end );
  if ( m == NULL ) 
    {
       free ( hist );
       mitkIpPicFree ( pic_new );
       return ( mitkIpFuncERROR );
    }

  /* allocate and initialize pic_new                                       */
 
  if ( border == mitkIpFuncBorderOld ) 
    pic_new = mitkIpPicClone ( pic_old );
  else if ( border == mitkIpFuncBorderZero )
    { 
       pic_new = mitkIpPicCopyHeader ( pic_old, NULL );
       if ( pic_new == NULL )
         {
            _mitkIpFuncSetErrno ( mitkIpFuncPICNEW_ERROR );
            return ( mitkIpFuncERROR );
         }
       pic_new->data = calloc ( _mitkIpPicElements ( pic_new ), pic_new->bpe / 8 );
       if ( pic_new->data == NULL )
         {
            mitkIpPicFree ( pic_new );
            _mitkIpFuncSetErrno ( mitkIpFuncPICNEW_ERROR );
            return ( mitkIpFuncERROR );
         }

       if ( kind == mitkIpFuncEroF ) 
         {
            mitkIpPicFORALL ( INIT, pic_new );                                       
         }
    }
  else 
    {
       _mitkIpFuncSetErrno ( mitkIpFuncFLAG_ERROR );
       return ( mitkIpFuncERROR );
    }
 
  if ( pic_new == NULL )  
    {  
       _mitkIpFuncSetErrno ( mitkIpFuncPICNEW_ERROR );
       return ( mitkIpFuncERROR );
    }

  /* check whether binary or greylevel image                               */
  
  if ( mitkIpFuncExtr ( pic_old, &min, &max ) != mitkIpFuncOK ) return ( mitkIpFuncERROR );
  mitkIpFuncHist ( pic_old, min, max, &hist, &size_hist );
  if ( hist == NULL ) 
    {
       mitkIpPicFree ( pic_new );
       return ( mitkIpFuncERROR );
    }

  i     = 0;
  no_gv = 0;
  while ( i <= ( mitkIpUInt4_t ) ( fabs ( min ) + max ) && no_gv < 3 )
    {
       if ( hist [i] != 0 ) no_gv++;
       i++;
    }
  

  /* transform image (depending on kind and no_gv)                         */

  if ( kind == mitkIpFuncEroF )                             /* erosion         */
    {
       if ( no_gv == 2 )                                /* binary image    */
         {
            mitkIpPicFORALL_5 ( MORPH_B, pic_old, pic_new, m, beg, end, ERO_B );
         }
       else if ( no_gv > 2 )                            /* greylevel image */
         {
            mitkIpPicFORALL_5 ( MORPH_G, pic_old, pic_new, m, beg, end, ERO_G );
         }
       else 
         {
            free ( hist );
            free ( m->off_vekt );
            free ( m->mask_vekt );
            free ( m );
            /*_mitkIpFuncSetErrno ( mitkIpFuncDATA_ERROR );*/
            return ( pic_new );
         }
    }
  else if ( kind == mitkIpFuncDilaF )                        /* dilation        */
    {
       if ( no_gv == 2 )                                /* binary image    */
         {
            mitkIpPicFORALL_5 ( MORPH_B, pic_old, pic_new, m, beg, end, DILA_B );
         }
       else if ( no_gv > 2 )                            /* greylevel image */
         {
            mitkIpPicFORALL_5 ( MORPH_G, pic_old, pic_new, m, beg, end, DILA_G );
         }
       else 
         {
            free ( hist );
            free ( m->off_vekt );
            free ( m->mask_vekt );
            free ( m );
            /*_mitkIpFuncSetErrno ( mitkIpFuncDATA_ERROR );*/
            return ( pic_new );
         }
    }
  else
    {
       mitkIpPicFree ( pic_new );
       free ( hist );
       free ( m->off_vekt );
       free ( m->mask_vekt );
       free ( m );
       _mitkIpFuncSetErrno ( mitkIpFuncFLAG_ERROR );
       return ( mitkIpFuncERROR );
    }

  /* free memory                                                           */
  free ( m->off_vekt );
  free ( m->mask_vekt );
  free ( m );
  free ( hist );

  return ( pic_new );
}
#endif
