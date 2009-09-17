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
 *  this function performs an edge detection using the Canny operator
 */
  
/** @brief performs an edge detection using the Canny operator.
 *
 *  The Canny-operator needs quite a lot memory. This can cause problems
 *  with large images.
 *
 *  @param pic_old pointer to original image
 *  @param dim_mask dimension of mask
 *  @param len_mask length of filtering mask
 *  @param threshold threshold to decide whether an edge belongs to 
 *                  the image or not
 *  @param border tells how the edge is transformed        
 *
 *  @arg @c mitkIpFuncBorderOld  : original greyvalues        
 *  @arg @c mitkIpFuncBorderZero : edge is set to minimal greyvalue
 *
 *  @return pointer to transformed iamge ( of type mitkIpFloat8_t )
 *
 * AUTHOR & DATE
 
 */

/* include files                                                     */

#include "mitkIpFuncP.h"

mitkIpPicDescriptor *mitkIpFuncCanny( mitkIpPicDescriptor *pic_old,
                              mitkIpUInt4_t       dim_mask,
                              mitkIpUInt4_t       len_mask,
                              mitkIpFloat8_t      threshold,
                              mitkIpFuncFlagI_t   border ) ;

#ifndef DOXYGEN_IGNORE

#ifndef lint
  static char *what = { "@(#)mitkIpFuncCanny\t\tDKFZ (Dept. MBI)\t"__DATE__ };
#endif


/* definition of macros                                              */

#define CANNY0( type_o, pic_old, pic_help1, pic_help2, pic_help3, m )     \
{                                                                         \
  mitkIpPicFORALL_5( CANNY1, pic_help1, pic_old, pic_help2, pic_help3, m, type_o );\
}

#define CANNY1( type_h, pic_help1, pic_old, pic_help2, pic_help3, m, type_o )\
{                                                                          \
  mitkIpUInt4_t     i, j;                /* loop index                      */ \
  mitkIpUInt4_t     len_m;                                                     \
  mitkIpInt4_t      ind[_mitkIpPicNDIM];     /* loop index vector               */ \
  mitkIpUInt4_t     off[_mitkIpPicNDIM];     /* offset vector                   */ \
  mitkIpUInt4_t     off_m;               /* offset in mask                  */ \
  mitkIpFloat8_t    help[_mitkIpPicNDIM];                                          \
  mitkIpFloat8_t    help2;                                                     \
                                                                           \
                                                                           \
  /* transformation of image                                            */ \
                                                                           \
  len_m = m->length / dim_mask;                                            \
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
                  for ( i = 0; i < dim_mask; i++ ) help[i] = 0;            \
                  off_m = 0;                                               \
                  for ( i = 0; i < dim_mask; i++ )                         \
                    for ( j = 0; j < len_m; j++ )                          \
                    {                                                      \
                       help[i] = ( mitkIpFloat8_t )m->mask_vekt[off_m] *       \
                                   (( type_o * ) pic_old->data )           \
                                     [off[0]+m->off_vekt[off_m]] + help[i];\
                       off_m++;                                            \
                    }                                                      \
                  help2 = 0;                                               \
                  for ( i = 0; i < dim_mask; i++ )                         \
                  {                                                        \
                    (( type_h * )pic_help1->data )[off[0]+off_p[i]] =      \
                                                        ( type_h ) help[i];\
                    help2 = help2 + help[i] * help[i];                     \
                  }                                                        \
                  (( type_h * ) pic_help2->data )[off[0]] =                \
                                                         ( type_h ) help2; \
                  off[0]++;                                                \
                }                                                          \
              }                                                            \
            }                                                              \
          }                                                                \
        }                                                                  \
      }                                                                    \
    }                                                                      \
  }                                                                        \
}

