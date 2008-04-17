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
 *  this function performs an absolute gradient to extract 
 *  edges of an image
 */

/** @brief performs an absolute gradient to extract 
 *  edges of an image
 *
 *  @param pic_old    pointer to original image
 *  @param dim_mask    dimension of mask
 *  @param border   tells how the edge is transformed        
 *                     ipFuncBorderOld:  original greyvalues        
 *                     ipFuncBorderZero: edge is set to minimal greyvalue
 *
 *  @return  pointer to transformed iamge 
 *
 * AUTHOR & DATE
 */

/* include files                                                     */

#include "ipFuncP.h"

ipPicDescriptor *ipFuncGrad ( ipPicDescriptor *pic_old,
                              ipUInt4_t       dim_mask,
                              ipFuncFlagI_t   border ) ;
#ifndef DOXYGEN_IGNORE

#ifndef lint
  static char *what = { "@(#)ipFuncGrad\t\tDKFZ (Dept. MBI)\t"__DATE__ };
#endif


/* definition of macros                                              */

#define GRAD( type, pic, pic_new, pic_mask, m, beg, end, dim_mask )        \
{                                                                          \
  ipUInt4_t     i, j;                /* loop index                      */ \
  ipUInt4_t     size[_mitkIpPicNDIM];    /*                                 */ \
  ipInt4_t      ind[_mitkIpPicNDIM];     /* loop index vector               */ \
  ipUInt4_t     off[_mitkIpPicNDIM];     /* offset vector                   */ \
  ipUInt4_t     begin[_mitkIpPicNDIM];                                         \
  ipFloat8_t    help, help2;                                               \
                                                                           \
  /* initialisation of vectors                                          */ \
                                                                           \
  size [0] = 1;                                                            \
  for ( i = 1; i < pic->dim; i++ )                                         \
    size[i] = size[i-1] * pic_old->n[i-1];                                 \
  for ( i = pic->dim; i < _mitkIpPicNDIM; i++ )                                \
    size[i] = 0;                                                           \
                                                                           \
  begin[0] =  0;                                                           \
  for ( i = 1; i <= dim_mask; i++ )                                        \
    begin[i] = begin[i-1] + m->length / dim_mask;                          \
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
                  help2 = 0;                                               \
                  for ( j = 0; j < dim_mask; j++ )                         \
                  {                                                        \
                    help = 0;                                              \
                    for ( i = begin[j]; i < begin[j+1]; i++ )              \
                    {                                                      \
                      help = help + m->mask_vekt[i] / sum  * ( ipFloat8_t )\
                             (( type * )pic->data )[off[0]+m->off_vekt[i]];\
                    }                                                      \
                    help2 = help2 + help * help;                           \
                  }                                                        \
                  (( type * )pic_new->data)[off[0]] =                      \
                             ( type )  sqrt ( help2 / dim_mask ) / 2.;     \
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

/* ----------------------------------------------------------------- */
/*
** ipFuncGrad
*/
/* ----------------------------------------------------------------- */

ipPicDescriptor *ipFuncGrad ( ipPicDescriptor *pic_old,
                              ipUInt4_t       dim_mask,
                              ipFuncFlagI_t   border ) 
{
  #include "gradient.h"

  ipPicDescriptor *pic_new;           /* pointer to new image           */
  ipPicDescriptor *pic_mask;          /* sobel mask                     */
  ipUInt4_t       pos;                /* position in m->off_vekt        */
  ipUInt4_t       i, j;               /* loopindex                      */
  ipUInt4_t       off_mask;           /* loopindex                      */
  ipFuncMasc_t    *m;                 /* compressed mask                */
  ipInt4_t        offset;                 
  ipInt4_t        beg[_mitkIpPicNDIM];
  ipInt4_t        end[_mitkIpPicNDIM];
  ipInt4_t        ind[_mitkIpPicNDIM];
  ipUInt4_t       size[_mitkIpPicNDIM];
  ipInt4_t        n[_mitkIpPicNDIM];
  ipFloat8_t      sum;

  /* check whether data are correct                                     */

  if ( _ipFuncError ( pic_old ) != mitkIpFuncOK ) return ( mitkIpFuncERROR );
  if ( ( dim_mask > 4 ) || ( dim_mask < 1 ) )
    {
       _ipFuncSetErrno ( mitkIpFuncDIMMASC_ERROR );
       return ( mitkIpFuncERROR );
    }
  if ( pic_old->dim < dim_mask ) 
    {
       _ipFuncSetErrno ( mitkIpFuncDIMMASC_ERROR );
       return ( mitkIpFuncERROR );
    }

  /* create a new picture, copy the header, allocate memory             */

  if ( border == ipFuncBorderOld ) 
    pic_new = ipPicClone ( pic_old );
  else if ( border == ipFuncBorderZero )
    { 
       pic_new = ipPicCopyHeader ( pic_old, 0 );
       pic_new->data = calloc ( _ipPicElements ( pic_new ), pic_new->bpe/8  );
    }
  else 
    {
       _ipFuncSetErrno ( mitkIpFuncFLAG_ERROR );
       return ( mitkIpFuncERROR );
    }

  if ( pic_new == NULL )
    {
       _ipFuncSetErrno ( mitkIpFuncPICNEW_ERROR );
       return ( mitkIpFuncERROR );
    }

  if ( pic_new->data == NULL )
    {
       _ipFuncSetErrno ( mitkIpFuncMALLOC_ERROR );
       return ( mitkIpFuncERROR );
    }

  /* initialisation of pic_mask                                         */

  pic_mask = ipPicNew ();
  if ( pic_mask == NULL ) 
    {
       ipPicFree ( pic_new );
       _ipFuncSetErrno ( mitkIpFuncPICNEW_ERROR );
       return ( mitkIpFuncERROR );
    }

  pic_mask->type = ipPicInt;
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
  else 
    {
       pic_mask->data = NULL;
       ipPicFree ( pic_new );
       _ipFuncSetErrno ( mitkIpFuncDIM_ERROR );
       return ( mitkIpFuncERROR );
    }

  /* initialisation of vectors                                          */


  for ( i = 0; i < dim_mask; i++ )
    n[i] = pic_mask->n[i] / 2 + 1;
  for ( i = dim_mask; i < _mitkIpPicNDIM; i++ )
    n[i] = 0;

  for ( i = 0; i < dim_mask; i++ )
    {
       end[i] = pic_old->n[i] - pic_mask->n[i] / 2;        
       beg[i] = ( ( pic_mask->n[i] % 2 ) == 1 ) ?
                ( pic_mask->n[i] / 2 ) : ( pic_mask->n[i] / 2 - 1 );        
    }
  for ( i = dim_mask; i < _mitkIpPicNDIM; i++ )
    beg[i] = 0;

  for ( i = dim_mask; i < pic_old->dim; i++ )        
    end[i] = pic_old->n[i];        
  for ( i = pic_old->dim; i < _mitkIpPicNDIM; i++ )        
    end[i] = beg[i] + 1;

  size [0] = 1;                                                             
  for ( i = 1; i < pic_old->dim; i++ )                                        
    size[i] = size[i-1] * pic_old->n[i-1];                                  


  /* allocate mask-structure                                            */

  m = malloc ( sizeof ( ipFuncMasc_t ) );
  if ( m == NULL ) 
    {
       ipPicFree ( pic_new );
       pic_mask->data = NULL;
       ipPicFree ( pic_mask );
       _ipFuncSetErrno ( mitkIpFuncMALLOC_ERROR );
       return ( mitkIpFuncERROR );
    }
  m->off_vekt  = malloc ( _ipPicElements( pic_mask ) * sizeof ( ipInt4_t ) );
  if ( m->off_vekt == NULL ) 
    {
       ipPicFree ( pic_new );
       pic_mask->data = NULL;
       ipPicFree ( pic_mask );
       free ( m );
       _ipFuncSetErrno ( mitkIpFuncMALLOC_ERROR );
       return ( mitkIpFuncERROR );
    }
  m->mask_vekt = malloc ( _ipPicElements( pic_mask ) * sizeof ( ipFloat8_t ) );
  if ( m->mask_vekt == NULL )
    {
       ipPicFree ( pic_new );
       pic_mask->data = NULL;
       ipPicFree ( pic_mask );
       free ( m->off_vekt );
       free ( m );
       _ipFuncSetErrno ( mitkIpFuncMALLOC_ERROR );
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
              if ( (( ipInt2_t * )pic_mask->data)[off_mask] != 0 )
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
  sum = 0;
  for ( i = 0; i < m->length; i++ )
    {
      while ( (( ipInt2_t * )pic_mask->data)[pos] == 0 ) pos++;
      m->mask_vekt[i] = ( ipFloat8_t )(( ipInt2_t * )pic_mask->data)[pos];
      sum = sum + fabs ( m->mask_vekt[i] );
      pos++;
    }
  
  sum = sum /  ( 2 * dim_mask );

  mitkIpPicFORALL_6 ( GRAD, pic_old, pic_new, pic_mask, m, beg, end, dim_mask );

  free ( m->off_vekt );
  free ( m->mask_vekt );
  free ( m );
  pic_mask->data = NULL;
  ipPicFree ( pic_mask );

  /* Copy Tags */

  ipFuncCopyTags(pic_new, pic_old);
  
  

  return ( pic_new );
}
#endif                              
 
