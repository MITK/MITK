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
 *  this function performs an edge detection by the Roberts gradient
 */

/** @brief performs an edge detection by the Roberts gradient
 *
 *  This gradient uses a 2x2 or 2x2x2 mask and calculates the maximum     
 *  of the difference of all diagonal elements
 *
 *  @param pic_old   pointer to original image
 *  @param dim_mask  dimension of mask (2 or 3)
 *  @param border    tells how the edge is handled
 *  @arg @c         ipFuncBorderOld  : keep greyvalues
 *  @arg @c         ipFuncBorderZero : set greyvalues to zero
 *
 * @return pointer to transformed image
 *
 * AUTHOR & DATE
 */

/* include files                                                         */

#include "ipFuncP.h"

ipPicDescriptor *ipFuncRoberts ( ipPicDescriptor *pic_old,
                                 ipUInt4_t       dim_mask,  
                                 ipFuncFlagI_t   border );

#ifndef DOXYGEN_IGNORE

#ifndef lint
  static char *what = { "@(#)????????\t\tDKFZ (Dept. MBI)\t"__DATE__ };
#endif



/* definition of macros                                                  */

#define ROBERTS( type, pic, msize, n, off, size )                         \
{                                                                         \
  ipUInt4_t       i;                /* loop index                      */ \
  ipUInt4_t       end;              /*                                 */ \
  ipUInt4_t       off_imag;         /* offset of pixels                */ \
  ipUInt4_t       ind[_mitkIpPicNDIM];  /* loop index vector               */ \
  type            help, help2;      /* used to calculate new greyvalue */ \
                                                                          \
  end = msize / 2;                                                        \
  msize = msize - 1;                                                      \
  for ( ind[7] = 0; ind[7] < n[7]; ind[7]++ )                             \
    for ( ind[6] = 0; ind[6] < n[6]; ind[6]++ )                           \
      for ( ind[5] = 0; ind[5] < n[5]; ind[5]++ )                         \
        for ( ind[4] = 0; ind[4] < n[4]; ind[4]++ )                       \
          for ( ind[3] = 0; ind[3] < n[3]; ind[3]++ )                     \
            for ( ind[2] = 0; ind[2] < n[2]; ind[2]++ )                   \
              for ( ind[1] = 0; ind[1] < n[1]; ind[1]++ )                 \
                for ( ind[0] = 0; ind[0] < n[0]; ind[0]++ )               \
                  {                                                       \
                    /* offset of actual pixel                          */ \
                                                                          \
                    off_imag = 0;                                         \
                    for ( i = 0; i < pic->dim; i++ )                      \
                      off_imag = off_imag + ind[i] * size[i];             \
                                                                          \
                    /* find out maximum of all diagonal differences    */ \
                                                                          \
                    help = 0;                                             \
                    for ( i = 0; i < end; i++ )                           \
                      {                                                   \
                        help2 = ( type ) fabs ( ( ipFloat8_t ) (          \
                         ((type *)pic->data)[off_imag+off[i]] -           \
                         ((type *)pic->data)[off_imag+off[msize-i]]));    \
                        help = ( help > help2 ) ? help : help2;           \
                      }                                                   \
                    (( type * )pic_new->data )[off_imag] = help;          \
                  }                                                       \
                                                                          \
}                                                                         \

/* --------------------------------------------------------------------- */
/* 
** function ipFuncRoberts                                               
*/
/* --------------------------------------------------------------------- */
 
