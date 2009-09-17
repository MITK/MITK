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
 *  this function performs a filtering of the image. The new greyvalue of
 *  the image is calculated by taking the greyvalue belonging to the                
 *  specified range after having sorted the greyvalues included by the mask
 */

/** @brief performs a filtering of the image. The new greyvalue of
 *  the image is calculated by taking the greyvalue belonging to the                
 *  specified range after having sorted the greyvalues included by the mask
 *
 *  @param pic_old    pointer to original image
 *  @param rank       rank (number of pixel which is taken after having sorted 
 *               the pixels under the mask)
 *                 rank = 0 : Median 
 *  @param mask_dim   dimension of mask
 *  @param mask_size  number of pixel in each dimension of the mask
 *  @param border  tells how the edge is handled
 *  @arg @c      mitkIpFuncBorderOld  :  greyvalue of pic_old is kept
 *  @arg @c      mitkIpFuncBorderZero :  greyvalue of edge pixels is set to zero
 *
 * @return  pointer to transformed image
 *
 * AUTHOR & DATE
 */

/* include files                                                                  */

#include "mitkIpFuncP.h"

mitkIpPicDescriptor *mitkIpFuncRank ( mitkIpPicDescriptor *pic_old,
                              mitkIpUInt4_t       rank,
                              mitkIpUInt4_t       mask_dim,
                              mitkIpUInt4_t       mask_size,
                              mitkIpFuncFlagI_t   border );
#ifndef DOXYGEN_IGNORE

#ifndef lint
  static char *what = { "@(#)????????\t\tDKFZ (Dept. MBI)\t"__DATE__ };
#endif

/* include files                                                                  */

#include "mitkIpFuncP.h"

/* definition of macros                                                           */

#define INDEX0( typ, pic ) ( mitkIpUInt4_t ) ( factor * (( typ * ) pic->data )         \
                                  [off_vekt[i] + off[0]] + help )        
#define INDEX2( typ, pic ) ( mitkIpUInt4_t ) ( factor * (( typ * ) pic->data )         \
                                  [off_vekt[i] + off[0]] + help )      
#define INDEX3( typ, pic ) ( mitkIpUInt4_t ) ( factor * (( typ * ) pic->data )         \
                                  [off[0] - off_vekt[i]] + help )        