#define CANNY2( type_h, pic_help2, pic_help3, pic_help1, m )               \
{                                                                          \
  mitkIpUInt4_t     i, j;                /* loop index                      */ \
  mitkIpUInt4_t     len_m;                                                     \
  mitkIpInt4_t      ind[_mitkIpPicNDIM];     /* loop index vector               */ \
  mitkIpUInt4_t     off[_mitkIpPicNDIM];     /* offset vector                   */ \
  mitkIpUInt4_t     off_m;               /* offset in mask                  */ \
  mitkIpFloat8_t    help[_mitkIpPicNDIM];                                          \
  mitkIpFloat8_t    help2;                                                     \
                                                                           \
                                                                           \
  /* transformation of image                                            */ \
                                                                           \
  len_m = m->length / dim_mask;                                            \
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
                  for ( i = 0; i < dim_mask; i++ ) help[i] = 0;            \
                  off_m = 0;                                               \
                  for ( i = 0; i < dim_mask; i++ )                         \
                    for ( j = 0; j < len_m; j++ )                          \
                    {                                                      \
                       help[i] = help[i] + ( mitkIpFloat8_t )m->mask_vekt[off_m] *\
                                 (( type_h * ) pic_help2->data )           \
                                               [off[0]+m->off_vekt[off_m]];\
                       off_m++;                                            \
                    }                                                      \
                  help2 = 0;                                               \
                  for ( i = 0; i < dim_mask; i++ )                         \
                  {                                                        \
                    help2  = help[i] * (( type_h * )pic_help1->data )      \
                                                [off[0]+ off_p[i]] + help2;\
                  }                                                        \
                  (( type_h * ) pic_help3->data )[off[0]] = ( type_h )help2;\
                  off[0]++;                                                \
                }                                                          \
              }                                                            \
            }                                                              \
          }                                                                \
        }                                                                  \
      }                                                                    \
    }                                                                      \
  }                                                                        \
}

#define CANNY3( type_h, pic_help2, pic_help3, threshold )                  \
{                                                                          \
   mitkIpUInt4_t  i;                    /* loop variable                    */ \
   mitkIpUInt4_t  no_elem;              /* number of pixels                 */ \
                                                                           \
   no_elem =  _mitkIpPicElements ( pic_help2 );                                \
   for ( i = 0; i < no_elem; i++ )                                         \
     (( type_h * )pic_help3->data )[i] =                                   \
        ( ( (( type_h * ) pic_help3->data )[i] != 0 ) &&                   \
          ( (( type_h * ) pic_help2->data )[i] > threshold ) ) ?           \
            1 : 0;                                                         \
} 

/* ----------------------------------------------------------------- */
/*
** mitkIpFuncCanny
*/
/* ----------------------------------------------------------------- */

