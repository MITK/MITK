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
 *  this function smoothes an image using a filtering mask which consists 
 *  of elements calculated from a Gausian distribution
 */

/**  this function smoothes an image using a filtering mask which consists 
 *  of elements calculated from a Gausian distribution
 *
 *  @param pic_old   pointer to original image  
 *  @param len_mask   number of mask elements for each dimension
 *  @param dim_mask   dimension of mask
 *  @param border    handling of the edge
 *  @arg @c   ipFuncBorderZero :  set edge pixels to zero
 *  @arg @c   ipFuncBorderOld  :  keep greyvalues of original image
 *
 *  @return pointer to smoothed image
 *
 * AUTHOR & DATE
 */

/* include files                                                         */

#include "ipFuncP.h"
ipPicDescriptor *ipFuncGausF   ( ipPicDescriptor *pic_old,
                                 ipUInt4_t       len_mask,
                                 ipUInt4_t       dim_mask,  
                                 ipFuncFlagI_t   border );
#ifndef DOXYGEN_IGNORE

#ifndef lint
  static char *what = { "@(#)ipFuncGaussF\t\tDKFZ (Dept. MBI)\t"__DATE__ };
#endif



/* --------------------------------------------------------------------- */
/* 
** function ipFuncGausF                                                 
*/
/* --------------------------------------------------------------------- */
 
ipPicDescriptor *ipFuncGausF   ( ipPicDescriptor *pic_old,
                                 ipUInt4_t       len_mask,
                                 ipUInt4_t       dim_mask,  
                                 ipFuncFlagI_t   border )
{
  ipPicDescriptor *pic_new;          /* pointer to new image structure   */
  ipPicDescriptor *pic_mask;         /* pointer to mask                  */
  ipUInt4_t       n[_mitkIpPicNDIM];     /* size of each dimension           */
  ipUInt4_t       ind[_mitkIpPicNDIM];   /* loop index vector                */
  ipUInt4_t       i, k;              /* loop index                       */
  ipUInt4_t       no_elem;           /* number of mask elements          */
  ipUInt4_t       offset;            /* offset of pixels                 */
  ipUInt4_t       element;           /* used to calculate mask elements  */
  ipUInt4_t       sum;               /* sum of all mask elements         */
  ipUInt4_t       nn, nfac, kfac;    /* used to calculate bin. coeff     */
  ipUInt4_t       *bin;              /* binomial coeffizients            */

  /* check data                                                          */

  if ( _ipFuncError ( pic_old ) != mitkIpFuncOK ) return ( mitkIpFuncERROR );
  if ( pic_old->dim < dim_mask || dim_mask < 1 )
    {
       _ipFuncSetErrno ( mitkIpFuncDIMMASC_ERROR );
       return ( mitkIpFuncERROR );
    }
  if ( len_mask % 2 != 1 )
    {
       _ipFuncSetErrno ( mitkIpFuncDIM_ERROR );
       return ( mitkIpFuncERROR );
    }

  /* calculate binomial coefficient                                      */

  bin       = malloc ( len_mask * sizeof ( ipUInt4_t ) );
  if ( bin == NULL ) 
    {
       _ipFuncSetErrno ( mitkIpFuncMALLOC_ERROR );
       return ( mitkIpFuncERROR );
    }

  nn        = len_mask;
  bin[0]    = 1;
  bin[nn-1] = 1;
  nfac      = 1;
  kfac      = 1;

  for ( k = 1; k < nn-1; k++ )
    {
       kfac   = k * kfac;
       nfac   = nfac * ( nn - k );
       bin[k] = nfac / kfac;
    }

  /* initialize mask                                                     */
  
  pic_mask       = ipPicNew();
  
  if ( pic_mask == NULL )
    {
       free ( bin );
       _ipFuncSetErrno ( mitkIpFuncPICNEW_ERROR );
       return ( mitkIpFuncERROR );
    }

  pic_mask->type = ipPicFloat;
  pic_mask->bpe  = 64;
  pic_mask->dim  = dim_mask;

  for ( i = 0; i < dim_mask; i++ ) pic_mask->n[i] = len_mask;

  pic_mask->data = malloc ( _ipPicSize ( pic_mask ) );

  if ( pic_mask->data == NULL )
    {
       free ( bin );
       ipPicFree ( pic_mask );
       _ipFuncSetErrno ( mitkIpFuncPICNEW_ERROR );
       return ( mitkIpFuncERROR );
    }

  /* initialize vectors                                                  */

  for ( i = 0; i < pic_mask->dim; i++ )
    n[i] = len_mask;
  for ( i = pic_mask->dim; i < _mitkIpPicNDIM; i++ )
    n[i] = 1;

  /* calculate mask                                                      */

  offset = 0;
  sum    = 0;
  for ( ind[7] = 0; ind[7] < n[7]; ind[7]++ )
    for ( ind[6] = 0; ind[6] < n[6]; ind[6]++ )
      for ( ind[5] = 0; ind[5] < n[5]; ind[5]++ )
        for ( ind[4] = 0; ind[4] < n[4]; ind[4]++ )
          for ( ind[3] = 0; ind[3] < n[3]; ind[3]++ )
            for ( ind[2] = 0; ind[2] < n[2]; ind[2]++ )
              for ( ind[1] = 0; ind[1] < n[1]; ind[1]++ )
                for ( ind[0] = 0; ind[0] < n[0]; ind[0]++ )
                  {
                    element = 1;
                    for ( i = 0; i < pic_mask->dim; i++ )
                      element = element * bin[ind[i]];

                    (( ipFloat8_t * )pic_mask->data)[offset] = 
                       ( ipFloat8_t ) element;
                    sum = sum + element;
                    offset++;
                  }

  no_elem = _ipPicElements ( pic_mask );
  for ( i = 0; i < no_elem; i++ )                          
    (( ipFloat8_t * ) pic_mask->data ) [i] =  
       (( ipFloat8_t * ) pic_mask->data ) [i] / ( ipFloat8_t ) sum;

  /* convolve image with Gausian mask                                  */

  pic_new = ipFuncConv ( pic_old, pic_mask, border );

  ipPicFree ( pic_mask );
  free ( bin );
  /* Copy Tags */

  ipFuncCopyTags(pic_new, pic_old);
  
  

  return ( pic_new );
}

#endif