#define RANK( typ, pic, begin, no_elem, size, rank )                               \
{                                                                                  \
  mitkIpUInt4_t      n[_mitkIpPicNDIM];    /* size of each dimension                    */ \
  mitkIpUInt4_t      i;                /* loop index                                */ \
  mitkIpUInt4_t      *hist;            /* greyvalue histogram                       */ \
  mitkIpUInt4_t      sum;              /* used to calculate rank                    */ \
  mitkIpUInt4_t      end;              /* number of elements that change            */ \
  mitkIpUInt4_t      ind[_mitkIpPicNDIM];  /* loop index vector                         */ \
  mitkIpUInt4_t      beg;              /* beginning of the loops                    */ \
  mitkIpUInt4_t      off[_mitkIpPicNDIM];  /* used to calculate offset of image pixels  */ \
  mitkIpFloat8_t     min_gv, max_gv;   /* min./max. possible greyvalues             */ \
  mitkIpFloat8_t     help, factor;                                                     \
  mitkIpUInt4_t      hist_size;                                                        \
                                                                                   \
  /* initialize vectors                                                         */ \
                                                                                   \
  beg    = ( mitkIpUInt4_t ) begin;                                                    \
  for ( i = 0; i < _mitkIpPicNDIM; i++ ) off[i] = 0;                                   \
  for ( i = 0; i < pic->dim; i++ ) n[i] = pic->n[i] - beg;                         \
  for ( i = pic->dim; i < _mitkIpPicNDIM; i++ )                                        \
    {                                                                              \
       n[i]    = 1 + beg;                                                          \
       size[i] = 0;                                                                \
    }                                                                              \
                                                                                   \
  /* calculate max. and min. possible greyvalues                                */ \
                                                                                   \
  if ( mitkIpFuncExtr( pic, &min_gv, &max_gv ) == mitkIpFuncERROR )                            \
    {                                                                              \
       free ( off_vekt );                                                          \
       mitkIpPicFree ( pic_new );                                                      \
       return ( mitkIpFuncERROR );                                                         \
    }                                                                              \
                                                                                   \
  /* allocate memory for histogram and calculate variables which are needed for */ \
  /* allocation                                                                 */ \
                                                                                   \
  end    = no_elem / mask_size;                                                    \
  factor = ( pic->type == mitkIpPicFloat ) ? 1000. : 1.;                               \
  help   = fabs ( min_gv );                                                        \
  hist_size = ( mitkIpUInt4_t ) ( factor * ( fabs ( max_gv ) + help ) );               \
  help   = help * factor;                                                          \
/*hist   = calloc ( hist_size+1, sizeof ( mitkIpUInt4_t ) );                           \
  if ( hist == NULL )                                                              \
    {                                                                              \
       free ( off_vekt );                                                          \
       mitkIpPicFree ( pic_new );                                                      \
       _mitkIpFuncSetErrno ( mitkIpFuncMALLOC_ERROR );                                      \
       return ( mitkIpFuncERROR );                                                         \
    }                                                                              \
 */                                                                                \
  /* transform pixels                                                           */ \
                                                                                   \
  for ( ind[7] =  beg ; ind[7] < n[7]; ind[7]++ )                                  \
  {                                                                                \
    off[7] = size[7] * ind[7];                                                     \
    for ( ind[6] =  beg ; ind[6] < n[6]; ind[6]++ )                                \
    {                                                                              \
      off[6] = size[6] * ind[6] + off[7];                                          \
      for ( ind[5] =  beg ; ind[5] < n[5]; ind[5]++ )                              \
      {                                                                            \
        off[5] = size[5] * ind[5] + off[6];                                        \
        for ( ind[4] =  beg ; ind[4] < n[4]; ind[4]++ )                            \
        {                                                                          \
          off[4] = size[4] * ind[4] + off[5];                                      \
          for ( ind[3] =  beg ; ind[3] < n[3]; ind[3]++ )                          \
          {                                                                        \
            off[3] = size[3] * ind[3] + off[4];                                    \
            for ( ind[2] =  beg ; ind[2] < n[2]; ind[2]++ )                        \
            {                                                                      \
              off[2] = size[2] * ind[2] + off[3];                                  \
              for ( ind[1] =  beg ; ind[1] < n[1]; ind[1]++ )                      \
              {                                                                    \
                /* calculate histogram with pixels under mask                   */ \
                /* (complete histogram for new rows)                            */ \
                                                                                   \
                off[1] = size[1] * ind[1] + off[2];                                \
                off[0] = off[1] + beg;                                             \
              /*memset ( hist, 0, hist_size );*/                                   \
  hist   = calloc ( hist_size+1, sizeof ( mitkIpUInt4_t ) );                           \
                for ( i = 0; i < no_elem; i++ ) hist[INDEX0 ( typ, pic )]++;       \
                for ( i = 0, sum = 0; sum < rank; i++ ) sum = sum + hist[i];       \
                (( typ * ) pic_new->data )[off[0]] = ( typ )                       \
                                       ( ( ( mitkIpFloat8_t ) i - help ) / factor );   \
                for ( ind[0] =  beg+1; ind[0] < n[0]; ind[0]++ )                   \
                {                                                                  \
                  /* change histogram for each column                           */ \
                                                                                   \
                  for ( i = 0; i < end; i++ ) hist[INDEX2 ( typ, pic )]--;         \
                  off[0]++;                                                        \
                  for ( i = 0; i < end; i++ ) hist[INDEX3 ( typ, pic )]++;         \
                  for ( i = 0, sum = 0; sum < rank; i++ ) sum = sum + hist[i];     \
                  (( typ * ) pic_new->data )[off[0]] = ( typ )                     \
                                       ( ( ( mitkIpFloat8_t ) i - help -1.) / factor );\
                }                                                                  \
                free ( hist );\
              }                                                                    \
            }                                                                      \
          }                                                                        \
        }                                                                          \
      }                                                                            \
    }                                                                              \
  }                                                                                \
/*free ( hist );  */                                                               \
}       


/* ------------------------------------------------------------------------------ */
/*
** function mitkIpFuncRank   
*/
/* ------------------------------------------------------------------------------ */
 