mitkIpPicDescriptor *mitkIpFuncCanny( mitkIpPicDescriptor *pic_old,
                              mitkIpUInt4_t       dim_mask,
                              mitkIpUInt4_t       len_mask,
                              mitkIpFloat8_t      threshold,
                              mitkIpFuncFlagI_t   border ) 
{
  #include "gradient.h"

  mitkIpPicDescriptor *pic_help1;         /* pointer to image               */
  mitkIpPicDescriptor *pic_help2;         /* pointer to image               */
  mitkIpPicDescriptor *pic_help3;         /* pointer to image               */
  mitkIpPicDescriptor *pic_help4;         /* pointer to image               */
  mitkIpPicDescriptor *pic_new;           /* pointer to image               */
  mitkIpPicDescriptor *pic_mask;          /* sobel mask                     */
  mitkIpUInt4_t       pos;                /* position in m->off_vekt        */
  mitkIpUInt4_t       i, j;               /* loopindex                      */
  mitkIpUInt4_t       off_mask;           /* loopindex                      */
  mitkIpFuncMasc_t    *m;                 /* compressed mask                */
  mitkIpInt4_t        offset;                 
  mitkIpInt4_t        beg[_mitkIpPicNDIM];
  mitkIpInt4_t        end[_mitkIpPicNDIM];
  mitkIpInt4_t        ind[_mitkIpPicNDIM];
  mitkIpUInt4_t       size[_mitkIpPicNDIM];
  mitkIpInt4_t        n[_mitkIpPicNDIM];
  mitkIpUInt4_t       off_p[_mitkIpPicNDIM];  /* offsets to calculate help1     */
  mitkIpUInt4_t       offset_h;           /* offset in pic_help1            */

  /* check whether data are correct                                     */

  if ( _mitkIpFuncError ( pic_old ) != mitkIpFuncOK ) return ( mitkIpFuncERROR );

  if ( ( dim_mask < 1 ) || ( dim_mask > 4 ) )
    {
       _mitkIpFuncSetErrno ( mitkIpFuncDIMMASC_ERROR );
       return ( mitkIpFuncERROR );
    }

  if ( pic_old->dim < dim_mask ) 
    {
       _mitkIpFuncSetErrno ( mitkIpFuncDIMMASC_ERROR );
       return ( mitkIpFuncERROR );
    }

  /* initialisation of pic_mask                                         */

  pic_mask = mitkIpPicNew ();
  if ( pic_mask == NULL ) 
    {
       _mitkIpFuncSetErrno ( mitkIpFuncPICNEW_ERROR );
       return ( mitkIpFuncERROR );
    }

  pic_mask->type = mitkIpPicInt;
  pic_mask->bpe  = 16;
  pic_mask->dim  = dim_mask+1;
  for ( i = 0; i < dim_mask; i++ )
    pic_mask->n[i] = 3;
  pic_mask->n[dim_mask] = dim_mask;

  if ( dim_mask == 4 ) 
    pic_mask->data = mask4;      
  else if ( dim_mask == 3 ) 
    pic_mask->data = mask3;      
  else if ( dim_mask == 2 )
    pic_mask->data = mask2; 

  /* initialisation of vectors                                          */
  
  size [0] = 1;                                                             
  for ( i = 1; i < pic_old->dim; i++ )                                          
    size[i] = size[i-1] * pic_old->n[i-1];                                  
  for ( i = pic_old->dim; i < _mitkIpPicNDIM; i++ )                                 
    size[i] = 0;                                                            

  offset_h = size[pic_old->dim-1] * pic_old->n[pic_old->dim-1];

  for ( i = 0; i < dim_mask; i++ )
    n[i] = pic_mask->n[i] / 2 + 1;
  for ( i = dim_mask; i < _mitkIpPicNDIM; i++ )
    n[i] = 0;

  for ( i = 0; i < dim_mask; i++ )
    {
       end[i] = pic_old->n[i] - pic_mask->n[i] / 2;        
       beg[i] = ( ( pic_mask->n[i] % 2 ) == 1 ) ?
                ( pic_mask->n[i] / 2 ) : ( pic_mask->n[i] / 2 - 1 );        
       off_p[i] = i * offset_h;
    }
  for ( i = dim_mask; i < _mitkIpPicNDIM; i++ )
    beg[i] = 0;

  for ( i = dim_mask; i < pic_old->dim; i++ )        
    end[i] = pic_old->n[i];        
  for ( i = pic_old->dim; i < _mitkIpPicNDIM; i++ )        
    end[i] = beg[i] + 1;

  /* create a new picture, copy the header, allocate memory             */

  pic_help2 = mitkIpPicCopyHeader ( pic_old, NULL );
  if ( pic_help2 == NULL ) 
    {
       pic_mask->data = NULL;
       mitkIpPicFree ( pic_mask);
       _mitkIpFuncSetErrno ( mitkIpFuncPICNEW_ERROR );
       return ( mitkIpFuncERROR );
    }
  if ( pic_old->type == mitkIpPicUInt || pic_old->type == mitkIpPicInt ) 
    {
       pic_help2->type = mitkIpPicFloat;
       pic_help2->bpe  = 64;
    }
  pic_help2->data = calloc ( _mitkIpPicElements ( pic_help2 ), pic_help2->bpe / 8 );
  if ( pic_help2->data == NULL )
    {
       pic_mask->data = NULL;
       mitkIpPicFree ( pic_mask);
       _mitkIpFuncSetErrno ( mitkIpFuncMALLOC_ERROR );
       return ( mitkIpFuncERROR );
    }

  pic_help1 = mitkIpPicCopyHeader ( pic_help2, NULL );
  if ( pic_help1 == NULL )
    {
       pic_help2->data = NULL;
       mitkIpPicFree ( pic_help2);
       pic_mask->data = NULL;
       mitkIpPicFree ( pic_mask);
       _mitkIpFuncSetErrno ( mitkIpFuncPICNEW_ERROR );
       return ( mitkIpFuncERROR );
    }
  pic_help1->dim = pic_help2->dim + 1;
  pic_help1->n[pic_help1->dim-1] = dim_mask;
  pic_help1->data = calloc ( _mitkIpPicElements ( pic_help1 ), pic_help1->bpe / 8 );
  if ( pic_help1->data == NULL ) 
    {
       pic_help2->data = NULL;
       mitkIpPicFree ( pic_help2);
       pic_mask->data = NULL;
       mitkIpPicFree ( pic_mask);
       _mitkIpFuncSetErrno ( mitkIpFuncMALLOC_ERROR );
       return ( mitkIpFuncERROR );
    }
  
  pic_help3 = mitkIpPicCopyHeader ( pic_help2, NULL );
  if ( pic_help3 == NULL ) 
    {
       pic_mask->data = NULL;
       mitkIpPicFree ( pic_mask );
       pic_help1->data = NULL;
       mitkIpPicFree ( pic_help1 );
       pic_help2->data = NULL;
       mitkIpPicFree ( pic_help2 );
       _mitkIpFuncSetErrno ( mitkIpFuncPICNEW_ERROR );
       return ( mitkIpFuncERROR );
    }
  pic_help3->data = calloc ( _mitkIpPicElements ( pic_help3 ), pic_help3->bpe / 8 );
  if ( pic_help3->data == NULL ) 
    {
       pic_mask->data = NULL;
       mitkIpPicFree ( pic_mask );
       pic_help1->data = NULL;
       mitkIpPicFree ( pic_help1 );
       pic_help2->data = NULL;
       mitkIpPicFree ( pic_help2 );
       _mitkIpFuncSetErrno ( mitkIpFuncMALLOC_ERROR );
       return ( mitkIpFuncERROR );
    }

  /* allocate mask-structure                                            */

  m = malloc ( sizeof ( mitkIpFuncMasc_t ) );
  if ( m == NULL ) 
    {
       pic_mask->data = NULL;
       mitkIpPicFree ( pic_mask );
       pic_help1->data = NULL;
       mitkIpPicFree ( pic_help1 );
       pic_help2->data = NULL;
       mitkIpPicFree ( pic_help2 );
       _mitkIpFuncSetErrno ( mitkIpFuncMALLOC_ERROR );
       return ( mitkIpFuncERROR );
    }
  m->off_vekt  = malloc ( _mitkIpPicElements( pic_mask ) * sizeof ( mitkIpInt4_t ) );
  if ( m->off_vekt == NULL ) 
    {
       pic_mask->data = NULL;
       mitkIpPicFree ( pic_mask );
       pic_help1->data = NULL;
       mitkIpPicFree ( pic_help1 );
       pic_help2->data = NULL;
       mitkIpPicFree ( pic_help2 );
       pic_help3->data = NULL;
       mitkIpPicFree ( pic_help3 );
       free ( m );
       _mitkIpFuncSetErrno ( mitkIpFuncMALLOC_ERROR );
       return ( mitkIpFuncERROR );
    }
  m->mask_vekt = malloc ( _mitkIpPicElements( pic_mask ) * sizeof ( mitkIpFloat8_t ) );
  if ( m->mask_vekt == NULL ) 
    {
       pic_mask->data = NULL;
       mitkIpPicFree ( pic_mask );
       pic_help1->data = NULL;
       mitkIpPicFree ( pic_help1 );
       pic_help2->data = NULL;
       mitkIpPicFree ( pic_help2 );
       pic_help3->data = NULL;
       mitkIpPicFree ( pic_help3 );
       free ( m->off_vekt );
       free ( m );
       _mitkIpFuncSetErrno ( mitkIpFuncMALLOC_ERROR );
       return ( mitkIpFuncERROR );
    }

  /* calculate offset vector for the compressed mask                    */

  m->length = 0;

  off_mask = 0;
  for ( i = 0; i < pic_mask->n[dim_mask] ; i++ )
    for ( ind[3] = -1; ind[3] < n[3]; ind[3]++ )
      for ( ind[2] = -1; ind[2] < n[2]; ind[2]++ )
        for ( ind[1] = -1; ind[1] < n[1]; ind[1]++ )
          for ( ind[0] = -1; ind[0] < n[0]; ind[0]++ )
            {
              if ( (( mitkIpInt2_t * )pic_mask->data)[off_mask] != 0 )
                {
                   offset = 0;
                   for ( j = 0; j < dim_mask; j++ )
                     offset = offset + ind[j] * size[j];
                   m->off_vekt[m->length] = offset;
                   m->length++;
                }
              off_mask++;
            }

  /* remove elements that are zero from mask                          */

  pos = 0;
  for ( i = 0; i < m->length; i++ )
    {
      while ( (( mitkIpInt2_t * )pic_mask->data)[pos] == 0 ) pos++;
      m->mask_vekt[i] = ( mitkIpFloat8_t )(( mitkIpInt2_t * )pic_mask->data)[pos];
      pos++;
    }

  /* smooth image using a binomial filter                              */

  pic_new = mitkIpFuncGausF ( pic_old, len_mask, dim_mask, border );

  /* calculate second derivation                                       */

  mitkIpPicFORALL_4 ( CANNY0, pic_new, pic_help1, pic_help2, pic_help3, m );       
  /* project second derivation to gradient                             */

  mitkIpPicFORALL_3 ( CANNY2, pic_help2, pic_help3, pic_help1, m );

  /* zero crossing                                                     */

  pic_help4 = mitkIpFuncZeroCr ( pic_help3 );
 
  mitkIpPicFORALL_2 ( CANNY3, pic_help2, pic_help4, threshold );

  /* free memory                                                       */

  free ( m->off_vekt );
  free ( m->mask_vekt );
  free ( m );
  mitkIpPicFree ( pic_new );
  mitkIpPicFree ( pic_help1 );
  mitkIpPicFree ( pic_help2 );
  mitkIpPicFree ( pic_help3 );
  pic_mask->data = NULL;
  mitkIpPicFree ( pic_mask );

  /* Copy Tags */

  mitkIpFuncCopyTags(pic_help4, pic_old);

  return ( pic_help4 );
}
                               
#endif 