ipPicDescriptor *ipFuncRoberts ( ipPicDescriptor *pic_old,
                                 ipUInt4_t       dim_mask,  
                                 ipFuncFlagI_t   border )
{
  ipPicDescriptor *pic_new;          /* pointer to new image structure   */
  ipInt4_t        *off_vekt;         /* offsets for the image pixels that*/
                                     /* are needed for the operation     */
  ipInt4_t        offset;            /* used to calculate off_vekt       */
  ipUInt4_t       off_mask;          /* offset of mask elements          */
  ipUInt4_t       mask_size;         /* number of elements in mask       */
  ipUInt4_t       ind[_mitkIpPicNDIM];   /* loop index vector                */
  ipUInt4_t       size[_mitkIpPicNDIM];  /*                                  */
  ipUInt4_t       n[_mitkIpPicNDIM];     /* size of each dimension           */
  ipUInt4_t       i;                 /* loop index                       */

  /* check data                                                          */

  if ( _ipFuncError ( pic_old ) != mitkIpFuncOK ) return ( mitkIpFuncERROR );
  if ( ( pic_old->dim < dim_mask ) || ( dim_mask < 1 ) )
    {
       _ipFuncSetErrno ( mitkIpFuncDIMMASC_ERROR );
       return ( mitkIpFuncERROR );
    }

  /* initialisation of vectors                                           */

  for ( i = 0; i < dim_mask; i++ )
    n[i] = 2;
 
  for ( i = dim_mask; i < _mitkIpPicNDIM; i++ )
    n[i] = 1;

  size[0] = 1;
  ind[0]  = 0;
  for ( i = 1; i < _mitkIpPicNDIM; i++ )
    {
      ind[i]  = 0;
      size[i] = size[i-1] * pic_old->n[i-1];
    }

  /* calculate offset vector                                             */

  mask_size = ( ipUInt4_t ) pow ( 2., ( ipFloat8_t ) dim_mask );
  off_vekt  = calloc ( mask_size, sizeof( ipInt4_t ) );
  off_mask  = 0;
  if ( off_vekt == NULL ) 
    {
       _ipFuncSetErrno ( mitkIpFuncMALLOC_ERROR );
       return ( mitkIpFuncERROR );
    }

  switch ( dim_mask )
   {
     case 8:
       for ( ind[7] = 0; ind[7] < n[7]; ind[7]++ )
     case 7:
         for ( ind[6] = 0; ind[6] < n[6]; ind[6]++ )
     case 6:
           for ( ind[5] = 0; ind[5] < n[5]; ind[5]++ )
     case 5:
             for ( ind[4] = 0; ind[4] < n[4]; ind[4]++ )
     case 4:
                 for ( ind[3] = 0; ind[3] < n[3]; ind[3]++ )
     case 3:
                   for ( ind[2] = 0; ind[2] < n[2]; ind[2]++ )
     case 2:
                     for ( ind[1] = 0; ind[1] < n[1]; ind[1]++ )
     case 1:
                       for ( ind[0] = 0; ind[0] < n[0]; ind[0]++ )
                         {
                           offset = 0;
                           for ( i = 0; i < dim_mask; i++ )
                             offset = offset + ind[i] * size[i];
                           off_vekt[off_mask] = offset;
                           off_mask++;
                         }
   }

  /* allocate new image                                                  */
     
  if ( border == ipFuncBorderOld )
    pic_new = ipPicClone ( pic_old );
  else if ( border == ipFuncBorderZero )
    {
       pic_new = ipPicCopyHeader ( pic_old, 0 );
       pic_new->data = malloc ( _ipPicSize ( pic_new ) );
    }
  else 
    {
       free ( off_vekt );
       _ipFuncSetErrno ( mitkIpFuncFLAG_ERROR );
       return ( mitkIpFuncERROR );
    }

  if ( pic_new == NULL )
    {
       free ( off_vekt );
       _ipFuncSetErrno ( mitkIpFuncMALLOC_ERROR );
       return ( mitkIpFuncERROR );
    }

  if ( pic_new->data == NULL )
    {
       free ( off_vekt );
       ipPicFree ( pic_new ); 
       _ipFuncSetErrno ( mitkIpFuncMALLOC_ERROR );
       return ( mitkIpFuncERROR );
    }

  /* macro                                                               */

  for ( i = 0; i < pic_old->dim; i++ )
    n[i] = pic_old->n[i] - 1;
 
  for ( i = pic_old->dim; i < _mitkIpPicNDIM; i++ )
    n[i] = 1;
 
  mitkIpPicFORALL_4 ( ROBERTS, pic_old, mask_size, n, off_vekt, size );

  free ( off_vekt );

  /* Copy Tags */

  ipFuncCopyTags(pic_new, pic_old);
  
  

  return ( pic_new );
}

#endif