mitkIpPicDescriptor *mitkIpFuncRank ( mitkIpPicDescriptor *pic_old,
                              mitkIpUInt4_t       rank,
                              mitkIpUInt4_t       mask_dim,
                              mitkIpUInt4_t       mask_size,
                              mitkIpFuncFlagI_t   border )
{
  
  mitkIpPicDescriptor *pic_new;            /* pointer to transformed image            */
  mitkIpInt4_t       i;                    /* loop index                              */
  mitkIpInt4_t       offset;               /* offset of image                         */
  mitkIpInt4_t       ind[_mitkIpPicNDIM];      /* loop index vector                       */
  mitkIpInt4_t       *off_vekt;            /* pointer to offset vector                */
  mitkIpInt4_t       begin;                /* 0.5 * mask_size                         */
  mitkIpUInt4_t      size[_mitkIpPicNDIM];     /*                                         */
  mitkIpInt4_t       n[_mitkIpPicNDIM];        /* size of each dimension                  */
  mitkIpUInt4_t      no_elem;              /* number of elements in mask              */
  mitkIpUInt4_t      len;                  /* length of offset vector                 */

  /* calculate number of elements in mask                                         */

  no_elem = mask_size;
  for ( i = 1; i < mask_dim; i++ )
    no_elem = no_elem * mask_size;

  /* check whether data are correct                                               */

  if ( _mitkIpFuncError ( pic_old ) != mitkIpFuncOK ) return ( mitkIpFuncERROR );
  if ( mask_dim < 1 || mask_dim > pic_old->dim )  
    { 
       _mitkIpFuncSetErrno ( mitkIpFuncDIMMASC_ERROR );
       return ( mitkIpFuncERROR );
    }
  if ( rank > no_elem ) 
    { 
       _mitkIpFuncSetErrno ( mitkIpFuncDATA_ERROR );
       return ( mitkIpFuncERROR );
    }
  if ( mask_size % 2 != 1 ) 
    { 
       _mitkIpFuncSetErrno ( mitkIpFuncSIZE_ERROR );
       return ( mitkIpFuncERROR );
    }

  /* initialize vectors and variables                                             */

  size[0] = 1;
  for ( i = 1; i < _mitkIpPicNDIM; i++ )
    size[i] = size[i-1] * pic_old->n[i-1];

  len = 0;
  begin = mask_size / 2;

  for ( i = 0; i < mask_dim; i++ )
    n[i] = begin + 1;
  for ( i = mask_dim; i < _mitkIpPicNDIM; i++ )
    n[i] = 1 - begin;
  
  /* allocate image structure                                                     */

  if ( border == mitkIpFuncBorderOld )
    pic_new = mitkIpPicClone ( pic_old );
  else if ( border == mitkIpFuncBorderZero )
    {
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
       _mitkIpFuncSetErrno ( mitkIpFuncPICNEW_ERROR );
       return ( mitkIpFuncERROR );
    }

  /* allocate offset vector                                                       */

  off_vekt = malloc ( no_elem * sizeof ( mitkIpUInt4_t ) );
  if ( off_vekt == NULL )
    {
       _mitkIpFuncSetErrno ( mitkIpFuncMALLOC_ERROR );
       return ( mitkIpFuncERROR );
    }

  /* calculate offset vector                                                      */

  for ( ind[0] = -begin; ind[0] < n[0]; ind[0]++ )
    for ( ind[7] = -begin; ind[7] < n[7]; ind[7]++ )
      for ( ind[6] = -begin; ind[6] < n[6]; ind[6]++ )
        for ( ind[5] = -begin; ind[5] < n[5]; ind[5]++ )
          for ( ind[4] = -begin; ind[4] < n[4]; ind[4]++ )
            for ( ind[3] = -begin; ind[3] < n[3]; ind[3]++ )
              for ( ind[2] = -begin; ind[2] < n[2]; ind[2]++ )
                for ( ind[1] = -begin; ind[1] < n[1]; ind[1]++ )
                  { 
                     offset = 0;
                     for ( i = 0; i < pic_old->dim; i++ )
                       offset = offset + ind[i] * size[i]; 
                     off_vekt[len] = offset;
                     len++;
                  }
  if ( rank == 0 ) rank = no_elem / 2 + 1;
  mitkIpPicFORALL_4 ( RANK, pic_old, begin, no_elem, size, rank );
      
  free ( off_vekt );

  /* Copy Tags */

  mitkIpFuncCopyTags(pic_new, pic_old);
  
  
                        
  return ( pic_new );
}
#endif
